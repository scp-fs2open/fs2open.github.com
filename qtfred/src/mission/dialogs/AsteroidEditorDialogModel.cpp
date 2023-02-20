#include "mission/dialogs/AsteroidEditorDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

AsteroidEditorDialogModel::AsteroidEditorDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport),
	_enable_asteroids(false),
	_enable_inner_bounds(false),
	_num_asteroids(0),
	_avg_speed(0),
	_min_x(""),
	_min_y(""),
	_min_z(""),
	_max_x(""),
	_max_y(""),
	_max_z(""),
	_inner_min_x(""),
	_inner_min_y(""),
	_inner_min_z(""),
	_inner_max_x(""),
	_inner_max_y(""),
	_inner_max_z(""),
	_field_type(FT_ACTIVE),
	_debris_genre(DG_ASTEROID),
	_bypass_errors(false),
	_modified(false),
	_cur_field(0),
	_last_field(-1)
{
	for (auto i = 0ul; i < ship_debris_idx_lookup.size(); ++i) {
		debris_inverse_idx_lookup.emplace(ship_debris_idx_lookup[i], i);
	}
	// note that normal asteroids use the same index field! Need to add dummy entries for them as well
	for (auto i = 0; i < MAX_ACTIVE_DEBRIS_TYPES; ++i) {
		debris_inverse_idx_lookup.emplace(i, 0);
	}
	initializeData();
}

bool AsteroidEditorDialogModel::apply()
{
	update_init();
	if (!AsteroidEditorDialogModel::validate_data()) {
		return false;
	}
	Asteroid_field = _a_field;
	return true;
}

void AsteroidEditorDialogModel::reject()
{
	//do nothing - only here because parent class reject() function is virtual
}

void AsteroidEditorDialogModel::initializeData()
{
	for (auto& i : _field_debris_type) {
		i = -1;
	}

	_a_field = Asteroid_field;
}

void AsteroidEditorDialogModel::setEnabled(bool enabled)
{
	_enable_asteroids = enabled;
}

bool AsteroidEditorDialogModel::getEnabled()
{
	return _enable_asteroids;
}

void AsteroidEditorDialogModel::setInnerBoxEnabled(bool enabled)
{
	_enable_inner_bounds = enabled;
}

bool AsteroidEditorDialogModel::getInnerBoxEnabled()
{
	return _enable_inner_bounds;
}

void AsteroidEditorDialogModel::setAsteroidEnabled(_roid_types type, bool enabled)
{
	Assertion(type >=0 && type < MAX_ACTIVE_DEBRIS_TYPES, "Invalid Asteroid checkbox type: %i\n", type);
	modify(_field_debris_type[type], enabled == true ? 1 : -1);
}

bool AsteroidEditorDialogModel::getAsteroidEnabled(_roid_types type)
{
	Assertion(type >=0 && type < MAX_ACTIVE_DEBRIS_TYPES, "Invalid Asteroid checkbox type: %i\n", type);
	return (_field_debris_type[type] == 1);
}

void AsteroidEditorDialogModel::setNumAsteroids(int num_asteroids)
{
	modify(_num_asteroids, num_asteroids);
}

int AsteroidEditorDialogModel::getNumAsteroids()
{
	return _num_asteroids;
}

QString & AsteroidEditorDialogModel::getBoxText(_box_line_edits type)
{
	switch (type) {
		case _O_MIN_X: return _min_x;
		case _O_MIN_Y: return _min_y;
		case _O_MIN_Z: return _min_z;
		case _O_MAX_X: return _max_x;
		case _O_MAX_Y: return _max_y;
		case _O_MAX_Z: return _max_z;
		case _I_MIN_X: return _inner_min_x;
		case _I_MIN_Y: return _inner_min_y;
		case _I_MIN_Z: return _inner_min_z;
		case _I_MAX_X: return _inner_max_x;
		case _I_MAX_Y: return _inner_max_y;
		case _I_MAX_Z: return _inner_max_z;
		default:
			UNREACHABLE("Unknown asteroid coordinates enum value found (%i); Get a coder! ", type);
			return _min_x;
	}
}

