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
		};
	}
}

#endif // HEADTRACKING_INTERNAL_H
