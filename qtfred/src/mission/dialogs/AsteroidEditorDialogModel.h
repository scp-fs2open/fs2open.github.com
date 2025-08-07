#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

#include "asteroid/asteroid.h"

#include <QString>
#include <QVector>
#include <utility>

namespace fso::fred::dialogs {

class AsteroidEditorDialogModel: public AbstractDialogModel {
Q_OBJECT

public:
	AsteroidEditorDialogModel(QObject* parent, EditorViewport* viewport);

	enum _box_line_edits {
		_O_MIN_X = 0,
		_O_MIN_Y,
		_O_MIN_Z,
		_O_MAX_X,
		_O_MAX_Y,
		_O_MAX_Z,
		_I_MIN_X,
		_I_MIN_Y,
		_I_MIN_Z,
		_I_MAX_X,
		_I_MAX_Y,
		_I_MAX_Z,
	};

	// overrides
	bool apply() override;
	void reject() override;

	// toggles
	void setFieldEnabled(bool enabled);
	bool getFieldEnabled() const;

	void setInnerBoxEnabled(bool enabled);
	bool getInnerBoxEnabled() const;

	void setEnhancedEnabled(bool enabled);
	bool getEnhancedEnabled() const;

	// field types
	void setFieldType(field_type_t type);
	field_type_t getFieldType();

	void setDebrisGenre(debris_genre_t genre);
	debris_genre_t getDebrisGenre();

	// basic values
	void setNumAsteroids(int num_asteroids);
	int  getNumAsteroids() const;

	void setAvgSpeed(const QString& speed);
	QString& getAvgSpeed();

	// box values
	void setBoxText(const QString& text, _box_line_edits type);
	QString& getBoxText(_box_line_edits type);

	// object selections
	QVector<std::pair<QString, bool>> getAsteroidSelections() const;
	void setAsteroidSelections(const QVector<bool>& selected);

	QVector<std::pair<QString, bool>> getDebrisSelections() const;
	void setDebrisSelections(const QVector<bool>& selected);

	QVector<std::pair<QString, bool>> getShipSelections();
	void setShipSelections(const QVector<bool>& selected);

private:

	void initializeData();
	void update_internal_field();
	bool validate_data();
	void showErrorDialogNoCancel(const SCP_string& message);

	// boilerplate
	bool _bypass_errors;
	const int _MIN_BOX_THICKNESS = 400;

	// working copy of the asteroid field
	asteroid_field _a_field;

	// toggles
	bool  _enable_asteroids;
	bool  _enable_inner_bounds;
	bool  _enable_enhanced_checking;

	// field types
	field_type_t _field_type;     // active or passive
	debris_genre_t _debris_genre; // debris or asteroid

	// basic values
	int   _num_asteroids;
	QString _avg_speed;

	// box values
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

	// object selections
	SCP_vector<SCP_string>     _field_asteroid_type; // asteroid types
	SCP_vector<int>            _field_debris_type;   // debris types
	SCP_vector<SCP_string>     _field_target_names;  // target ships

	// Helper vectors for the checkbox dialog
	SCP_vector<SCP_string>                 asteroidOptions; // asteroid options for the checkbox dialog
	SCP_vector<std::pair<SCP_string, int>> debrisOptions;   // debris options for the checkbox dialog.. for this one we include the index in the pair so we can use it to map
	SCP_vector<SCP_string>                 shipOptions;     // ship options for the checkbox dialog
};

} // namespace fso::fred::dialogs
