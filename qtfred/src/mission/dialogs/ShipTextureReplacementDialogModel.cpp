#include "ShipTextureReplacementDialogModel.h"

#include "mission/object.h"

namespace fso {
	namespace fred {
		namespace dialogs {
			ShipTextureReplacementDialogModel::ShipTextureReplacementDialogModel(QObject* parent, EditorViewport* viewport, bool multi) :
				AbstractDialogModel(parent, viewport)
			{
				m_multi = multi;
				initialiseData();
			}

			void ShipTextureReplacementDialogModel::initialiseData()
			{
				char texture_file[MAX_FILENAME_LEN];
				char* p = nullptr;
				int duplicate;
				polymodel* pm = model_get(Ship_info[Ships[_editor->cur_ship].ship_info_index].model_num);
				defaultTextures.clear();
				defaultTextures.resize(pm->n_textures);
				currentTextures.clear();
				currentTextures.resize(pm->n_textures);
				subTypesAvailable.clear();
				subTypesAvailable.resize(pm->n_textures);
				replaceMap.clear();
				replaceMap.resize(pm->n_textures);
				inheritMap.clear();
				inheritMap.resize(pm->n_textures);

				// look for textures to populate the list
				for (int i = 0; i < pm->n_textures; i++) {
					// get texture file name
					bm_get_filename(pm->maps[i].textures[0].GetOriginalTexture(), texture_file);

					// skip blank textures
					if (!strlen(texture_file))
						continue;

					// get rid of file extension
					p = strchr(texture_file, '.');
					if (p)
					{
						//mprintf(( "ignoring extension on file '%s'\n", texture_file ));
						*p = 0;
					}

					// check for duplicate textures in list
					duplicate = -1;
					for (int k = 0; k < defaultTextures.size(); k++)
					{
						if (!stricmp(defaultTextures[k].c_str(), texture_file))
						{
							duplicate = k;
							break;
						}
					}

					if (duplicate >= 0)
						continue;

					// make old texture lowercase
					strlwr(texture_file);

					// add it to the field
					defaultTextures[i] = texture_file;
					currentTextures[i].insert(std::pair<SCP_string, SCP_string>("main", ""));
					//Get all Available SubTypes
					initSubTypes(pm, i);

				}

				if (!m_multi) {
					for (SCP_vector<texture_replace>::iterator ii = Fred_texture_replacements.begin(); ii != Fred_texture_replacements.end(); ++ii)
					{
						if (!stricmp(Ships[_editor->cur_ship].ship_name, ii->ship_name) && !(ii->from_table))
						{
							SCP_string pureName = strlwr(ii->old_texture);
							auto npos = pureName.find_last_of('-');
							if (npos != SCP_string::npos) {
								pureName = pureName.substr(0, pureName.find_last_of('-'));
							}

							// look for corresponding old texture
							for (int i = 0; i < defaultTextures.size(); i++)
							{
								// if match
								if (!stricmp(defaultTextures[i].c_str(), pureName.c_str()))
								{
									SCP_string newText = strlwr(ii->new_texture);
									npos = newText.find_last_of('-');
									SCP_string type;
									if (npos != SCP_string::npos) {
										type = newText.substr(npos + 1);
										newText = newText.substr(0, newText.find_last_of('-'));
									}
									if (!type.empty()) {
										if (type == "misc") {
											currentTextures[i]["misc"] = newText;
											replaceMap[i]["misc"] = true;
											inheritMap[i]["misc"] = (newText == pureName);
										}
										if (type == "shine") {
											currentTextures[i]["shine"] = newText;
											replaceMap[i]["shine"] = true;
											inheritMap[i]["shine"] = (newText == pureName);

										}
										if (type == "glow") {
											currentTextures[i]["glow"] = newText;
											replaceMap[i]["glow"] = true;
											inheritMap[i]["glow"] = (newText == pureName);

										}
										if (type == "normal") {
											currentTextures[i]["normal"] = newText;
											replaceMap[i]["normal"] = true;
											inheritMap[i]["normal"] = (newText == pureName);

										}
										if (type == "height") {
											currentTextures[i]["height"] = newText;
											replaceMap[i]["height"] = true;
											inheritMap[i]["height"] = (newText == pureName);

										}
										if (type == "ao") {
											currentTextures[i]["ao"] = newText;
											replaceMap[i]["ao"] = true;
											inheritMap[i]["ao"] = (newText == pureName);

										}
										if (type == "reflect") {
											currentTextures[i]["reflect"] = newText;
											replaceMap[i]["reflect"] = true;
											inheritMap[i]["reflect"] = (newText == pureName);

										}
									}
									else {
										currentTextures[i]["main"] = newText;
									}

									// we found one, so no more to check
									break;
								}
							}
						}
					}
				}
			}
			void ShipTextureReplacementDialogModel::initSubTypes(polymodel* model, int MapNum)
			{
				subTypesAvailable[MapNum].insert(std::pair<SCP_string, bool>("misc", false));
				subTypesAvailable[MapNum].insert(std::pair<SCP_string, bool>("shine", false));
				subTypesAvailable[MapNum].insert(std::pair<SCP_string, bool>("glow", false));
				subTypesAvailable[MapNum].insert(std::pair<SCP_string, bool>("normal", false));
				subTypesAvailable[MapNum].insert(std::pair<SCP_string, bool>("height", false));
				subTypesAvailable[MapNum].insert(std::pair<SCP_string, bool>("ao", false));
				subTypesAvailable[MapNum].insert(std::pair<SCP_string, bool>("reflect", false));

				currentTextures[MapNum].insert(std::pair<SCP_string, SCP_string>("misc", ""));
				currentTextures[MapNum].insert(std::pair<SCP_string, SCP_string>("shine", ""));
				currentTextures[MapNum].insert(std::pair<SCP_string, SCP_string>("glow", ""));
				currentTextures[MapNum].insert(std::pair<SCP_string, SCP_string>("normal", ""));
				currentTextures[MapNum].insert(std::pair<SCP_string, SCP_string>("height", ""));
				currentTextures[MapNum].insert(std::pair<SCP_string, SCP_string>("ao", ""));
				currentTextures[MapNum].insert(std::pair<SCP_string, SCP_string>("reflect", ""));

				replaceMap[MapNum].insert(std::pair<SCP_string, bool>("misc", false));
				replaceMap[MapNum].insert(std::pair<SCP_string, bool>("shine", false));
				replaceMap[MapNum].insert(std::pair<SCP_string, bool>("glow", false));
				replaceMap[MapNum].insert(std::pair<SCP_string, bool>("normal", false));
				replaceMap[MapNum].insert(std::pair<SCP_string, bool>("height", false));
				replaceMap[MapNum].insert(std::pair<SCP_string, bool>("ao", false));
				replaceMap[MapNum].insert(std::pair<SCP_string, bool>("reflect", false));

				inheritMap[MapNum].insert(std::pair<SCP_string, bool>("misc", true));
				inheritMap[MapNum].insert(std::pair<SCP_string, bool>("shine", true));
				inheritMap[MapNum].insert(std::pair<SCP_string, bool>("glow", true));
				inheritMap[MapNum].insert(std::pair<SCP_string, bool>("normal", true));
				inheritMap[MapNum].insert(std::pair<SCP_string, bool>("height", true));
				inheritMap[MapNum].insert(std::pair<SCP_string, bool>("ao", true));
				inheritMap[MapNum].insert(std::pair<SCP_string, bool>("reflect", true));
				char subMap[MAX_FILENAME_LEN];
				//init saftly, probly not necessary
				for (int j = 1; j < TM_NUM_TYPES; j++) {
					bm_get_filename(model->maps[MapNum].textures[j].GetOriginalTexture(), subMap);
					char* p = strchr(subMap, '.');
					if (p)
					{
						//mprintf(( "ignoring extension on file '%s'\n", texture_file ));
						*p = 0;
					}
					SCP_string subMapClean = strlwr(subMap);
					SCP_string type;
					auto npos = subMapClean.find_last_of('-');
					if (npos != SCP_string::npos) {
						type = subMapClean.substr(npos + 1);
					}
					else {
						continue;
					}
					if (!type.empty()) {
						if (type == "trans") {
						}
						else if (type == "misc") {
							subTypesAvailable[MapNum]["misc"] = true;
						}
						else if (type == "shine") {
							subTypesAvailable[MapNum]["shine"] = true;
						}
						else if (type == "glow") {
							subTypesAvailable[MapNum]["glow"] = true;
						}
						else if (type == "normal") {
							subTypesAvailable[MapNum]["normal"] = true;
						}
						else if (type == "height") {
							subTypesAvailable[MapNum]["height"] = true;
						}
						else if (type == "ao") {
							subTypesAvailable[MapNum]["ao"] = true;
						}
						else if (type == "reflect") {
							subTypesAvailable[MapNum]["reflect"] = true;
						}
						else {
							error_display(1, "Invalid Map type %s. Check your model's texture names or get a programmer", type.c_str());
						}
					}
				}
			}