void AsteroidEditorDialogModel::setBoxText(const QString &text, _box_line_edits type)
{
	switch (type) {
		case _O_MIN_X: modify(_min_x, text); break;
		case _O_MIN_Y: modify(_min_y, text); break;
		case _O_MIN_Z: modify(_min_z, text); break;
		case _O_MAX_X: modify(_max_x, text); break;
		case _O_MAX_Y: modify(_max_y, text); break;
		case _O_MAX_Z: modify(_max_z, text); break;
		case _I_MIN_X: modify(_inner_min_x, text); break;
		case _I_MIN_Y: modify(_inner_min_y, text); break;
		case _I_MIN_Z: modify(_inner_min_z, text); break;
		case _I_MAX_X: modify(_inner_max_x, text); break;
		case _I_MAX_Y: modify(_inner_max_y, text); break;
		case _I_MAX_Z: modify(_inner_max_z, text); break;
		default:
			Error(LOCATION, "Get a coder! Unknown enum value found! %i", type);
			break;
	}
}

void AsteroidEditorDialogModel::setDebrisGenre(debris_genre_t genre)
{
	modify(_debris_genre, genre);
}

debris_genre_t AsteroidEditorDialogModel::getDebrisGenre()
{
	return _debris_genre;
}

void AsteroidEditorDialogModel::setFieldType(field_type_t type)
{
	modify(_field_type, type);
}

field_type_t AsteroidEditorDialogModel::getFieldType()
{
	return _field_type;
}

void AsteroidEditorDialogModel::setFieldDebrisType(int idx, int debris_type)
{
	Assertion(idx >= 0 && idx < MAX_ACTIVE_DEBRIS_TYPES, "Invalid debris index provided: %i\n", idx);
	modify(_field_debris_type[idx], ship_debris_idx_lookup.at(debris_type));
}

int AsteroidEditorDialogModel::getFieldDebrisType(int idx)
{
	Assertion(idx >= 0 && idx < MAX_ACTIVE_DEBRIS_TYPES, "Invalid debris index provided: %i\n", idx);
	return debris_inverse_idx_lookup.at(_field_debris_type[idx]);
}

void AsteroidEditorDialogModel::setAvgSpeed(int speed)
{
	modify(_avg_speed, speed);
}

QString AsteroidEditorDialogModel::getAvgSpeed()
{
	return QString::number(_avg_speed);
}

