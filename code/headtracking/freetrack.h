
#ifndef HEADTRACKING_FREETRACK_H
#define HEADTRACKING_FREETRACK_H

#include "headtracking/headtracking.h"
#include "headtracking/headtracking_internal.h"

#include "external_dll/externalcode.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace headtracking
{
	namespace freetrack
	{
		struct FreeTrackData
		{
			unsigned int dataID;
			int camWidth;
			int camHeight;

			float yaw;
			float pitch;
			float roll;
			float x;
			float y;
			float z;

			float rawyaw;
			float rawpitch;
			float rawroll;
			float rawx;
			float rawy;
			float rawz;

			float x1;
			float y1;
			float x2;
			float y2;
			float x3;
			float y3;
			float x4;
			float y4;
		};

		typedef bool (WINAPI *FTGetData_PTR)(FreeTrackData * data);
		typedef char *(WINAPI *FTGetDllVersion_PTR)(void);
		typedef void (WINAPI *FTReportID_PTR)(int name);
		typedef char *(WINAPI *FTProvider_PTR)(void);

		class FreeTrackLibrary : public SCP_ExternalCode
		{
		private:
			FTGetData_PTR mFTGetData;
			FTGetDllVersion_PTR mFTGetDllVersion;
			FTReportID_PTR mFTReportID;
			FTProvider_PTR mFTProvider;

			bool mEnabled;

		public:
			FreeTrackLibrary();

			virtual ~FreeTrackLibrary() {}

			bool GetData(FreeTrackData * data);

			char* GetDllVersion(void);

			void ReportID(int name);

			char* Provider(void);

			bool Enabled() const { return mEnabled; }
		};

		class FreeTrackProvider : public internal::HeadTrackingProvider
		{
		private:
			FreeTrackLibrary library;

		public:
			FreeTrackProvider();

			virtual ~FreeTrackProvider();

			bool query(HeadTrackingStatus* statusOut) override;

			static SCP_string getName();
		};
	}
}

#endif // HEADTRACKING_FREETRACK_H
