#pragma once

#include "AbstractDialogModel.h"

namespace fso {
	namespace fred {
		namespace dialogs {
			constexpr auto NUM__SUBTEXTURE_TYPES = 7;
			class ShipTextureReplacementDialogModel : public AbstractDialogModel {
			private:
				void initSubTypes(polymodel* model, int);

				template<typename T>
				void modify(T& a, const T& b);
				bool _modified = false;
				void set_modified();

				bool m_multi;
				//Used to dermeine what type of texutre a map has.
				SCP_vector<SCP_map<SCP_string, bool>> subTypesAvailable;

				//Wether to replace textures.
				SCP_vector<SCP_map<SCP_string, bool>> replaceMap;
				//Wether to inherit name from base texture.
				SCP_vector<SCP_map<SCP_string, bool>> inheritMap;


				SCP_vector<SCP_string> defaultTextures;
				SCP_vector<SCP_map<SCP_string, SCP_string>> currentTextures;
				bool mainChanged = false;
				void saveSubMap(const int index, const SCP_string& type);
				static bool testTexture(const SCP_string&);
				static texture_replace* texture_set(texture_replace* dest, const texture_replace* src);
			public:
				ShipTextureReplacementDialogModel(QObject* parent, EditorViewport* viewport, bool multi);
			  void initialiseData(bool);

				bool apply() override;
				void reject() override;

				int getSize() const;
				SCP_string getDefaultName(const int) const;

				void setMap(const int index, const SCP_string& type, const SCP_string& newName);
				SCP_string getMap(const int index, const SCP_string& type) const;

				SCP_map<SCP_string, bool> getSubtypesForMap(int index) const;
				SCP_map<SCP_string, bool> getReplace(int index) const;
				SCP_map<SCP_string, bool> getInherit(int index) const;
				void setReplace(const int index, const SCP_string& type, const bool state);
				void setInherit(const int index, const SCP_string& type, const bool state);

				bool query_modified() const;

			};
			template<typename T>
			inline void ShipTextureReplacementDialogModel::modify(T& a, const T& b)
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