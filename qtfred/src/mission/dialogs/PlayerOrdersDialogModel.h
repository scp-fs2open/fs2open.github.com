#pragma once

#include "AbstractDialogModel.h"

namespace fso {
	namespace fred {
		namespace dialogs {

			class PlayerOrdersDialogModel : public AbstractDialogModel {
			private:

				template<typename T>
				void modify(T& a, const T& b);

				bool _modified = false;

				void set_modified();

				bool m_multi;

				int m_num_checks_active;

				SCP_vector<int> acceptedOrders;
				SCP_vector<SCP_string> orderNames;

				SCP_vector<int> currentOrders;
			public:
				PlayerOrdersDialogModel(QObject* parent, EditorViewport* viewport, bool multi);
			  void initialiseData(bool);

				bool apply() override;
				void reject() override;

				 bool query_modified() const;

				 SCP_vector<int> getAcceptedOrders() const;
				 SCP_vector<SCP_string> getOrderNames() const;
				 SCP_vector<int> getCurrentOrders() const;
				void setCurrentOrder(const int, const int);
			};
			template<typename T>
			inline void PlayerOrdersDialogModel::modify(T& a, const T& b)
			{
				if (a != b) {
					a = b;
					set_modified();
					modelChanged();
				}
			}
		}
	}
}