			bool ShipTextureReplacementDialogModel::apply()
			{
				if (query_modified()) {
					for (int i = 0; i < getSize(); i++) {
						if ((!currentTextures[i]["main"].empty()) && (currentTextures[i]["main"] != defaultTextures[i])) {
							mainChanged = true;
							SCP_string name = currentTextures[i]["main"];
							if (testTexture(name)) {
								SCP_vector<texture_replace>::iterator ii, end;
								end = Fred_texture_replacements.end();
								if (!m_multi) {
									end = Fred_texture_replacements.end();
									for (ii = Fred_texture_replacements.begin(); ii != end; ++ii)
									{
										if (!stricmp(ii->ship_name, Ships[_editor->cur_ship].ship_name))
										{
											do {
												end--;
											} while (end != ii && !stricmp(end->ship_name, Ships[_editor->cur_ship].ship_name));
											if (end == ii)
												break;
											texture_set(&(*ii), &(*end));
										}
									}

									if (end != Fred_texture_replacements.end())
										Fred_texture_replacements.erase(end, Fred_texture_replacements.end());

									// now put the new entries on the end of the list
									texture_replace tr;
									strcpy_s(tr.old_texture, defaultTextures[i].c_str());
									strcpy_s(tr.new_texture, name.c_str());
									strcpy_s(tr.ship_name, Ships[_editor->cur_ship].ship_name);
									tr.new_texture_id = -1;
									tr.from_table = false;

									// assign to global FRED array
									Fred_texture_replacements.push_back(tr);
									_editor->missionChanged();
								}
								else {
									object* objp = nullptr;
									objp = GET_FIRST(&obj_used_list);
									while (objp != END_OF_LIST(&obj_used_list)) {
										if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
											(objp->flags[Object::Object_Flags::Marked])) {
											Assert((objp->type == OBJ_SHIP) || (objp->type == OBJ_START));
											auto shipp = &Ships[get_ship_from_obj(objp)];
											end = Fred_texture_replacements.end();
											for (ii = Fred_texture_replacements.begin(); ii != end; ++ii)
											{
												if (!stricmp(ii->ship_name, shipp->ship_name))
												{
													do {
														end--;
													} while (end != ii && !stricmp(end->ship_name, shipp->ship_name));
													if (end == ii)
														break;
													texture_set(&(*ii), &(*end));
												}
											}
											if (end != Fred_texture_replacements.end())
												Fred_texture_replacements.erase(end, Fred_texture_replacements.end());

											// now put the new entries on the end of the list
											texture_replace tr;
											strcpy_s(tr.old_texture, defaultTextures[i].c_str());
											strcpy_s(tr.new_texture, name.c_str());
											strcpy_s(tr.ship_name, shipp->ship_name);
											tr.new_texture_id = -1;
											tr.from_table = false;

											// assign to global FRED array
											Fred_texture_replacements.push_back(tr);
											_editor->missionChanged();
										}

										objp = GET_NEXT(objp);
									}
								}
							}
							else {
								auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Missing Texture", "FRED was unable to find Main Texture %s \n Aborting at this point",
									{ DialogButton::Ok });
								if (button == DialogButton::Ok) {
									return false;
								}
							}
						}
						saveSubMap(i, "misc");
						saveSubMap(i, "shine");
						saveSubMap(i, "glow");
						saveSubMap(i, "normal");
						saveSubMap(i, "height");
						saveSubMap(i, "ao");
						saveSubMap(i, "reflect");	
						_editor->missionChanged();
					}
					_editor->missionChanged();
					return true;
				}
				else {
					return true;
				}
			}
			void ShipTextureReplacementDialogModel::reject()
			{
			}
			texture_replace* ShipTextureReplacementDialogModel::texture_set(texture_replace* dest, const texture_replace* src)
			{
				dest->new_texture_id = src->new_texture_id;
				strcpy_s(dest->ship_name, src->ship_name);
				strcpy_s(dest->old_texture, src->old_texture);
				strcpy_s(dest->new_texture, src->new_texture);
				dest->from_table = src->from_table;

				return dest;
			}

