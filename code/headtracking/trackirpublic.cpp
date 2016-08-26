
#include "headtracking/trackirpublic.h"

TrackIRDLL::TrackIRDLL()
{
	/* Load the DLL and functions
	* If this is done globally, we'll be found by MSPDBDEBUGGING :)
	*/
	Reset();

	if (!LoadExternal(TRACKIRBRIDGEDLLNAME))
		return;

	m_Init = (SCPTRACKIR_PFINIT)LoadFunction("SCPTIR_Init");
	m_Close = (SCPTRACKIR_PFINTVOID)LoadFunction("SCPTIR_Close");
	m_Query = (SCPTRACKIR_PFINTVOID)LoadFunction("SCPTIR_Query");

	m_GetX = (SCPTRACKIR_PFFLOATVOID)LoadFunction("SCPTIR_GetX");
	m_GetY = (SCPTRACKIR_PFFLOATVOID)LoadFunction("SCPTIR_GetY");
	m_GetZ = (SCPTRACKIR_PFFLOATVOID)LoadFunction("SCPTIR_GetZ");
	m_GetRoll = (SCPTRACKIR_PFFLOATVOID)LoadFunction("SCPTIR_GetRoll");
	m_GetPitch = (SCPTRACKIR_PFFLOATVOID)LoadFunction("SCPTIR_GetPitch");
	m_GetYaw = (SCPTRACKIR_PFFLOATVOID)LoadFunction("SCPTIR_GetYaw");
	m_enabled = true;
}

TrackIRDLL::~TrackIRDLL()
{
}

bool TrackIRDLL::Enabled() const
{
	return m_enabled;
}

void TrackIRDLL::Reset()
{
	m_Init = NULL;
	m_Close = NULL;
	m_Query = NULL;
	m_GetX = NULL;
	m_GetY = NULL;
	m_GetZ = NULL;
	m_GetPitch = NULL;
	m_GetRoll = NULL;
	m_GetYaw = NULL;
	m_enabled = false;
}

int TrackIRDLL::Init(SDL_Window* window)
{
	if (m_Init)
	{
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version); // initialize info structure with SDL version info

		if (SDL_GetWindowWMInfo(window, &info))
		{ // the call returns true on success
			// success

			return m_Init(info.info.win.window);
		}
		else
		{
			// call failed
			mprintf(("Couldn't get window information: %s\n", SDL_GetError()));
			return 0;
		}
	}

	return -1;
}

int TrackIRDLL::Close()
{
	if (m_Close)
		return m_Close();
	return 0;
}

int TrackIRDLL::Query()
{
	if (m_Query)
		return m_Query();
	return 0;
}

float TrackIRDLL::GetX() const
{
	if (m_GetX)
		return m_GetX();
	return 0.0f;
}

float TrackIRDLL::GetY() const
{
	if (m_GetY)
		return m_GetY();
	return 0.0f;
}

float TrackIRDLL::GetZ() const
{
	if (m_GetZ)
		return m_GetZ();
	return 0.0f;
}

float TrackIRDLL::GetPitch() const
{
	if (m_GetPitch)
		return m_GetPitch();
	return 0.0f;
}

float TrackIRDLL::GetRoll() const
{
	if (m_GetRoll)
		return m_GetRoll();
	return 0.0f;
}

float TrackIRDLL::GetYaw() const
{
	if (m_GetYaw)
		return m_GetYaw();
	return 0.0f;
}
