#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

#include "asteroid/asteroid.h"

namespace fso {
namespace fred {
namespace dialogs {

class AsteroidEditorDialogModel: public AbstractDialogModel {
Q_OBJECT

public:
	AsteroidEditorDialogModel(QObject* parent, EditorViewport* viewport);

	enum _box_line_edits {
		_I_MIN_X =0,
		_I_MIN_Y,
		_I_MIN_Z,
		_I_MAX_X,
		_I_MAX_Y,
		_I_MAX_Z,
		_O_MIN_X,
		_O_MIN_Y,
		_O_MIN_Z,
		_O_MAX_X,
		_O_MAX_Y,
		_O_MAX_Z,
	};
	enum _roid_types {
		_AST_BROWN  =0,
		_AST_BLUE   =1,
		_AST_ORANGE =2,
	};

	bool apply() override;
	void reject() override;

	void setEnabled(bool enabled);
	bool getEnabled();
	void setInnerBoxEnabled(bool enabled);
	bool getInnerBoxEnabled();
	void setAsteroidEnabled(_roid_types type, bool enabled);
	bool getAsteroidEnabled(_roid_types type);
	void setNumAsteroids(int num_asteroids);
	int  getNumAsteroids();
	void setDebrisGenre(debris_genre_t genre);
	debris_genre_t getDebrisGenre();
	void setFieldType(field_type_t type);
	field_type_t getFieldType();
	void setFieldDebrisType(int idx, int num_asteroids);
	int  getFieldDebrisType(int idx);
	void setAvgSpeed(int speed);
	QString getAvgSpeed();
	void setBoxText(const QString &text, _box_line_edits type);
	QString & getBoxText(_box_line_edits type);

	void update_init();
	bool validate_data();

	void set_modified();
	void unset_modified();
	bool get_modified();

private:
	template<typename T>
	void modify(T &a, const T &b);

	void showErrorDialogNoCancel(const SCP_string& message);
	void initializeData();

	bool  _enable_asteroids;
	bool  _enable_inner_bounds;
	int   _num_asteroids;
	int   _avg_speed;

	QString _min_x;
	QString _min_y;
	QString _min_z;
	QString _max_x;
	QString _max_y;
	QString _max_z;
	QString _inner_min_x;
	QString _inner_min_y;
	QString _inner_min_z;
	QString _inner_max_x;
	QString _inner_max_y;
	QString _inner_max_z;

	int            _field_debris_type[MAX_ACTIVE_DEBRIS_TYPES];  // species and size of ship debris
	field_type_t   _field_type;                                  // active or passive
	debris_genre_t _debris_genre;                                // ship or asteroid
	asteroid_field _a_field;      // :v: had unfinished plans for multiple fields?

	bool _bypass_errors;
	bool _modified;
	int  _cur_field;
	int  _last_field;

	const int _MIN_BOX_THICKNESS = 400;
	// for debris combo box indexes
	// -1 == none, 3 == terran debris (small), etc to 11 == shivan debris (large)
	const std::array<int, 10> ship_debris_idx_lookup{ {-1, 3, 4, 5, 6, 7, 8, 9, 10, 11} };
	// and the inverse as a map + roids - populate in ctor
	std::unordered_map<int, int> debris_inverse_idx_lookup;
};

template<typename T>
inline void AsteroidEditorDialogModel::modify(T &a, const T &b) {
	if (a != b) {
		a = b;
		set_modified();
		modelChanged();
	}
}

} // namespace dialogs
} // namespace fred
} // namespace fso
