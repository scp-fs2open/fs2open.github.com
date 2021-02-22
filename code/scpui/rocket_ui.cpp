//
//

// Undefine stupid windows API defines that get included by the prefix headers
#ifdef GetNextSibling
#undef GetNextSibling
#endif

#ifdef GetFirstChild
#undef GetFirstChild
#endif

#include "scpui/rocket_ui.h"

#include "cfile/cfile.h"
#include "mod_table/mod_table.h"
#include "osapi/osapi.h"
#include "scpui/IncludeNodeHandler.h"
#include "scpui/RocketFileInterface.h"
#include "scpui/RocketLuaSystemInterface.h"
#include "scpui/RocketRenderingInterface.h"
#include "scpui/RocketSystemInterface.h"
#include "scpui/SoundPlugin.h"
#include "scpui/elements/AnimationElement.h"
#include "scpui/elements/ScrollingTextElement.h"
#include "scripting/scripting.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include <Rocket/Controls.h>
#include <Rocket/Core.h>

#include <Rocket/Debugger.h>

#include <Rocket/Controls/Lua/Controls.h>
#include <Rocket/Core/Lua/Interpreter.h>

#pragma pop_macro("Assert")

#include <codecvt>
#include <locale>

using namespace Rocket::Core;

namespace {
bool rocket_initialized = false;

std::unique_ptr<scpui::RocketRenderingInterface> rendering_interface;
std::unique_ptr<scpui::RocketSystemInterface> system_interface;
std::unique_ptr<scpui::RocketFileInterface> file_interface;

Context* input_context = nullptr;
vec2d render_offset    = {0.f, 0.f};

bool debugger_initialized = false;

bool transform_to_rocket(vec2d& in)
{
	in = {in.x - render_offset.x, in.y - render_offset.y};

	if (in.x < 0.f || in.y < 0.f) {
		// Out of bounds
		return false;
	}

	auto dims = input_context->GetDimensions();
	if (in.x > dims.x || in.y > dims.y) {
		return false;
	}

	return true;
}

int get_modifier_state()
{
	int mods = 0;

	auto sdl_mods = SDL_GetModState();

	if (sdl_mods & KMOD_CTRL) {
		mods |= Input::KM_CTRL;
	}

	if (sdl_mods & KMOD_SHIFT) {
		mods |= Input::KM_SHIFT;
	}

	if (sdl_mods & KMOD_ALT) {
		mods |= Input::KM_ALT;
	}

	if (sdl_mods & KMOD_GUI) {
		mods |= Input::KM_META;
	}

	if (sdl_mods & KMOD_CAPS) {
		mods |= Input::KM_CAPSLOCK;
	}

	if (sdl_mods & KMOD_NUM) {
		mods |= Input::KM_NUMLOCK;
	}

	auto states = SDL_GetKeyboardState(nullptr);
	if (states[SDL_SCANCODE_SCROLLLOCK]) {
		mods |= Input::KM_SCROLLLOCK;
	}

	return mods;
}

void load_fonts()
{
	SCP_vector<SCP_string> files;
	cf_get_file_list(files, CF_TYPE_FONT, "*.ttf");

	for (auto& name : files) {
		// The file extension gets removed by CFile
		FontDatabase::LoadFontFace((name + ".ttf").c_str());
	}

	files.clear();
	cf_get_file_list(files, CF_TYPE_FONT, "*.otf");

	for (auto& name : files) {
		// The file extension gets removed by CFile
		FontDatabase::LoadFontFace((name + ".otf").c_str());
	}
}

bool mouse_motion_handler(const SDL_Event& evt)
{
	if (!os::events::isWindowEvent(evt, os::getSDLMainWindow())) {
		// Ignore events that are not relevant to the main window
		return false;
	}

	if (input_context == nullptr) {
		// No active input context
		return false;
	}

	vec2d evt_pos = {(float)evt.motion.x, (float)evt.motion.y};
	if (!transform_to_rocket(evt_pos)) {
		// Out of bounds
		return false;
	}

	input_context->ProcessMouseMove((int)evt_pos.x, (int)evt_pos.y, get_modifier_state());

	return true;
}

bool mouse_button_handler(const SDL_Event& evt)
{
	if (!os::events::isWindowEvent(evt, os::getSDLMainWindow())) {
		// Ignore events that are not relevant to the main window
		return false;
	}

	if (input_context == nullptr) {
		// No active input context
		return false;
	}

	vec2d evt_pos = {(float)evt.button.x, (float)evt.button.y};
	if (!transform_to_rocket(evt_pos)) {
		// Out of bounds
		return false;
	}

	int button;
	switch (evt.button.button) {
	case SDL_BUTTON_LEFT:
		button = 0;
		break;
	case SDL_BUTTON_MIDDLE:
		button = 2;
		break;
	case SDL_BUTTON_RIGHT:
		button = 1;
		break;
	case SDL_BUTTON_X1:
		button = 3;
		break;
	case SDL_BUTTON_X2:
		button = 4;
		break;
	default:
		// Unknown SDL button value
		mprintf(("Unknown SDL button value %d in libRocket mouse handler!\n", evt.button.button));
		return false;
	}

	if (evt.type == SDL_MOUSEBUTTONUP) {
		// Up
		input_context->ProcessMouseButtonUp(button, get_modifier_state());
	} else {
		// Down
		input_context->ProcessMouseButtonDown(button, get_modifier_state());
	}

	return true;
}

bool mouse_wheel_handler(const SDL_Event& evt)
{
	if (!os::events::isWindowEvent(evt, os::getSDLMainWindow())) {
		// Ignore events that are not relevant to the main window
		return false;
	}

	if (input_context == nullptr) {
		// No active input context
		return false;
	}

	int wheel_value = evt.wheel.y;
#if SDL_VERSION_ATLEAST(2, 0, 4)
	if (evt.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
		wheel_value *= -1;
	}
#endif

	// libRocket expects a different direction than what SDL delivers
	input_context->ProcessMouseWheel(-wheel_value, get_modifier_state());

	return true;
}

Input::KeyIdentifier translateKey(SDL_Keycode Key)
{
	switch (Key) {
	case SDLK_a:
		return Rocket::Core::Input::KI_A;
	case SDLK_b:
		return Rocket::Core::Input::KI_B;
	case SDLK_c:
		return Rocket::Core::Input::KI_C;
	case SDLK_d:
		return Rocket::Core::Input::KI_D;
	case SDLK_e:
		return Rocket::Core::Input::KI_E;
	case SDLK_f:
		return Rocket::Core::Input::KI_F;
	case SDLK_g:
		return Rocket::Core::Input::KI_G;
	case SDLK_h:
		return Rocket::Core::Input::KI_H;
	case SDLK_i:
		return Rocket::Core::Input::KI_I;
	case SDLK_j:
		return Rocket::Core::Input::KI_J;
	case SDLK_k:
		return Rocket::Core::Input::KI_K;
	case SDLK_l:
		return Rocket::Core::Input::KI_L;
	case SDLK_m:
		return Rocket::Core::Input::KI_M;
	case SDLK_n:
		return Rocket::Core::Input::KI_N;
	case SDLK_o:
		return Rocket::Core::Input::KI_O;
	case SDLK_p:
		return Rocket::Core::Input::KI_P;
	case SDLK_q:
		return Rocket::Core::Input::KI_Q;
	case SDLK_r:
		return Rocket::Core::Input::KI_R;
	case SDLK_s:
		return Rocket::Core::Input::KI_S;
	case SDLK_t:
		return Rocket::Core::Input::KI_T;
	case SDLK_u:
		return Rocket::Core::Input::KI_U;
	case SDLK_v:
		return Rocket::Core::Input::KI_V;
	case SDLK_w:
		return Rocket::Core::Input::KI_W;
	case SDLK_x:
		return Rocket::Core::Input::KI_X;
	case SDLK_y:
		return Rocket::Core::Input::KI_Y;
	case SDLK_z:
		return Rocket::Core::Input::KI_Z;
	case SDLK_0:
		return Rocket::Core::Input::KI_0;
	case SDLK_1:
		return Rocket::Core::Input::KI_1;
	case SDLK_2:
		return Rocket::Core::Input::KI_2;
	case SDLK_3:
		return Rocket::Core::Input::KI_3;
	case SDLK_4:
		return Rocket::Core::Input::KI_4;
	case SDLK_5:
		return Rocket::Core::Input::KI_5;
	case SDLK_6:
		return Rocket::Core::Input::KI_6;
	case SDLK_7:
		return Rocket::Core::Input::KI_7;
	case SDLK_8:
		return Rocket::Core::Input::KI_8;
	case SDLK_9:
		return Rocket::Core::Input::KI_9;
	case SDLK_KP_0:
		return Rocket::Core::Input::KI_NUMPAD0;
	case SDLK_KP_1:
		return Rocket::Core::Input::KI_NUMPAD1;
	case SDLK_KP_2:
		return Rocket::Core::Input::KI_NUMPAD2;
	case SDLK_KP_3:
		return Rocket::Core::Input::KI_NUMPAD3;
	case SDLK_KP_4:
		return Rocket::Core::Input::KI_NUMPAD4;
	case SDLK_KP_5:
		return Rocket::Core::Input::KI_NUMPAD5;
	case SDLK_KP_6:
		return Rocket::Core::Input::KI_NUMPAD6;
	case SDLK_KP_7:
		return Rocket::Core::Input::KI_NUMPAD7;
	case SDLK_KP_8:
		return Rocket::Core::Input::KI_NUMPAD8;
	case SDLK_KP_9:
		return Rocket::Core::Input::KI_NUMPAD9;
	case SDLK_LEFT:
		return Rocket::Core::Input::KI_LEFT;
	case SDLK_RIGHT:
		return Rocket::Core::Input::KI_RIGHT;
	case SDLK_UP:
		return Rocket::Core::Input::KI_UP;
	case SDLK_DOWN:
		return Rocket::Core::Input::KI_DOWN;
	case SDLK_KP_PLUS:
		return Rocket::Core::Input::KI_ADD;
	case SDLK_BACKSPACE:
		return Rocket::Core::Input::KI_BACK;
	case SDLK_DELETE:
		return Rocket::Core::Input::KI_DELETE;
	case SDLK_KP_DIVIDE:
		return Rocket::Core::Input::KI_DIVIDE;
	case SDLK_END:
		return Rocket::Core::Input::KI_END;
	case SDLK_ESCAPE:
		return Rocket::Core::Input::KI_ESCAPE;
	case SDLK_F1:
		return Rocket::Core::Input::KI_F1;
	case SDLK_F2:
		return Rocket::Core::Input::KI_F2;
	case SDLK_F3:
		return Rocket::Core::Input::KI_F3;
	case SDLK_F4:
		return Rocket::Core::Input::KI_F4;
	case SDLK_F5:
		return Rocket::Core::Input::KI_F5;
	case SDLK_F6:
		return Rocket::Core::Input::KI_F6;
	case SDLK_F7:
		return Rocket::Core::Input::KI_F7;
	case SDLK_F8:
		return Rocket::Core::Input::KI_F8;
	case SDLK_F9:
		return Rocket::Core::Input::KI_F9;
	case SDLK_F10:
		return Rocket::Core::Input::KI_F10;
	case SDLK_F11:
		return Rocket::Core::Input::KI_F11;
	case SDLK_F12:
		return Rocket::Core::Input::KI_F12;
	case SDLK_F13:
		return Rocket::Core::Input::KI_F13;
	case SDLK_F14:
		return Rocket::Core::Input::KI_F14;
	case SDLK_F15:
		return Rocket::Core::Input::KI_F15;
	case SDLK_HOME:
		return Rocket::Core::Input::KI_HOME;
	case SDLK_INSERT:
		return Rocket::Core::Input::KI_INSERT;
	case SDLK_LCTRL:
		return Rocket::Core::Input::KI_LCONTROL;
	case SDLK_LSHIFT:
		return Rocket::Core::Input::KI_LSHIFT;
	case SDLK_KP_MULTIPLY:
		return Rocket::Core::Input::KI_MULTIPLY;
	case SDLK_PAUSE:
		return Rocket::Core::Input::KI_PAUSE;
	case SDLK_RCTRL:
		return Rocket::Core::Input::KI_RCONTROL;
	case SDLK_RETURN:
		return Rocket::Core::Input::KI_RETURN;
	case SDLK_RSHIFT:
		return Rocket::Core::Input::KI_RSHIFT;
	case SDLK_SPACE:
		return Rocket::Core::Input::KI_SPACE;
	case SDLK_KP_MINUS:
		return Rocket::Core::Input::KI_SUBTRACT;
	case SDLK_TAB:
		return Rocket::Core::Input::KI_TAB;
	};

	return Rocket::Core::Input::KI_UNKNOWN;
}

bool key_event_handler(const SDL_Event& evt)
{
	if (!os::events::isWindowEvent(evt, os::getSDLMainWindow())) {
		// Ignore events that are not relevant to the main window
		return false;
	}

	if (input_context == nullptr) {
		// No active input context
		return false;
	}

	// Check for special key combinations
	auto sdl_mods = SDL_GetModState();

	auto shift_down = (sdl_mods & KMOD_SHIFT) != 0;

	if (evt.key.keysym.sym == SDLK_ESCAPE && shift_down) {
		// Ignore this combination to still enable quitting
		return false;
	}
	if (evt.key.keysym.sym == SDLK_PRINTSCREEN) {
		// Ignore print screen buttons to still allow the default engine actions
		return false;
	}
	if (evt.key.keysym.sym == SDLK_LSHIFT || evt.key.keysym.sym == SDLK_RSHIFT || evt.key.keysym.sym == SDLK_LCTRL ||
	    evt.key.keysym.sym == SDLK_RCTRL||evt.key.keysym.sym == SDLK_LALT||evt.key.keysym.sym == SDLK_RALT) {
		// Ignore simple key events of modifiers since they are not meaningful to libRocket
		return false;
	}

	if (evt.type == SDL_KEYUP) {
		auto ctrl_down = (sdl_mods & KMOD_CTRL) != 0;

		if (evt.key.keysym.sym == SDLK_r && ctrl_down && shift_down) {
			// Ctrl+Shift+R reloads everything
			scpui::reloadAllContexts();

			return true;
		} else if (evt.key.keysym.sym == SDLK_d && ctrl_down && shift_down) {
			// Ctrl+Shift+D toggles the debugger
			if (!debugger_initialized) {
				// First we need to initialize the debugger. We use the current context for that, should work in
				// most cases
				if (!Rocket::Debugger::Initialise(input_context)) {
					Warning(LOCATION, "Failed to initialize debugger!");
					return true;
				}

				debugger_initialized = true;
			}

			Rocket::Debugger::SetContext(input_context);

			// Toggle the debugger state
			Rocket::Debugger::SetVisible(!Rocket::Debugger::IsVisible());

			return true;
		}
	}

	if (evt.type == SDL_KEYDOWN) {
		return input_context->ProcessKeyDown(translateKey(evt.key.keysym.sym), get_modifier_state());
	} else {
		return input_context->ProcessKeyUp(translateKey(evt.key.keysym.sym), get_modifier_state());
	}
}
#if _MSC_VER >= 1900
std::u16string utf8_to_utf16(const char* utf8_string)
{
	std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
	auto intString = convert.from_bytes(utf8_string);

	return std::u16string(reinterpret_cast<const char16_t*>(intString.data()));
}
#else
std::u16string utf8_to_utf16(const char* utf8_string)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.from_bytes(utf8_string);
}
#endif

