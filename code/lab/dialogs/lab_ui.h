#pragma once

class LabUi {
  public:
	void createUi() const;
private:
	void buildShipList() const;
	void buildWeaponList() const;
	void buildBackgroundList() const;
	void showRenderOptions() const;
};