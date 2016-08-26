
#include "headtracking/trackir.h"

#include "headtracking/trackirpublic.h"

#include "osapi/osapi.h"

#include <SDL_syswm.h>

namespace headtracking
{
	namespace trackir
	{
		TrackIRProvider::TrackIRProvider()
		{
			auto window = os::getSDLMainWindow();
			if (window == nullptr)
			{
				throw internal::HeadTrackingException("TrackIR is only available with a valid window!");
			}

			// calling the function that will init all the function pointers for TrackIR stuff (Swifty)
			int trackIrInitResult = _trackIRDll.Init(window);
			if (trackIrInitResult != SCP_INITRESULT_SUCCESS)
			{
				mprintf(("TrackIR Init Failed - %d\n", trackIrInitResult));
				throw internal::HeadTrackingException("Failed to initialize TrackIR");
			}
		}

		TrackIRProvider::~TrackIRProvider()
		{
		}

		bool TrackIRProvider::query(HeadTrackingStatus* statusOut)
		{
			_trackIRDll.Query();

			statusOut->pitch = _trackIRDll.GetPitch();
			statusOut->roll = _trackIRDll.GetRoll();
			statusOut->yaw = _trackIRDll.GetYaw();

			statusOut->x = _trackIRDll.GetX();
			statusOut->y = _trackIRDll.GetY();
			statusOut->z = _trackIRDll.GetZ();

			return true;
		}

		SCP_string TrackIRProvider::getName()
		{
			return "TrackIR";
		}
	}
}
