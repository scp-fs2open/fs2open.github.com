
#ifndef HEADTRACKING_TRACKIR_H
#define HEADTRACKING_TRACKIR_H

#ifdef _WIN32

#include "headtracking/headtracking.h"
#include "headtracking/headtracking_internal.h"
#include "headtracking/trackirpublic.h"

namespace headtracking
{
	namespace trackir
	{
		class TrackIRProvider : public internal::HeadTrackingProvider
		{
			TrackIRDLL _trackIRDll;
		public:
			TrackIRProvider();
			virtual ~TrackIRProvider();

			bool query(HeadTrackingStatus* statusOut) override;

			static SCP_string getName();
		};
	}
}

#endif	// _WIN32

#endif // HEADTRACKING_TRACKIR_H
