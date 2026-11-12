#pragma once

#include <QObject>

#include <mission/FredRenderer.h>

#include <array>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QToolBar;
class QToolButton;
class QWidget;

namespace fso::fred {

class Editor;
class EditorViewport;

/**
 * The bottom toolbar of the main window: the camera speeds, the IFF and the Local mode of the marked objects, the
 * X/Y/Z (or H/P/B) boxes of the current object, its radius, and its layer.
 *
 * A box keeps the focus while the user types. Enter applies the typed value to the edited axis only. Escape and all
 * other focus changes discard it.
 */
class TransformToolBar : public QObject {
	Q_OBJECT

  public:
	// Adds the widgets to toolbar. viewportWidget gets the focus back after an edit in a box.
	TransformToolBar(QToolBar* toolbar, Editor* editor, EditorViewport* viewport, QWidget* viewportWidget);

	// Updates the widgets from the editor state. Call this function on each idle tick.
	void update();

	// The layer list changed. The layer combo box rebuilds its list at the next update.
	void markLayersChanged();

	// Returns true if the object is one of the three boxes. The viewport must not take the focus from such a box.
	bool isTransformBox(const QObject* object) const;

	// Writes the Local preference of each mode to the settings
	void saveSettings() const;

  protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

  private:
	// The state when a box got the focus. Enter applies the typed value only if the current object and the mode
	// did not change, and only if the value is different from the start value.
	struct Edit {
		int object = -1;
		CursorMode mode = CursorMode::Selecting;
		double startValue = 0.0;
	};

	void addWidgets();
	void addFixedSpacer(int width);
	void updateBoxes(bool valid, bool rotateMode, bool editable);
	void updateLocalButton(bool editable);
	void updateCameraSpeeds();
	void updateIff(bool valid, bool isShip, int numMarked);
	void updateRadius(bool valid, int numMarked);
	void updateLayer(bool valid, int numMarked);

	// Returns 0, 1, or 2 for a box, and -1 for all other objects
	int boxAxis(const QObject* object) const;
	bool editIsCurrent() const;
	void applyValue(int axis, double value);

	QToolBar* _toolbar;
	Editor* _editor;
	EditorViewport* _viewport;
	QWidget* _viewportWidget;

	QComboBox* _moveSpeedCombo = nullptr;
	QComboBox* _rotSpeedCombo = nullptr;
	QComboBox* _iffCombo = nullptr;
	QToolButton* _localButton = nullptr;
	QLabel* _modeLabel = nullptr;
	std::array<QLabel*, 3> _axisLabels{};
	std::array<QDoubleSpinBox*, 3> _boxes{};
	QLabel* _radiusLabel = nullptr;
	QComboBox* _layerCombo = nullptr;

	Edit _edit;
	bool _layersChanged = true;  // rebuild the layer list only when the layers change
	bool _iffPopulated = false;  // the IFF list fills later, because the tables load after the toolbar
	bool _localMove = false;     // the Local preference in move mode
	bool _localRotate = false;   // the Local preference in rotate mode
	int _cachedMode = -1;        // -1 forces the restore of the Local preference at the first update
};

} // namespace fso::fred
