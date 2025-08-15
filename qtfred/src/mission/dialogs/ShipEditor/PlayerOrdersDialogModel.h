#pragma once

#include "../AbstractDialogModel.h"

namespace fso {
	namespace fred {
		namespace dialogs {

			class PlayerOrdersDialogModel : public AbstractDialogModel {
			private:

				bool m_multi;

				int m_num_checks_active;

				SCP_vector<size_t> acceptedOrders;
				SCP_vector<SCP_string> orderNames;

				SCP_vector<int> currentOrders;
			public:
				PlayerOrdersDialogModel(QObject* parent, EditorViewport* viewport, bool multi);
			  void initialiseData(bool);

				bool apply() override;
				void reject() override;

				 SCP_vector<size_t> getAcceptedOrders() const;
				 SCP_vector<SCP_string> getOrderNames() const;
				 SCP_vector<int> getCurrentOrders() const;
				 void setCurrentOrder(const int, const size_t);
			};
		}
	}
}