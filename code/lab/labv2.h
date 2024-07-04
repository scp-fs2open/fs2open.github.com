#pragma once

enum class LabMode {
	Ship,
	Weapon,
	None
};

void lab_init();
void lab_close();
void lab_do_frame(float frametime);