			void ShipTextureReplacementDialogModel::saveSubMap(int index, SCP_string type) {
				SCP_string fullName;
				if (replaceMap[index][type]) {
					if (inheritMap[index][type]) {
						if (mainChanged) {
							if (!currentTextures[index]["main"].empty()) {
								if (currentTextures[index]["main"] != "invisible") {
									fullName = currentTextures[index]["main"] + "-" + type;
								}
								else {
									fullName = currentTextures[index]["main"];
								}
								if (testTexture(fullName)) {
									SCP_vector<texture_replace>::iterator ii, end;
									end = Fred_texture_replacements.end();
									if (!m_multi) {
										end = Fred_texture_replacements.end();
										for (ii = Fred_texture_replacements.begin(); ii != end; ++ii)
										{
											if (!stricmp(ii->ship_name, Ships[_editor->cur_ship].ship_name))
											{
												do {
													end--;
												} while (end != ii && !stricmp(end->ship_name, Ships[_editor->cur_ship].ship_name));
												if (end == ii)
													break;
												texture_set(&(*ii), &(*end));
											}
										}

										if (end != Fred_texture_replacements.end())
											Fred_texture_replacements.erase(end, Fred_texture_replacements.end());

										// now put the new entries on the end of the list
										texture_replace tr;
										strcpy_s(tr.old_texture, defaultTextures[index].c_str());
										strcpy_s(tr.new_texture, fullName.c_str());
										strcpy_s(tr.ship_name, Ships[_editor->cur_ship].ship_name);
										tr.new_texture_id = -1;
										tr.from_table = false;

										// assign to global FRED array
										Fred_texture_replacements.push_back(tr);
										_editor->missionChanged();
									}
									else {
										object* objp = nullptr;
										objp = GET_FIRST(&obj_used_list);
										while (objp != END_OF_LIST(&obj_used_list)) {
											if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
												(objp->flags[Object::Object_Flags::Marked])) {
												Assert((objp->type == OBJ_SHIP) || (objp->type == OBJ_START));
												auto shipp = &Ships[get_ship_from_obj(objp)];
												end = Fred_texture_replacements.end();
												for (ii = Fred_texture_replacements.begin(); ii != end; ++ii)
												{
													if (!stricmp(ii->ship_name, shipp->ship_name))
													{
														do {
															end--;
														} while (end != ii && !stricmp(end->ship_name, shipp->ship_name));
														if (end == ii)
															break;
														texture_set(&(*ii), &(*end));
													}
												}

												if (end != Fred_texture_replacements.end())
													Fred_texture_replacements.erase(end, Fred_texture_replacements.end());

												// now put the new entries on the end of the list
												texture_replace tr;
												strcpy_s(tr.old_texture, defaultTextures[index].c_str());
												strcpy_s(tr.new_texture, fullName.c_str());
												strcpy_s(tr.ship_name, shipp->ship_name);
												tr.new_texture_id = -1;
												tr.from_table = false;

												// assign to global FRED array
												Fred_texture_replacements.push_back(tr);
												_editor->missionChanged();
											}

											objp = GET_NEXT(objp);
										}
									}
								}
								else {
									auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Missing Texture", "FRED was unable to find %s \n Skipping",
										{ DialogButton::Ok });
									if (button == DialogButton::Ok) {
										return;
									}
								}

							}
						}
						else {
							error_display(0, "You told us to replace the -misc map with inherited data but didnt change the main texture name. Ignoring -misc map change");
						}
					}
					else {
						if (!currentTextures[index][type].empty()) {
							if (currentTextures[index][type] != "invisible") {
								fullName = currentTextures[index][type] + "-" + type;
							}
							else {
								fullName = currentTextures[index][type];
							}
							if (testTexture(fullName)) {
								SCP_vector<texture_replace>::iterator ii, end;
								end = Fred_texture_replacements.end();
								if (!m_multi) {
									end = Fred_texture_replacements.end();
									for (ii = Fred_texture_replacements.begin(); ii != end; ++ii)
									{
										if (!stricmp(ii->ship_name, Ships[_editor->cur_ship].ship_name))
										{
											do {
												end--;
											} while (end != ii && !stricmp(end->ship_name, Ships[_editor->cur_ship].ship_name));
											if (end == ii)
												break;
											texture_set(&(*ii), &(*end));
										}
									}

									if (end != Fred_texture_replacements.end())
										Fred_texture_replacements.erase(end, Fred_texture_replacements.end());

									// now put the new entries on the end of the list
									texture_replace tr;
									strcpy_s(tr.old_texture, defaultTextures[index].c_str());
									strcpy_s(tr.new_texture, fullName.c_str());
									strcpy_s(tr.ship_name, Ships[_editor->cur_ship].ship_name);
									tr.new_texture_id = -1;
									tr.from_table = false;

									// assign to global FRED array
									Fred_texture_replacements.push_back(tr);
									_editor->missionChanged();
								}
								else {
									object* objp = nullptr;
									objp = GET_FIRST(&obj_used_list);
									while (objp != END_OF_LIST(&obj_used_list)) {
										if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
											(objp->flags[Object::Object_Flags::Marked])) {
											Assert((objp->type == OBJ_SHIP) || (objp->type == OBJ_START));
											auto shipp = &Ships[get_ship_from_obj(objp)];
											end = Fred_texture_replacements.end();
											for (ii = Fred_texture_replacements.begin(); ii != end; ++ii)
											{
												if (!stricmp(ii->ship_name, shipp->ship_name))
												{
													do {
														end--;
													} while (end != ii && !stricmp(end->ship_name, shipp->ship_name));
													if (end == ii)
														break;
													texture_set(&(*ii), &(*end));
												}
											}

											if (end != Fred_texture_replacements.end())
												Fred_texture_replacements.erase(end, Fred_texture_replacements.end());

											// now put the new entries on the end of the list
											texture_replace tr;
											strcpy_s(tr.old_texture, defaultTextures[index].c_str());
											strcpy_s(tr.new_texture, fullName.c_str());
											strcpy_s(tr.ship_name, shipp->ship_name);
											tr.new_texture_id = -1;
											tr.from_table = false;

											// assign to global FRED array
											Fred_texture_replacements.push_back(tr);
											_editor->missionChanged();
										}

										objp = GET_NEXT(objp);
									}
								}
							}
							else {
								auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Missing Texture", "FRED was unable to find %s \n Skipping",
									{ DialogButton::Ok });
								if (button == DialogButton::Ok) {
									return;
								}
							}

						}
					}
				}
			}

			bool ShipTextureReplacementDialogModel::testTexture(SCP_string fullName)
			{
				int temp_bmp, temp_frames, temp_fps;
				if (fullName == "invisible") {
					return true;
				}
				else {
					// try loading the texture (bmpman should take care of eventually unloading it)
					temp_bmp = bm_load(fullName);
					if (temp_bmp < 0)
					{
						temp_bmp = bm_load_animation(fullName.c_str(), &temp_frames, &temp_fps, nullptr, nullptr, nullptr, true);
					}
					if (temp_bmp < 0)
					{
						return false;
					}
					return true;
				}
			}

			int ShipTextureReplacementDialogModel::getSize() const
			{
				return defaultTextures.size();
			}
			SCP_string ShipTextureReplacementDialogModel::getDefaultName(const int index) const
			{
				Assert(index <= defaultTextures.size());
				return defaultTextures[index];
			}
			void ShipTextureReplacementDialogModel::setMap(const int index, const SCP_string& type, const SCP_string& newName)
			{
				Assert(index <= currentTextures.size());
				SCP_map<SCP_string, SCP_string>::const_iterator pos = currentTextures[index].find(type);
				if (pos == currentTextures[index].end()) {
					//handle the error
				}
				else {
					modify(currentTextures[index][type], newName);
				}

			}
			SCP_string ShipTextureReplacementDialogModel::getMap(const int index, const SCP_string& type) const {
				Assert(index <= currentTextures.size());
				SCP_map<SCP_string, SCP_string>::const_iterator pos = currentTextures[index].find(type);
				if (pos == currentTextures[index].end()) {
					//handle the error
				}
				else {
					return pos->second;
				}
			}
			SCP_map<SCP_string, bool> ShipTextureReplacementDialogModel::getSubtypesForMap(int index) const
			{
				return subTypesAvailable[index];
			}
			SCP_map<SCP_string, bool> ShipTextureReplacementDialogModel::getReplace(int index) const
			{
				return replaceMap[index];
			}
			SCP_map<SCP_string, bool> ShipTextureReplacementDialogModel::getInherit(int index) const
			{
				return inheritMap[index];
			}

			void ShipTextureReplacementDialogModel::setReplace(const int index, const SCP_string& type, const bool state)
			{
				modify(replaceMap[index][type], state);
			}
			void ShipTextureReplacementDialogModel::setInherit(const int index, const SCP_string& type, const bool state)
			{
				modify(inheritMap[index][type], state);
			}
			void ShipTextureReplacementDialogModel::set_modified()
			{
				if (!_modified) {
					_modified = true;
				}
			}

			bool ShipTextureReplacementDialogModel::query_modified() const
			{
				return  _modified;;
			}
		}
	}
}