bool AsteroidEditorDialogModel::validate_data()
{
	if (!_enable_asteroids) {
		return true;
	}
	else {
		// be helpful to the FREDer; try to advise precisely what the problem is
		// more general checks 1st, followed by more specific ones
		_bypass_errors = false;

		// check outer x/y/z max is greater than min
		if (_a_field.max_bound.xyz.x < _a_field.min_bound.xyz.x) {
			showErrorDialogNoCancel( "Outer box 'X' min is greater than max\n");
			return false;
		}

		// check y
		if (_a_field.max_bound.xyz.y < _a_field.min_bound.xyz.y) {
			showErrorDialogNoCancel( "Outer box 'Y' min is greater than max\n");
			return false;
		}

		// check z
		if (_a_field.max_bound.xyz.z < _a_field.min_bound.xyz.z) {
			showErrorDialogNoCancel( "Outer box 'Z' min is greater than max\n");
			return false;
		}

		if (_a_field.has_inner_bound) {
			// check inner x/y/z max is greater than min
			if (_a_field.inner_max_bound.xyz.x < _a_field.inner_min_bound.xyz.x) {
				showErrorDialogNoCancel( "Inner box 'X' min is greater than inner max\n");
				return false;
			}

			if (_a_field.inner_max_bound.xyz.y < _a_field.inner_min_bound.xyz.y) {
				showErrorDialogNoCancel( "Inner box 'Y' min is greater than inner max\n");
				return false;
			}

			if (_a_field.inner_max_bound.xyz.z < _a_field.inner_min_bound.xyz.z) {
				showErrorDialogNoCancel( "Inner box 'Z' min is greater than inner max\n");
				return false;
			}

			// check outer x/y/z max is greater than inner x/y/z max
			if (_a_field.max_bound.xyz.x < _a_field.inner_max_bound.xyz.x) {
				showErrorDialogNoCancel( "Outer box 'X' max is less than inner max\n");
				return false;
			}

			if (_a_field.max_bound.xyz.y < _a_field.inner_max_bound.xyz.y) {
				showErrorDialogNoCancel( "Outer box 'Y' max is less than inner max\n");
				return false;
			}

			if (_a_field.max_bound.xyz.z < _a_field.inner_max_bound.xyz.z) {
				showErrorDialogNoCancel( "Outer box 'Z' max is less than inner max\n");
				return false;
			}

			// check outer x/y/z min is less than inner x/y/z min
			if (_a_field.min_bound.xyz.x > _a_field.inner_min_bound.xyz.x) {
				showErrorDialogNoCancel( "Inner box 'X' min is less than outer min\n");
				return false;
			}

			if (_a_field.min_bound.xyz.y > _a_field.inner_min_bound.xyz.y) {
				showErrorDialogNoCancel( "Inner box 'Y' min is less than outer min\n");
				return false;
			}

			if (_a_field.min_bound.xyz.z > _a_field.inner_min_bound.xyz.z) {
				showErrorDialogNoCancel( "Inner box 'Z' min is less than outer min\n");
				return false;
			}

			// split checks to give FREDers more specific feedback
			// check x thickness
			if (_a_field.inner_min_bound.xyz.x - _MIN_BOX_THICKNESS < _a_field.min_bound.xyz.x) {
				showErrorDialogNoCancel(
						"X axis minimum values must be at least " + \
						std::to_string(_MIN_BOX_THICKNESS) + " apart");
				return false;
			}
			if (_a_field.inner_max_bound.xyz.x + _MIN_BOX_THICKNESS > _a_field.max_bound.xyz.x) {
				showErrorDialogNoCancel(
						"X axis maximum values must be at least " + \
						std::to_string(_MIN_BOX_THICKNESS) + " apart");
				return false;
			}

			// check y thickness
			if (_a_field.inner_min_bound.xyz.y - _MIN_BOX_THICKNESS < _a_field.min_bound.xyz.y) {
				showErrorDialogNoCancel(
						"Y axis minimum values must be at least " + \
						std::to_string(_MIN_BOX_THICKNESS) + " apart");
				return false;
			}
			if (_a_field.inner_max_bound.xyz.y + _MIN_BOX_THICKNESS > _a_field.max_bound.xyz.y) {
				showErrorDialogNoCancel(
						"Y axis maximum values must be at least " + \
						std::to_string(_MIN_BOX_THICKNESS) + " apart");
				return false;
			}

			// check z thickness
			if (_a_field.inner_min_bound.xyz.z - _MIN_BOX_THICKNESS < _a_field.min_bound.xyz.z) {
				showErrorDialogNoCancel(
						"Z axis minimum values must be at least " + \
						std::to_string(_MIN_BOX_THICKNESS) + " apart");
				return false;
			}
			if (_a_field.inner_max_bound.xyz.z + _MIN_BOX_THICKNESS > _a_field.max_bound.xyz.z) {
				showErrorDialogNoCancel(
						"Z axis maximum values must be at least " + \
						std::to_string(_MIN_BOX_THICKNESS) + " apart");
				return false;
			}
		}

		// for a ship debris (i.e. passive) field, need at least one debris type is selected
		if (_a_field.field_type == FT_PASSIVE) {
			if (_a_field.debris_genre == DG_DEBRIS) {
				if ( (_a_field.field_debris_type[0] == -1) && \
						(_a_field.field_debris_type[1] == -1) && \
						(_a_field.field_debris_type[2] == -1) ) {
					showErrorDialogNoCancel("You must choose one or more types of ship debris\n");
					return false;
				}
			}
		}

		// check at least one asteroid subtype is selected
		if (_a_field.debris_genre == DG_ASTEROID) {
			if ( (_a_field.field_debris_type[_AST_BROWN] == -1) && \
					(_a_field.field_debris_type[_AST_BLUE] == -1) && \
					(_a_field.field_debris_type[_AST_ORANGE] == -1) ) {
				showErrorDialogNoCancel("You must choose one or more asteroid subtypes\n");
				return false;
			}
		}

	}

	return true;
}

