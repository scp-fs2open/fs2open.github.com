#include "PlayerOrdersDialogModel.h"
#include <hud/hudsquadmsg.h>

namespace fso {
	namespace fred {
		namespace dialogs {

			PlayerOrdersDialogModel::PlayerOrdersDialogModel(QObject* parent, EditorViewport* viewport, bool multi) :
				AbstractDialogModel(parent, viewport)
			{
				initialiseData(multi);
			}

			bool PlayerOrdersDialogModel::apply()
			{
				std::set<size_t> orders_accepted;
				object* objp;

				if (!m_multi) {
					for (int i = 0; i < m_num_checks_active; i++) {
						if (currentOrders[i] == 1)
							orders_accepted.insert(acceptedOrders[i]);
					}
					Ships[_editor->cur_ship].orders_accepted = orders_accepted;
				}
				else {
					for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
						if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags[Object::Object_Flags::Marked])) {
							Ships[objp->instance].orders_accepted.clear();
							for (int i = 0; i < m_num_checks_active; i++) {
								int box_value;

								box_value = currentOrders[i];
								// get the status of the checkbox -- if in the indeterminite state, then
								// skip it
								if (box_value == 2)
									continue;

								// if the button is set, then set the bit, otherwise, clear the bit
								if (box_value == 1)
									Ships[objp->instance].orders_accepted.insert(acceptedOrders[i]);
								else
									Ships[objp->instance].orders_accepted.erase(acceptedOrders[i]);
							}
						}
					}
				}

				return true;
			}

			void PlayerOrdersDialogModel::reject()
			{
			}

			 bool PlayerOrdersDialogModel::query_modified() const
			{
				return _modified;
			}

			 SCP_vector<int> PlayerOrdersDialogModel::getAcceptedOrders() const
			{
				return acceptedOrders;
			}

			 SCP_vector<SCP_string> PlayerOrdersDialogModel::getOrderNames() const
			{
				return orderNames;
			}

			 SCP_vector<int> PlayerOrdersDialogModel::getCurrentOrders() const
			{
				return currentOrders;
			}

			void PlayerOrdersDialogModel::setCurrentOrder(const int value, const int index)
			{
				currentOrders[index] = value;
				set_modified();
				modelChanged();
			}

			void PlayerOrdersDialogModel::initialiseData(bool multi) {
				std::set<size_t> default_orders;

				object* objp;
				m_multi = multi;

				if (!m_multi) {
					default_orders = ship_get_default_orders_accepted(&Ship_info[Ships[_editor->cur_ship].ship_info_index]);
				}
				else {
					for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
						if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags[Object::Object_Flags::Marked])) {
							const std::set<size_t>& these_orders = ship_get_default_orders_accepted(&Ship_info[Ships[objp->instance].ship_info_index]);
							
							if (default_orders.empty()) {
								default_orders = these_orders;
							}
							else { Assert(default_orders == these_orders); }
						}
					}
				}
				m_num_checks_active = 0;
				acceptedOrders.resize(Player_orders.size());
				orderNames.resize(Player_orders.size());
				for (size_t order_id : default_orders)
				{
					orderNames[m_num_checks_active] = Player_orders[order_id].localized_name;
					acceptedOrders[m_num_checks_active] = (int) order_id;
					m_num_checks_active++;
				}

				currentOrders.resize(m_num_checks_active);
				if (!m_multi) {
					const std::set<size_t>& orders_accepted = Ships[_editor->cur_ship].orders_accepted;
					for (int i = 0; i < m_num_checks_active; i++) {
						if (orders_accepted.find(acceptedOrders[i]) != orders_accepted.end())
							currentOrders[i] = 1;
					}
				}
				else {
					int first_time;

					first_time = 1;
					std::set<size_t> orders_accepted;
					for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
						if (((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) && (objp->flags[Object::Object_Flags::Marked])) {

							// get the orders for this ship.  If a state is not set 
							orders_accepted = Ships[objp->instance].orders_accepted;
							if (first_time) {
								for (int i = 0; i < m_num_checks_active; i++) {
									if (orders_accepted.find(acceptedOrders[i]) != orders_accepted.end())
										currentOrders[i] = 1;
								}
								first_time = 0;
							}
							else {
								for (int i = 0; i < m_num_checks_active; i++) {
									// see if the order matches the check box order
									if (orders_accepted.find(acceptedOrders[i]) != orders_accepted.end()) {
										// if it matches, if it is not already set, then it is indeterminate.
										if (!(currentOrders[i] == 1))
											currentOrders[i] = 2;;
									}
									else {
										// if the order isn't active, and already set, mark as indeterminite.
										if (currentOrders[i] == 1)
											currentOrders[i] = 2;
									}
								}
							}
						}
					}
				}
				modelChanged();
			}
			void PlayerOrdersDialogModel::set_modified()
			{
				if (!_modified) {
					_modified = true;
				}
			}
		}
	}
}