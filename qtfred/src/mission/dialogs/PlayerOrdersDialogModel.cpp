#include "PlayerOrdersDialogModel.h"
#include <hud/hudsquadmsg.h>

namespace fso {
	namespace fred {
		namespace dialogs {

			PlayerOrdersDialogModel::PlayerOrdersDialogModel(QObject* parent, EditorViewport* viewport, bool multi) :
				AbstractDialogModel(parent, viewport)
			{
				m_multi = multi;
				initialiseData();
			}

			bool PlayerOrdersDialogModel::apply()
			{
				int orders_accepted;
				object* objp;

				if (!m_multi) {
					orders_accepted = 0;
					for (int i = 0; i < m_num_checks_active; i++) {
						if (currentOrders[i] == 1)
							orders_accepted |= acceptedOrders[i];
					}
					Ships[_editor->cur_ship].orders_accepted = orders_accepted;
				}
				else {
					for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
						if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags[Object::Object_Flags::Marked])) {
							Ships[objp->instance].orders_accepted = 0;
							for (int i = 0; i < m_num_checks_active; i++) {
								int box_value;

								box_value = currentOrders[i];
								// get the status of the checkbox -- if in the indeterminite state, then
								// skip it
								if (box_value == 2)
									continue;

								// if the button is set, then set the bit, otherwise, clear the bit
								if (box_value == 1)
									Ships[objp->instance].orders_accepted |= acceptedOrders[i];
								else
									Ships[objp->instance].orders_accepted &= ~(acceptedOrders[i]);
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

			void PlayerOrdersDialogModel::initialiseData() {
				int default_orders;

				object* objp;

				if (!m_multi) {
					default_orders = ship_get_default_orders_accepted(&Ship_info[Ships[_editor->cur_ship].ship_info_index]);
				}
				else {
					default_orders = 0;
					for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
						if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags[Object::Object_Flags::Marked])) {
							int these_orders;

							these_orders = ship_get_default_orders_accepted(&Ship_info[Ships[objp->instance].ship_info_index]);
							if (default_orders == 0) {
								default_orders = these_orders;
							}
							else { Assert(default_orders == these_orders); }
						}
					}
				}
				m_num_checks_active = 0;
				acceptedOrders.resize(NUM_COMM_ORDER_ITEMS);
				orderNames.resize(NUM_COMM_ORDER_ITEMS);
				for (int i = 0; i < NUM_COMM_ORDER_ITEMS; i++)
				{
					if (default_orders & Comm_orders[i].item)
					{
						orderNames[m_num_checks_active] = Comm_orders[i].name;
						acceptedOrders[m_num_checks_active] = Comm_orders[i].item;
						m_num_checks_active++;
					}
				}

				currentOrders.resize(m_num_checks_active);
				if (!m_multi) {
					int orders_accepted = Ships[_editor->cur_ship].orders_accepted;
					for (int i = 0; i < m_num_checks_active; i++) {
						if (acceptedOrders[i] & orders_accepted)
							currentOrders[i] = 1;
					}
				}
				else {
					int first_time;

					first_time = 1;
					int orders_accepted;
					for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
						if (((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) && (objp->flags[Object::Object_Flags::Marked])) {

							// get the orders for this ship.  If a state is not set 
							orders_accepted = Ships[objp->instance].orders_accepted;
							if (first_time) {
								for (int i = 0; i < m_num_checks_active; i++) {
									if (acceptedOrders[i] & orders_accepted)
										currentOrders[i] = 1;
								}
								first_time = 0;
							}
							else {
								for (int i = 0; i < m_num_checks_active; i++) {
									// see if the order matches the check box order
									if (acceptedOrders[i] & orders_accepted) {
										// if it matches, if it is not already set, then it is indeterminate.
										if (!(currentOrders[i] == 1))
											currentOrders[i] = 2;;
									}
									else {
										// if the order isn't active, and already set, mark as indeterminite.
										if (currentOrders[i] = 1)
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