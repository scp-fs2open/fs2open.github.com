
#include "headtracking/headtracking.h"
#include "headtracking/headtracking_internal.h"

#ifdef WIN32
#include "headtracking/trackir.h"
#include "headtracking/freetrack.h"
#endif

#include <memory>

namespace headtracking
{
	std::unique_ptr<internal::HeadTrackingProvider> currentProvider = nullptr;
	HeadTrackingStatus status;

	template<class Provider>
	std::unique_ptr<internal::HeadTrackingProvider> initProvider()
	{
		std::unique_ptr<internal::HeadTrackingProvider> provider;
		try
		{
			provider.reset(new Provider());
			mprintf(("    Successfully initialized '%s'\n", Provider::getName().c_str() ));

			return provider;
		}
		catch(const internal::HeadTrackingException& e)
		{
			mprintf(("    Failed to initialize '%s': %s.\n", Provider::getName().c_str(), e.what() ));

			return nullptr;
		}
	}

	bool init()
	{
		mprintf(("Initializing head tracking...\n"));

#ifdef WIN32
		auto tir = initProvider<trackir::TrackIRProvider>();
		if (tir)
		{
			currentProvider = std::move(tir);
			return true;
		}

		auto freetrack = initProvider<freetrack::FreeTrackProvider>();
		if (freetrack)
		{
			currentProvider = std::move(freetrack);
			return true;
		}
#endif

		mprintf(("  No supported provider found, headtracking will be disabled...\n"));
		return false;
	}

	bool isEnabled()
	{
		return currentProvider != nullptr;
	}

	bool query()
	{
		if (currentProvider == nullptr)
		{
			return false;
		}

		return currentProvider->query(&status);
	}

	HeadTrackingStatus* getStatus()
	{
		return &status;
	}

	void shutdown()
	{
		currentProvider.reset();
	}
}