bool text_input_handler(const SDL_Event& evt)
{
	if (!os::events::isWindowEvent(evt, os::getSDLMainWindow())) {
		// Ignore events that are not relevant to the main window
		return false;
	}

	if (input_context == nullptr) {
		// No active input context
		return false;
	}

	// libRocket expects UCS-2 so we first need to convert to that
	auto ucs2String = utf8_to_utf16(evt.text.text);

	bool consumed = true;
	for (auto& c : ucs2String) {
		consumed = consumed && input_context->ProcessTextInput(c);
	}

	return consumed;
}

} // namespace

namespace scpui {

void initialize()
{
	if (!Unicode_text_mode) {
		mprintf(("NOTE: libRocket is disabled since unicode text mode is not enabled!\n"));
		return;
	}

	rendering_interface.reset(new RocketRenderingInterface());
	system_interface.reset(new RocketSystemInterface());
	file_interface.reset(new RocketFileInterface());

	rendering_interface->setRenderOffset(render_offset);

	// Setup our custom interfaces
	Rocket::Core::SetRenderInterface(rendering_interface.get());
	Rocket::Core::SetSystemInterface(system_interface.get());
	Rocket::Core::SetFileInterface(file_interface.get());

	Rocket::Core::Initialise();
	// Initialise the Rocket Controls library.
	Rocket::Controls::Initialise();

	Rocket::Core::Factory::RegisterElementInstancer("ani", new ElementInstancerGeneric<elements::AnimationElement>())
		->RemoveReference();
	Rocket::Core::Factory::RegisterElementInstancer("scrollingText",
		new ElementInstancerGeneric<elements::ScrollingTextElement>())
		->RemoveReference();

	XMLParser::RegisterNodeHandler("include", new IncludeNodeHandler())->RemoveReference();

	// Setup the plugin a style sheet properties for the sound support
	Rocket::Core::RegisterPlugin(new SoundPlugin());
	Rocket::Core::StyleSheetSpecification::RegisterParser("sound", new SoundPropertyParser());
	for (auto& type : SoundPlugin::getEventTypes()) {
		auto prop_name = type + "-sound";
		Rocket::Core::StyleSheetSpecification::RegisterProperty(prop_name.c_str(), "-1", false).AddParser("sound");
	}

	// Initialise the Lua interface
	Rocket::Core::Lua::Interpreter::Initialise(Script_system.GetLuaSession(),
		std::unique_ptr<Rocket::Core::Lua::LuaSystemInterface>(new RocketLuaSystemInterface()));
	Rocket::Controls::Lua::RegisterTypes(Rocket::Core::Lua::Interpreter::GetLuaState());

	load_fonts();

	// initialize events
	os::events::addEventListener(SDL_MOUSEMOTION, os::events::DEFAULT_LISTENER_WEIGHT - 10, mouse_motion_handler);
	os::events::addEventListener(SDL_MOUSEBUTTONUP, os::events::DEFAULT_LISTENER_WEIGHT - 10, mouse_button_handler);
	os::events::addEventListener(SDL_MOUSEBUTTONDOWN, os::events::DEFAULT_LISTENER_WEIGHT - 10, mouse_button_handler);
	os::events::addEventListener(SDL_MOUSEWHEEL, os::events::DEFAULT_LISTENER_WEIGHT - 10, mouse_wheel_handler);

	os::events::addEventListener(SDL_KEYUP, os::events::DEFAULT_LISTENER_WEIGHT - 10, key_event_handler);
	os::events::addEventListener(SDL_KEYDOWN, os::events::DEFAULT_LISTENER_WEIGHT - 10, key_event_handler);

	os::events::addEventListener(SDL_TEXTINPUT, os::events::DEFAULT_LISTENER_WEIGHT - 10, text_input_handler);

	rocket_initialized = true;
}
void setOffset(float x, float y) { rendering_interface->setRenderOffset({x, y}); }

void shutdown_scripting()
{
	if (!rocket_initialized) {
		return;
	}
	Rocket::Core::Lua::Interpreter::Shutdown(false);
}

void shutdown()
{
	if (!rocket_initialized) {
		return;
	}

	if (input_context != nullptr) {
		// If this is still here then the user forgot to disable input
		mprintf(("Warning: The input context is still active! Disabling input...\n"));
		disableInput();
	}

	// Shutdown Rocket.
	Rocket::Core::Shutdown();

	rendering_interface.reset();
	system_interface.reset();
	file_interface.reset();

	rocket_initialized = false;
}

void reloadContext(Rocket::Core::Context* context)
{
	struct DocumentStatus {
		ElementDocument* original = nullptr;
		String file_name;
		bool visible = true;
		bool focus   = false;
	};

	// Save the current status of the documents in this context
	SCP_vector<DocumentStatus> documents;
	for (auto i = 0; i < context->GetNumDocuments(); ++i) {
		ElementDocument* doc = context->GetDocument(i);
		if (doc->GetSourceURL() == "") {
			// In-memory documents can't be reloaded
			continue;
		}

		DocumentStatus status;
		status.original  = doc;
		status.file_name = doc->GetSourceURL();
		status.visible   = doc->IsVisible();
		status.focus     = doc->GetFocusLeafNode() != nullptr;

		documents.push_back(status);
	}

	Factory::ClearStyleSheetCache();
	Factory::ClearTemplateCache();

	// And readd them all
	for (auto& status : documents) {
		// Selectively unload the old documents.
		// We can't just unload all since that destroys the debugger
		context->UnloadDocument(status.original);

		auto doc = context->LoadDocument(status.file_name);
		if (status.visible) {
			doc->Show(status.focus ? ElementDocument::FOCUS : ElementDocument::NONE);
		}
		// We don't own this reference
		doc->RemoveReference();
	}
}

void reloadAllContexts()
{
	for (auto i = 0; i < Rocket::Core::GetNumContexts(); ++i) {
		reloadContext(Rocket::Core::GetContext(i));
	}
}

void enableInput(Rocket::Core::Context* main_ctx)
{
	Assertion(main_ctx != nullptr, "Invalid context pointer detected!");

	if (input_context != nullptr) {
		// If we still have an active context, disable the input on the previous context
		disableInput();
	}

	input_context = main_ctx;
	input_context->AddReference();
}
void disableInput()
{
	if (input_context == nullptr) {
		return;
	}

	input_context->RemoveReference();
	input_context = nullptr;
}

} // namespace scpui
