#pragma once

#include "globalincs/linklist.h"
#include "math/vecmat.h"
#include "object/object.h"
#include "parse/parselo.h"
#include "ship/ship.h"

#include <tuple>
#include <functional>
#include <type_traits>
#include <memory>
#include <map>

//TODO: Move, make pretty, whatever
template<typename T>
class optional {
	union data {
		T t;
		bool dummy;
		constexpr data(T t) : t(t) {}
		constexpr data() : dummy(false) {}
	} data;
	bool filled;

public:
	constexpr optional(const T& data) : data(data), filled(true) { }
	constexpr optional() : data(), filled(false) { }
	constexpr operator T() const {
		if (filled)
			return data.t;
		else {
			UNREACHABLE("Tried access a non-filled optional.");
			return T();
		}
	}

	inline bool has() const {
		return filled;
	}

	inline void if_filled(const std::function<void(const T&)>& fnc) const {
		if (filled)
			fnc(data.t);
	}

	inline void if_filled_or_set(const std::function<void(const T&)>& fnc, const T* orSet = nullptr) {
		if (filled)
			fnc(data.t);
		else if (orSet != nullptr)
			data = *orSet;
	}
};

namespace animation {

	enum class ModelAnimationState { UNTRIGGERED, RUNNING_FWD, TRIGGERED, RUNNING_RWD };


	template <bool is_optional = false>
	struct ModelAnimationData {
	private:
		template<typename T>
		using maybe_optional = std::conditional_t<is_optional, optional<T>, T>;

	public:
		ModelAnimationData() = default;
		ModelAnimationData(const ModelAnimationData<!is_optional>& other) :
			position(other.position),
			orientation(other.orientation) {};

		maybe_optional<vec3d> position;
		maybe_optional<matrix> orientation;

		//This might be a performance bottleneck, but it's the cleanest I can make this without if constexpr and not repeating this code for both types of MAD.
		void applyDelta(const ModelAnimationData<true>& delta) {
			ModelAnimationData<true> data = *this;

			delta.orientation.if_filled([&data](const matrix& other) -> void {
				data.orientation.if_filled_or_set([&data, &other](const matrix& current) -> void {
					matrix tmp;
					vm_matrix_x_matrix(&tmp, &current, &other);
					data.orientation = tmp;
				}, &other);
			});

			delta.position.if_filled([&data](const vec3d& other) -> void {
				data.position.if_filled_or_set([&data, &other](const vec3d& current) -> void {
					vec3d tmp;
					vm_vec_add(&tmp, &current, &other);
					data.position = tmp;
				}, &other);
			});

			*this = data;
		}
	};

	class ModelAnimationSegment {
	protected:
		float m_duration;

	public:
		virtual ~ModelAnimationSegment() = default;

		float getDuration() const;

		//Will be called to give the animations an opportunity to recalculate based on current ship data, as well as animation delta up to that point.
		virtual void recalculate(const ship_subsys* ship_info, const ModelAnimationData<true>& base) = 0;
		//This function needs to contain anything that manipulates ModelAnimationData (such as any movement)
		virtual ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const = 0;
		//This function needs to contain any animation parts that do not change ModelAnimationData (such as sound or particles)
		virtual void executeAnimation(const ModelAnimationData<>& state, float time) const = 0;
	};


	class ModelAnimationSubsystem {
		model_subsystem* m_subsysStatic;
		std::unique_ptr<ModelAnimationSegment> m_mainSegment;
		ModelAnimationData<> m_initialData;
		ModelAnimationData<> m_lastFrame;

		friend class ModelAnimation;
	public:
		ModelAnimationSubsystem(model_subsystem* ssp, std::unique_ptr<ModelAnimationSegment> mainSegment);
		void play(float frametime, ship_subsys* subsys);
		void reset(ship_subsys* subsys);
		ship_subsys* findSubsys(ship* ship) const;

	private:
		void saveCurrentAsBase(ship_subsys* subsys);
		void copyToSubsystem(const ModelAnimationData<>& data, ship_subsys* subsys);
	};


	class ModelAnimation : public std::enable_shared_from_this <ModelAnimation> {
		static std::multimap<ship*, std::shared_ptr<ModelAnimation>> s_runningAnimations;

		std::vector<std::unique_ptr<ModelAnimationSubsystem>> m_submodelAnimation;
		float m_duration;

		ModelAnimationState m_state = ModelAnimationState::UNTRIGGERED;
		float m_time;
		ModelAnimationState play(float frametime, ship* ship);
		static void cleanRunning();
	public:
		void addSubsystemAnimation(std::unique_ptr<ModelAnimationSubsystem> animation);

		void start(ship* ship);
		void stop(ship* ship);

		static void stepAnimations();
		static void clearAnimations();
		//static std::shared_ptr<ModelAnimation> parseAnimationTable();
	};
}