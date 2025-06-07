
#include "globalincs/utility.h"
#include "math/curve.h"
#include "parse/parselo.h"

SCP_vector<Curve> Curves;

int curve_get_by_name(const SCP_string& in_name) {
	return find_item_with_field(Curves, &Curve::name, in_name);
}

static int curve_inline_def(){
	//The curve needs a unique identifier. Build this with semicolon + filename + semicolon + character in file
	SCP_string curve_name = ';' + SCP_string(Current_filename) + ';' + std::to_string(Mp - Parse_text);

	int resulting_curve = static_cast<int>(Curves.size());
	Curve& new_curve = Curves.emplace_back(curve_name);

	do {
		curve_keyframe& new_keyframe = new_curve.keyframes.emplace_back();
		stuff_float(&new_keyframe.pos.x);
		stuff_float(&new_keyframe.pos.y);
		required_string(")");

		bool found_interpolation = true;
		int interpolation_mode = optional_string_one_of(4, "--", "-|", "-/", "/-");
		switch (interpolation_mode) {
		case 0:
			new_keyframe.interp_func = CurveInterpFunction::Linear;
			break;
		case 1:
			new_keyframe.interp_func = CurveInterpFunction::Constant;
			break;
		case 2:
			new_keyframe.interp_func = CurveInterpFunction::Polynomial;
			new_keyframe.param1 = 2.0f;
			new_keyframe.param2 = 1.0f;
			break;
		case 3:
			new_keyframe.interp_func = CurveInterpFunction::Polynomial;
			new_keyframe.param1 = 2.0f;
			new_keyframe.param2 = -1.0f;
			break;
		default:
			new_keyframe.interp_func = CurveInterpFunction::Linear;
			found_interpolation = false;
			break;
		}
		if (!found_interpolation)
			break;
	} while (optional_string("("));

	return resulting_curve;
}

int curve_parse(const char *err_msg) {
	if(optional_string("(")) {
		return curve_inline_def();
	}
	else {
		SCP_string curve_name;
		stuff_string(curve_name, F_NAME, "()");
		int curve_id = curve_get_by_name(curve_name);
		if (curve_id < 0) {
			error_display(0, "Curve %s not found!%s", curve_name.c_str(), err_msg);
		}
		return curve_id;
	}
}

void parse_curve_table(const char* filename) {
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		required_string("#Curves");

		while (optional_string("$Name:")) {
			SCP_string name;
			stuff_string(name, F_NAME);

			if (name.find('(') != SCP_string::npos || name.find(')') != SCP_string::npos) {
				error_display(0, "Curve name %s contains parentheses, which are not permitted.", name.c_str());
			}

			int index = curve_get_by_name(name);

			if (index < 0) {
				Curve curv = Curve(std::move(name));
				curv.ParseData();
				Curves.push_back(curv);
			} else {
				Curves[index].ParseData();
			}
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
	}
}

void fill_default_curves() {
	for (int i = 1; i < 6; i++) {
		for (int rev = 0; rev < 2; rev++) {
			SCP_string name("EaseIn");
			switch (i) {
			case 1: name += "Circ"; break;
			case 2: name += "Quad"; break;
			case 3: name += "Cubic"; break;
			case 4: name += "Quart"; break;
			case 5: name += "Quint"; break;
			}
			if (rev)
				name += "Rev";

			Curves.emplace_back(name);
			if (i == 1)
				Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 0.0f, rev ? 1.0f : 0.0f}, CurveInterpFunction::Circular, 0.0f, 1.0f });
			else
				Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 0.0f, rev ? 1.0f : 0.0f}, CurveInterpFunction::Polynomial, (float)i, 1.0f });
			Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 1.0f, rev ? 0.0f : 1.0f}, CurveInterpFunction::Constant, 0.0f, 0.0f });

			name = "EaseOut";
			switch (i) {
			case 1: name += "Circ"; break;
			case 2: name += "Quad"; break;
			case 3: name += "Cubic"; break;
			case 4: name += "Quart"; break;
			case 5: name += "Quint"; break;
			}
			if (rev)
				name += "Rev";

			Curves.emplace_back(name);
			if (i == 1)
				Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 0.0f, rev ? 1.0f : 0.0f}, CurveInterpFunction::Circular, 0.0f, -1.0f });
			else
				Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 0.0f, rev ? 1.0f : 0.0f}, CurveInterpFunction::Polynomial, (float)i, -1.0f });
			Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 1.0f, rev ? 0.0f : 1.0f}, CurveInterpFunction::Constant, 0.0f, 0.0f });

			name = "EaseInOut";
			switch (i) {
			case 1: name += "Circ"; break;
			case 2: name += "Quad"; break;
			case 3: name += "Cubic"; break;
			case 4: name += "Quart"; break;
			case 5: name += "Quint"; break;
			}
			if (rev)
				name += "Rev";

			Curves.emplace_back(name);

			if (i == 1) {
				Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 0.0f, rev ? 1.0f : 0.0f}, CurveInterpFunction::Circular, 0.0f, 1.0f });
				Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 0.5f, 0.5f}, CurveInterpFunction::Circular, 0.0f, -1.0f });
			} else {
				Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 0.0f, rev ? 1.0f : 0.0f}, CurveInterpFunction::Polynomial, (float)i, 1.0f });
				Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 0.5f, 0.5f}, CurveInterpFunction::Polynomial, (float)i, -1.0f });
			}
			Curves.back().keyframes.push_back(curve_keyframe{ vec2d { 1.0f, rev ? 0.0f : 1.0f}, CurveInterpFunction::Constant, 0.0f, 0.0f });
		}
	}
}

