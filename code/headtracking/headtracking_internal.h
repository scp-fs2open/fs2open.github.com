#ifndef HEADTRACKING_INTERNAL_H
#define HEADTRACKING_INTERNAL_H

#include "globalincs/pstypes.h"

#include "headtracking/headtracking.h"

namespace headtracking
{
	namespace internal
	{
		class HeadTrackingException : public std::runtime_error
		{
		public:
			explicit HeadTrackingException(const std::string& _Message)
				: runtime_error(_Message)
			{
			}
		};

		class HeadTrackingProvider
		{
		public:
			virtual ~HeadTrackingProvider()
			{
			}

			virtual bool query(HeadTrackingStatus* statusOut) = 0;

			/**
			 * @brief Whether the provider currently has a live tracking source.
			 *
			 * Providers that may be initialized without a tracker actually running (e.g. a
			 * network listener) can return false so the normal view controls stay usable.
			 */
			virtual bool isActive()
			{
				return true;
			}
		};
	}
}

#endif // HEADTRACKING_INTERNAL_H
