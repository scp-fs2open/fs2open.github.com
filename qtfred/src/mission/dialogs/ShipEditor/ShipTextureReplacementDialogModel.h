#pragma once

#include "../AbstractDialogModel.h"

namespace fso {
	namespace fred {
		namespace dialogs {
			constexpr auto NUM__SUBTEXTURE_TYPES = 7;
			class ShipTextureReplacementDialogModel : public AbstractDialogModel {
			private:
				void initSubTypes(polymodel* model, int);

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
				void saveSubMap(const size_t index, const SCP_string& type);
				static bool testTexture(const SCP_string&);
				static texture_replace* texture_set(texture_replace* dest, const texture_replace* src);
			public:
				ShipTextureReplacementDialogModel(QObject* parent, EditorViewport* viewport, bool multi);
			  void initialiseData(bool);

				bool apply() override;
				void reject() override;

				size_t getSize() const;
				SCP_string getDefaultName(const size_t) const;

				void setMap(const size_t index, const SCP_string& type, const SCP_string& newName);
				SCP_string getMap(const size_t index, const SCP_string& type) const;

				SCP_map<SCP_string, bool> getSubtypesForMap(const size_t index) const;
				SCP_map<SCP_string, bool> getReplace(const size_t index) const;
				SCP_map<SCP_string, bool> getInherit(const size_t index) const;
				void setReplace(const size_t index, const SCP_string& type, const bool state);
				void setInherit(const size_t index, const SCP_string& type, const bool state);


			};
		}
	}
}