void curves_init() {

	Curves.clear();
	fill_default_curves();

	if (cf_exists_full("curves.tbl", CF_TYPE_TABLES))
		parse_curve_table("curves.tbl");

	parse_modular_table(NOX("*-crv.tbm"), parse_curve_table);
}

Curve::Curve(SCP_string in_name)
{
	name = std::move(in_name);
}

void Curve::ParseData()
{
	keyframes.clear();

	required_string("$Keyframes:");
	do
	{
		bool valid_keyframe = true;
		curve_keyframe kframe;
		stuff_parenthesized_vec2d(&kframe.pos);
		required_string(":");

		SCP_string type;
		stuff_string(type, F_NAME, ",");

		if (type == "Constant") {
			kframe.interp_func = CurveInterpFunction::Constant;
		} else if (type == "Linear") {
			kframe.interp_func = CurveInterpFunction::Linear;
		} else if (type == "Polynomial") {
			kframe.interp_func = CurveInterpFunction::Polynomial;

			required_string(",");
			if (stuff_float(&kframe.param1)) {
				bool ease_in;
				stuff_boolean(&ease_in);
				if (ease_in)
					kframe.param2 = 1.0f;
				else
					kframe.param2 = -1.0f;
			} else {
				kframe.param2 = 1.0;
			}
		} else if (type == "Circular") {
			kframe.interp_func = CurveInterpFunction::Circular;

			if (optional_string(",")) {
				bool ease_in;
				stuff_boolean(&ease_in);
				if (ease_in)
					kframe.param2 = 1.0f;
				else
					kframe.param2 = -1.0f;
			} else {
				kframe.param2 = 1.0f;
			}
		} else {
			int index = curve_get_by_name(type);

			if (index < 0) {
				Warning(LOCATION, "Unrecognized interpolation function '%s' used in '%s'. Remember that any other curves used in this curve must have been defined before this one.", 
					type.c_str(), name.c_str());
				valid_keyframe = false;
			} else {
				kframe.interp_func = CurveInterpFunction::Curve;
				kframe.param1 = (float)index;
			}
		}

		if (valid_keyframe && keyframes.size() > 0 && kframe.pos.x < keyframes.back().pos.x) {
			Warning(LOCATION, "The keyframe (%f,%f) of Curve '%s' has a smaller x value than the previous keyframe. They must increase in order. Skipping this keyframe...", 
				kframe.pos.x, kframe.pos.y, name.c_str());
		} else {
			keyframes.push_back(kframe);
		}

		if (optional_string("#End"))
			return;

	} while (!check_for_string("$Name:"));

	if (keyframes.back().pos.x < 1.0f) {
		Warning(LOCATION, "The last keyframe of Curve '%s' has an x value less then 1. A (1, 1) point has been added.", name.c_str());
		curve_keyframe kframe;
		kframe.interp_func = CurveInterpFunction::Constant;
		kframe.pos.x = 1.0f;
		kframe.pos.y = 1.0f;
		keyframes.push_back(kframe);
	}
}