void AsteroidEditorDialogModel::update_init()
{
	int num_asteroids;

	if (_last_field >= 0) {
		// store into temp asteroid field
		num_asteroids = _a_field.num_initial_asteroids;
		_a_field.num_initial_asteroids = _enable_asteroids ? _num_asteroids : 0;
		CLAMP(_a_field.num_initial_asteroids, 0, MAX_ASTEROIDS);

		if (num_asteroids != _a_field.num_initial_asteroids) {
			set_modified();
		}

		vec3d vel_vec = vmd_x_vector;
		vm_vec_scale(&vel_vec, static_cast<float>(_avg_speed));
		modify(_a_field.vel, vel_vec);

		// save the box coords
		modify(_a_field.min_bound.xyz.x, _min_x.toFloat());
		modify(_a_field.min_bound.xyz.y, _min_y.toFloat());
		modify(_a_field.min_bound.xyz.z, _min_z.toFloat());
		modify(_a_field.max_bound.xyz.x, _max_x.toFloat());
		modify(_a_field.max_bound.xyz.y, _max_y.toFloat());
		modify(_a_field.max_bound.xyz.z, _max_z.toFloat());
		modify(_a_field.inner_min_bound.xyz.x, _inner_min_x.toFloat());
		modify(_a_field.inner_min_bound.xyz.y, _inner_min_y.toFloat());
		modify(_a_field.inner_min_bound.xyz.z, _inner_min_z.toFloat());
		modify(_a_field.inner_max_bound.xyz.x, _inner_max_x.toFloat());
		modify(_a_field.inner_max_bound.xyz.y, _inner_max_y.toFloat());
		modify(_a_field.inner_max_bound.xyz.z, _inner_max_z.toFloat());

		// type of field
		modify(_a_field.field_type, _field_type);
		modify(_a_field.debris_genre, _debris_genre);

		// ship debris
		if ( (_field_type == FT_PASSIVE) && (_debris_genre == DG_DEBRIS) ) {
			for (auto idx=0; idx<MAX_ACTIVE_DEBRIS_TYPES; ++idx) {
				modify(_a_field.field_debris_type[idx], _field_debris_type[idx]);
			}
		}

		// asteroids
		if ( _debris_genre == DG_ASTEROID ) {
			modify(_a_field.field_debris_type[_AST_BROWN], getAsteroidEnabled(_AST_BROWN) == true ? 1 : -1);
			modify(_a_field.field_debris_type[_AST_BLUE], getAsteroidEnabled(_AST_BLUE) == true ? 1 : -1);
			modify(_a_field.field_debris_type[_AST_ORANGE], getAsteroidEnabled(_AST_ORANGE) == true ? 1 : -1);
		}

		modify(_a_field.has_inner_bound, _enable_inner_bounds);
	}

	// get from temp asteroid field into class
	_enable_asteroids = _a_field.num_initial_asteroids ? true : false;
	_enable_inner_bounds = _a_field.has_inner_bound;
	_num_asteroids = _a_field.num_initial_asteroids;
	if (!_enable_asteroids) {
		_num_asteroids = 10;
	}

	// set field type
	_field_type = _a_field.field_type;
	_debris_genre = _a_field.debris_genre;

	_avg_speed = static_cast<int>(vm_vec_mag(&_a_field.vel));

	_min_x = QString::number(_a_field.min_bound.xyz.x, 'f', 1);
	_min_y = QString::number(_a_field.min_bound.xyz.y, 'f', 1);
	_min_z = QString::number(_a_field.min_bound.xyz.z, 'f', 1);
	_max_x = QString::number(_a_field.max_bound.xyz.x, 'f', 1);
	_max_y = QString::number(_a_field.max_bound.xyz.y, 'f', 1);
	_max_z = QString::number(_a_field.max_bound.xyz.z, 'f', 1);
	_inner_min_x = QString::number(_a_field.inner_min_bound.xyz.x, 'f', 1);
	_inner_min_y = QString::number(_a_field.inner_min_bound.xyz.y, 'f', 1);
	_inner_min_z = QString::number(_a_field.inner_min_bound.xyz.z, 'f', 1);
	_inner_max_x = QString::number(_a_field.inner_max_bound.xyz.x, 'f', 1);
	_inner_max_y = QString::number(_a_field.inner_max_bound.xyz.y, 'f', 1);
	_inner_max_z = QString::number(_a_field.inner_max_bound.xyz.z, 'f', 1);

	// ship debris or asteroids
	for (auto i = 0; i < MAX_ACTIVE_DEBRIS_TYPES; ++i) {
		_field_debris_type[i] = _a_field.field_debris_type[i];
	}

	_last_field = _cur_field;
}

void AsteroidEditorDialogModel::set_modified()
{
	_modified = true;
}

void AsteroidEditorDialogModel::unset_modified()
{
	_modified = false;
}

bool AsteroidEditorDialogModel::get_modified()
{
	return _modified;
}

void AsteroidEditorDialogModel::showErrorDialogNoCancel(const SCP_string& message)
{
	if (_bypass_errors) {
		return;
	}

	_bypass_errors = true;
	_viewport->dialogProvider->showButtonDialog(DialogType::Error,
												"Error",
												message,
												{ DialogButton::Ok });
}

} // namespace dialogs
} // namespace fred
} // namespace fso
