#pragma once

enum class LabMode {
	Object,
	Ship,
	Weapon,
	None
};

void lab_init();
void lab_close();
void lab_do_frame(float frametime);