float Curve::GetValue(float x_val) const {
	const curve_keyframe* kframe = &keyframes[0];
	const vec2d* next_pos = &kframe->pos;
	for (size_t i = 0; i < keyframes.size(); i++) {
		kframe = &keyframes[i];
		if (x_val < kframe->pos.x) {
			if (i == 0) {
				return kframe->pos.y; // 'stretch' the initial y value back to -infinity
			} else {
				next_pos = &kframe->pos;
				kframe = &keyframes[i-1];
				break;
			}
		}

		if (i == keyframes.size() - 1) {
			return kframe->pos.y; // 'stretch' the final y value forward to infinity
		}
	}

	float t = (x_val - kframe->pos.x) / (next_pos->x - kframe->pos.x);
	float out;
	switch (kframe->interp_func) {
		case CurveInterpFunction::Constant:
			return kframe->pos.y;
		case CurveInterpFunction::Linear:
			return kframe->pos.y + t * (next_pos->y - kframe->pos.y);
		case CurveInterpFunction::Polynomial:
			out = kframe->param2 > 0.0f ? powf(t, kframe->param1) : (1 - powf(1 - t, kframe->param1));
			return kframe->pos.y + out * (next_pos->y - kframe->pos.y);
		case CurveInterpFunction::Circular:
			out = kframe->param2 > 0.0f ? 1.0f - sqrtf(1 - powf(t, 2.0f)) : sqrtf(1.0f - powf(t - 1.0f, 2.0f));
			return kframe->pos.y + out * (next_pos->y - kframe->pos.y);
		case CurveInterpFunction::Curve:
			// add 0.5 to ensure this behaves like rounding
			out = Curves[fl2i(kframe->param1 + 0.5f)].GetValue(t);
			return kframe->pos.y + out * (next_pos->y - kframe->pos.y);
		default:
			UNREACHABLE("Unrecognized curve function");
			return 0.0f;
	}
}

float Curve::GetValueIntegrated(float x_val) const
{
	float integrated_value = 0.f;

	if (x_val < keyframes[0].pos.x) {
		return (keyframes[0].pos.x - x_val) * keyframes[0].pos.y;
	}

	for (size_t i = 0; i < keyframes.size(); i++) {
		const curve_keyframe* kframe = &keyframes[i];
		if (i == keyframes.size() - 1) {
			integrated_value += (x_val - kframe->pos.x) * kframe->pos.y;
			break;
		}
		const vec2d* next_pos = &keyframes[i+1].pos;
		float end_x = MIN(x_val, next_pos->x);
		bool last_keyframe = false;
		if (x_val <= next_pos->x) {
			last_keyframe = true;
		}
		float t = (end_x - kframe->pos.x) / (next_pos->x - kframe->pos.x);
		float m = (next_pos->y - kframe->pos.y) * (next_pos->x - kframe->pos.x);
		float out;
		integrated_value += (end_x - kframe->pos.x) * kframe->pos.y;
		switch (kframe->interp_func) {
		case CurveInterpFunction::Constant:
			break;
		case CurveInterpFunction::Linear:
			integrated_value += t * t * m * 0.5f;
			break;
		case CurveInterpFunction::Polynomial:
			out = kframe->param2 > 0.0f ? powf(t, kframe->param1 + 1.f) : (powf(1 - t, kframe->param1 + 1.f) + (kframe->param1 * t) + t - 1.f);
			integrated_value += (out * m)/(kframe->param1 + 1.f);
			break;
		case CurveInterpFunction::Circular:
			if (kframe->param2 > 0.0f) {
				integrated_value += m * (0.5f * (-sqrtf(1.f - (t * t)) * t - asin(t)) + t);
			} else {
				integrated_value += m * (0.5f * sqrtf(-(t - 2) * t) * (t - 1) + asin(sqrtf(t) / sqrtf(2.f)));
			}
			break;
		case CurveInterpFunction::Curve:
			// add 0.5 to ensure this behaves like rounding
			integrated_value += m * (Curves[fl2i(kframe->param1 + 0.5f)].GetValueIntegrated(t) - Curves[fl2i(kframe->param1 + 0.5f)].GetValueIntegrated(0.f));
			break;
		default:
			UNREACHABLE("Unrecognized curve function");
			break;
		}
		if (last_keyframe) {
			break;
		}
	}

	return integrated_value;
}
