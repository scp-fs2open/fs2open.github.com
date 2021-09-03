#pragma once

#include "globalincs/linklist.h"
#include "math/vecmat.h"
#include "model/model.h"
#include "object/object.h"
#include "parse/parselo.h"

#include <tuple>
#include <functional>
#include <type_traits>
#include <memory>
#include <map>

//Only needed for the trigger type, will eventually disappear
#include "model/modelanim.h"

//Since we don't have C++17, this is a small (actually not std conform) implementation of an optional that works for objects with complex constructors
template<typename T>
class optional {
	union data {
		T t;
		bool dummy;
		constexpr data(T other_t) : t(other_t) {}
		constexpr data() : dummy(false) {}
	} data;
	bool filled = false;

public:
	constexpr optional(const T& other_data) : data(other_data), filled(true) { }
	constexpr optional() : data() { }

	operator T() const {
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

	inline bool operator==(const optional<T>& rhs) {
		if (filled != rhs.filled)
			return false;
		else if (!filled)
			return true;
		return data.t == rhs.data.t;
	}
};

class ship;
class ship_info;

namespace animation {

	enum class ModelAnimationState { UNTRIGGERED, RUNNING_FWD, COMPLETED, RUNNING_RWD };


	template <bool is_optional = false>
	struct ModelAnimationData {
	private:
		template<typename T>
		using maybe_optional = typename std::conditional<is_optional, optional<T>, T>::type;

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
					vm_matrix_x_matrix(&tmp, &other, &current);
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
		float m_duration = 0.0f;

	public:
		virtual ~ModelAnimationSegment() = default;

		float getDuration() const;

		//Will be called to give the animations an opportunity to recalculate based on current ship data, as well as animation data up to that point.
		virtual void recalculate(const submodel_instance* submodel_instance, const bsp_info* submodel, const ModelAnimationData<>& base) = 0;
		//This function needs to contain anything that manipulates ModelAnimationData (such as any movement)
		virtual ModelAnimationData<true> calculateAnimation(const ModelAnimationData<>& base, const ModelAnimationData<>& lastState, float time) const = 0;
		//This function needs to contain any animation parts that do not change ModelAnimationData (such as sound or particles)
		virtual void executeAnimation(const ModelAnimationData<>& state, float time) const = 0;
	};
	

	class ModelAnimationSubmodel {
	protected:
		SCP_string m_name;
		optional<int> m_submodel;
		
	private:
		std::shared_ptr<ModelAnimationSegment> m_mainSegment;
		//Polymodel Instance ID -> ModelAnimationData
		std::map<int, ModelAnimationData<>> m_initialData;
		//Polymodel Instance ID -> ModelAnimationData
		std::map<int, ModelAnimationData<>> m_lastFrame;

		friend class ModelAnimation;
	public:
		//Create a submodel animation based on the name of the submodel
		ModelAnimationSubmodel(SCP_string submodelName, std::shared_ptr<ModelAnimationSegment> mainSegment);

		virtual ~ModelAnimationSubmodel() = default;

		//Sets the animation to the specified time and applies it to the submodel
		void play(float time, polymodel_instance* pmi);

		void reset(polymodel_instance* pmi);

		//Hack needed for potential cloning of animations due to templates, while still allowing changing the subsystem data for turret retrieval later on.
		virtual ModelAnimationSubmodel* copy(const SCP_string& /*newSIPname*/);

	private:
		//Set the submodels current state as the base for the animation, recalculate the animation data (e.g. take this as the base for absolutely defined angles)
		float saveCurrentAsBase(polymodel_instance* pmi);
		//Reapply the calculated animation state to the submodel
		virtual void copyToSubmodel(const ModelAnimationData<>& data, polymodel_instance* pmi);
		virtual std::pair<submodel_instance*, bsp_info*> findSubmodel(polymodel_instance* pmi);
	};

	class ModelAnimationSubmodelTurret : public ModelAnimationSubmodel {
		SCP_string m_SIPname;
		bool m_findBarrel;
		void copyToSubmodel(const ModelAnimationData<>& data, polymodel_instance* pmi) override;
		std::pair<submodel_instance*, bsp_info*> findSubmodel(polymodel_instance* pmi) override;
	public:
		/*Create a submodel animation by taking the submodel assigned to a subsystem with a given name, or, if requested, the submodel of the turret barrel.
		Due to how turrets work in FSO, this should never be given a segment that does anything but rotate the turret around its axis
		*/
		ModelAnimationSubmodelTurret(SCP_string subsystemName, bool findBarrel, SCP_string SIPname, std::shared_ptr<ModelAnimationSegment> mainSegment);

		ModelAnimationSubmodel* copy(const SCP_string& newSIPname) override;
	};


	class ModelAnimation : public std::enable_shared_from_this <ModelAnimation> {
		//Polymodel Instance ID -> ModelAnimation*
		static std::multimap<int, std::shared_ptr<ModelAnimation>> s_runningAnimations;

		std::vector<std::shared_ptr<ModelAnimationSubmodel>> m_submodelAnimation;
		float m_duration = 0.0f;

		//Polymodel Instance ID -> ModelAnimationState
		std::map<int, ModelAnimationState> m_state;
		//Polymodel Instance ID -> ModelAnimation Time
		std::map<int, float> m_time;

		bool m_isInitialType;

		ModelAnimationState play(float frametime, polymodel_instance* pmi);

		static void cleanRunning();

		friend struct ModelAnimationSet;
	public:
		//Initial type animations must complete within a single frame, and then never modifiy the submodel again. If this is the case, we do not need to remember them being active for massive performance gains with lots of turrets
		ModelAnimation(bool isInitialType = false);

		void addSubmodelAnimation(std::shared_ptr<ModelAnimationSubmodel> animation);

		//Start playing the animation. Will stop other animations that have components running on the same submodels
		void start(polymodel_instance* pmi, bool reverse, bool force = false);
		//Stops the animation. If cleanup is set, it will remove the animation from the list of running animations. Don't call without cleanup unless you know what you are doing
		void stop(polymodel_instance* pmi, bool cleanup = true);

		static void stepAnimations(float frametime);
		//Find animations in the running animations list that are fully reset and need to be removed
		static void clearAnimations();
		//Parses the legacy animation table in ships.tbl of a single subsystem. Currently initial animations only
		static void parseLegacyAnimationTable(model_subsystem* sp, ship_info* sip);
		//static std::shared_ptr<ModelAnimation> parseAnimationTable();
	};


	//Use the old modelanim.cpp TriggerType here. Eventually migrate to here.
	using ModelAnimationTriggerType = ::AnimationTriggerType;

	struct ModelAnimationSet {
		static int SUBTYPE_DEFAULT;

		std::map <std::pair<ModelAnimationTriggerType, int>, std::map <SCP_string, std::shared_ptr<ModelAnimation>>> animationSet;

		//Helper function to shorten animation emplaces
		void emplace(const std::shared_ptr<ModelAnimation>& animation, const SCP_string& name, ModelAnimationTriggerType type = ModelAnimationTriggerType::Scripted, int subtype = SUBTYPE_DEFAULT);

		void changeShipName(const SCP_string& name);
	};


	//Start of section of helper functions, mostly to complement the old modelanim functions as required

	void anim_set_initial_states(ship* shipp);
}