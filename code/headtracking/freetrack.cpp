
#include "headtracking/freetrack.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace
{
	const char* const LIBRARY_NAME_32 = "FreeTrackClient.dll";
	const char* const LIBRARY_NAME_64 = "FreeTrackClient64.dll";

	/**
	 * @brief Find the location of the FreeTrack library
	 * 
	 * For windows there is a registry entry at HKEY_CURRENT_USER\Software\FreeTrack\FreeTrackClient
	 * that specifies where the dll is located.
	 */
	SCP_string getLibraryLocation()
	{
		HKEY hKey = NULL;
		LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER,
			TEXT("Software\\FreeTrack\\FreeTrackClient"),
			NULL,
			KEY_QUERY_VALUE,
			&hKey);

		if (lResult != ERROR_SUCCESS)	{
			return "";
		}

		DWORD dwLen;

		// First get the size of the value
		lResult = RegQueryValueEx(hKey,
			"Path",
			NULL,
			NULL,
			NULL,
			&dwLen);

		if (lResult != ERROR_SUCCESS)	{
			return "";
		}

		SCP_string value;
		value.resize(dwLen);

		dwLen = static_cast<DWORD>(value.size());

		lResult = RegQueryValueEx(hKey,
			"Path",
			NULL,
			NULL,
			reinterpret_cast<BYTE*>(&value[0]),
			&dwLen);

		// Windows appends a \0 character at the end
		value.resize(value.length() - 1);

		if (!value.empty())
		{
			// Fix paths without slash at the end
			if (value[value.length() - 1] != '\\' && value[value.length() - 1] != '/')
			{
				value.append("/");
			}
		}

		// now append the name of the library and return
#ifdef _WIN64
		value.append(LIBRARY_NAME_64);
#else
		value.append(LIBRARY_NAME_32);
#endif
		return value;
	}
}

namespace headtracking
{
	namespace freetrack
	{
		FreeTrackLibrary::FreeTrackLibrary() : mFTGetData(nullptr), mFTGetDllVersion(nullptr),
			mFTReportID(nullptr), mFTProvider(nullptr), mEnabled(false)
		{
			if (!LoadExternal(getLibraryLocation().c_str()))
				return;

			mFTGetData = LoadFunction<FTGetData_PTR>("FTGetData");
			mFTGetDllVersion = LoadFunction<FTGetDllVersion_PTR>("FTGetDllVersion");
			mFTReportID = LoadFunction<FTReportID_PTR>("FTReportName");
			mFTProvider = LoadFunction<FTProvider_PTR>("FTProvider");
			
			mEnabled = true;
		}

		bool FreeTrackLibrary::GetData(FreeTrackData * data)
		{
			if (mFTGetData)
				return mFTGetData(data);

			return false;
		}

		char* FreeTrackLibrary::GetDllVersion(void)
		{
			if (mFTGetDllVersion)
				return mFTGetDllVersion();

			return nullptr;
		}

		void FreeTrackLibrary::ReportID(int name)
		{
			if (mFTReportID)
				mFTReportID(name);
		}

		char* FreeTrackLibrary::Provider(void)
		{
			if (mFTProvider)
				return mFTProvider();

			return nullptr;
		}

		FreeTrackProvider::FreeTrackProvider(): library(FreeTrackLibrary())
		{
			if (!library.Enabled())
			{
				throw internal::HeadTrackingException("Library could not be loaded!");
			}

			// Try to get initial test data
			FreeTrackData data;
			data.dataID = std::numeric_limits<unsigned int>::max();
			data.camHeight = std::numeric_limits<int>::min();
			data.camWidth = std::numeric_limits<int>::min();

			if (!library.GetData(&data))
			{
				throw internal::HeadTrackingException("Failed to get test data set!");
			}

			if (data.dataID == std::numeric_limits<unsigned int>::max() ||
				data.camHeight == std::numeric_limits<int>::min() ||
				data.camWidth == std::numeric_limits<int>::min())
			{
				// Check if all the values have been changed
				throw internal::HeadTrackingException("The test values reported by FreeTrack were invalid!");
			}

			// I have no idea what a correct value for this function is so I used this random value
			library.ReportID(7919);

			mprintf(("Found FreeTrack provider '%s' with version %s.\n", library.Provider(), library.GetDllVersion()));
		}

		FreeTrackProvider::~FreeTrackProvider()
		{
		}

		bool FreeTrackProvider::query(HeadTrackingStatus* statusOut)
		{
			FreeTrackData data;
			if (!library.GetData(&data))
			{
				return false;
			}

			statusOut->pitch = data.pitch;
			statusOut->yaw = data.yaw;
			statusOut->roll = data.roll;

			// Coordinates are in millimeters
			statusOut->x = data.x / 1000.0f;
			statusOut->y = data.y / 1000.0f;
			statusOut->z = data.z / 1000.0f;

			return true;
		}

		SCP_string FreeTrackProvider::getName()
		{
			return "FreeTrack";
		}
	}
}
