#pragma once

class LabUi {
  public:
	void createUi();
	void objectChanged();
	void closeUi();

  private:
	void buildShipList() const;
	void buildWeaponList() const;
	void buildBackgroundList() const;
	void showRenderOptions() const;
	void showObjectOptions() const;

	bool rebuildAfterObjectChange = false;
};