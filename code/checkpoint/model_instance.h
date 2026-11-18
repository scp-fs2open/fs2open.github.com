#pragma once

#include "checkpoint/codec.h"

namespace checkpoint {

// Saves the state of an object's model instance: the pose of its submodels, replaced textures and running animations.
// Objects without a model instance (-1) save nothing.
void save_model_instance(Writer& w, int model_instance_num);

// Loads that state onto a newly created model instance, whose initial animation states (if any) have already been set
// up.  If the object has no model instance (-1), or its model is a different one than when saving, the saved state is
// skipped.
void load_model_instance(Reader& r, int model_instance_num);

} // namespace checkpoint
