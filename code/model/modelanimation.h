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

#include <linb/any.hpp>

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
	
	inline const T* operator&() const {
		return filled ? &data.t : nullptr;
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
		else if (orSet != nullptr) {
			data = *orSet;
			filled = true;
		}
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

// Model Animation Position settings
enum EModelAnimationPosition {
	MA_POS_NOT_SET = 0,	// not yet setup
	MA_POS_SET = 1,	// set, but is moving
	MA_POS_READY = 2     // set, done with move
};

#define ANIMATION_SUBTYPE_ALL INT_MAX

namespace animation {

	enum class ModelAnimationDirection { FWD, RWD };

	enum class ModelAnimationState { UNTRIGGERED, RUNNING_FWD, COMPLETED, RUNNING_RWD, PAUSED, NEED_RECALC };

	enum class ModelAnimationTriggerType : int {
		None = -1,       // No animation
		Initial,		 // This is just the position the subobject should be placed in
		OnSpawn,		 // starts when a ship is being created.
		Docking_Stage1,	 // following the dock path until just before the end. Triggered when AIS_DOCK_1 begins.
		Docking_Stage2,	 // drag oneself right to the second last point on the dock path. The old docking-type trigger. Triggered when AIS_DOCK_2 begins.
		Docking_Stage3,	 // move directly to the dockpoint using thrusters. Triggered when AIS_DOCK_3 begins.
		Docked,			 // As you dock / the attachment sound is played and the mission log indicates docking. Triggered when AIS_DOCK_4 begins.
		PrimaryBank,	 // Primary banks
		SecondaryBank,	 // Secondary banks
		DockBayDoor,	 // Fighter bays
		Afterburner,	 // Afterburner -C
		TurretFiring,	 // Turret shooting -C
		Scripted,		 // Triggered exclusively by scripting...maybe SEXPs? -C
		TurretFired,	 // Triggered after a turret has fired -The E
		PrimaryFired,    // Triggered when a primary weapon has fired.
		SecondaryFired,  // Triggered when a secondary weapon has fired.

		MaxAnimationTypes
	};

	FLAG_LIST(Animation_Flags) {
		Auto_Reverse,			//Will make the animation automatically transition into reverse mode as opposed to waiting in a completed state
		Reset_at_completion,	//Will cause the animation to reset once it completes. This usually only makes sense when the state at the end of the animation is identical to the state at the start of the animation. Incompatible with Auto_reverse
		Loop,					//Will automatically loop the animation once it completes. Is compatible with Reset_at_completion to loop back from the start instead of reversing. Incompatible with Auto_reverse
		Random_starting_phase,  //When an animation is started from an untriggered state, will randomize its time to any possible time of the animation + possibly on the reverse, if the animation would automatically enter that
		Pause_on_reverse,		//Will cause any start in RWD direction to behave as a call to pause the animation. Required (and also only really useful) when a looping animation is supposed to be triggered by an internal engine trigger
		NUM_VALUES
	};

	FLAG_LIST(Animation_Instance_Flags) {
		Stop_after_next_loop,	//Once a looping animation would start the next loop, stop the animation instead. Only valid for looping animations
		NUM_VALUES
	};

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
		ModelAnimationData(const vec3d& copy_position, const matrix& copy_orientation) :
			position(copy_position),
			orientation(copy_orientation) {};

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

	class ModelAnimationSet;

	class ModelAnimationSubmodel {
	protected:
		SCP_string m_name;
		optional<int> m_submodel;
		bool is_turret = false;

	private:
		//Polymodel Instance ID -> ModelAnimationData
		std::map<int, ModelAnimationData<>> m_initialData;
		static ModelAnimationData<> identity;

	public:
		ModelAnimationSubmodel(SCP_string submodelName);
		virtual ~ModelAnimationSubmodel() = default;

		void reset(polymodel_instance* pmi);

		bool saveCurrentAsBase(polymodel_instance* pmi, bool isInitialType = false);
		const ModelAnimationData<>& getInitialData(polymodel_instance* pmi);

		virtual std::pair<submodel_instance*, bsp_info*> findSubmodel(polymodel_instance* pmi);
	private:
		//Hack needed for potential cloning of animations due to templates, while still allowing changing the subsystem data for turret retrieval later on.
		virtual void renameSIP(const SCP_string& /*newSIPname*/) { };
		virtual ModelAnimationSubmodel* copy() const;

		//Reapply the calculated animation state to the submodel
		virtual void copyToSubmodel(const ModelAnimationData<>& data, polymodel_instance* pmi);

		void resetPhysicsData(polymodel_instance* pmi);

		friend class ModelAnimationSet;
	};

	class ModelAnimationSubmodelTurret : public ModelAnimationSubmodel {
	private:
		SCP_string m_SIPname;
		bool m_findBarrel;
		void copyToSubmodel(const ModelAnimationData<>& data, polymodel_instance* pmi) override;


		/*Create a submodel animation by taking the submodel assigned to a subsystem with a given name, or, if requested, the submodel of the turret barrel.
		Due to how turrets work in FSO, this should never be given a segment that does anything but rotate the turret around its axis
		*/

		void renameSIP(const SCP_string& newSIPname) override;
		ModelAnimationSubmodel* copy() const override;

		friend class ModelAnimationSet;
	public:
		ModelAnimationSubmodelTurret(SCP_string subsystemName, bool findBarrel, SCP_string SIPname);
		std::pair<submodel_instance*, bsp_info*> findSubmodel(polymodel_instance* pmi) override;
	};

	struct ModelAnimationSubmodelBufferData { ModelAnimationData<> data; bool modified; };
	//Submodel -> data + was_set
	using ModelAnimationSubmodelBuffer = std::map<std::shared_ptr<ModelAnimationSubmodel>, ModelAnimationSubmodelBufferData>;

	class ModelAnimationSegment {
	protected:

		std::map<int, float> m_duration;

	public:
		virtual ~ModelAnimationSegment() = default;

		float getDuration(int pmi_id) const;

		//This function needs to provide a deep copy operation that returns a copy of this segment, including with all potential child segments copied as well.
		virtual ModelAnimationSegment* copy() const = 0;
		//Will be called to give the animations an opportunity to recalculate based on current ship data, as well as animation data up to that point.
		virtual void recalculate(ModelAnimationSubmodelBuffer& base, polymodel_instance* pmi) = 0;
		//This function needs to contain anything that manipulates ModelAnimationData (such as any movement)
		virtual void calculateAnimation(ModelAnimationSubmodelBuffer& base, float time, int pmi_id) const = 0;
		//This function needs to contain any animation parts that do not change ModelAnimationData (such as sound or particles)
		virtual void executeAnimation(const ModelAnimationSubmodelBuffer& state, float timeboundLower, float timeboundUpper, ModelAnimationDirection direction, int pmi_id) = 0;
		//This function must exchange all held submodel pointers of itself and children with ones acquired from replaceWith.
		virtual void exchangeSubmodelPointers(ModelAnimationSet& replaceWith) = 0;
	};

	class ModelAnimation : public std::enable_shared_from_this <ModelAnimation> {
		struct instance_data {
			ModelAnimationState state = ModelAnimationState::UNTRIGGERED;
			float time = 0.0f;
			float duration = 0.0f;
			flagset<animation::Animation_Instance_Flags> instance_flags;
		};
		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

		const ModelAnimationSet* m_set;

		//True if the animation doesn't need to be kept in running memory, but needs to be applied to a submodels base
		bool m_isInitialType;
		//True if the animation is guaranteed to be identical on each client and can be multi-synced
		bool m_isMultiCompatible;
		//True if the animation can externally have its state changed. Needs special handling
		bool m_canChangeState;

		flagset<animation::Animation_Flags>	m_flags;

		ModelAnimationState play(float frametime, polymodel_instance* pmi, ModelAnimationSubmodelBuffer& applyBuffer, bool applyOnly = false);

		friend class ModelAnimationSet;
		friend class ModelAnimationParseHelper;
	public:
		//Initial type animations must complete within a single frame, and then never modifiy the submodel again. If this is the case, we do not need to remember them being active for massive performance gains with lots of turrets
	  	ModelAnimation(bool isInitialType = false, bool isMultiCompatible = true, bool canStateChange = false, const ModelAnimationSet* defaultSet = nullptr);

		void setAnimation(std::shared_ptr<ModelAnimationSegment> animation);

		void forceRecalculate(polymodel_instance* pmi);

		//Start playing the animation. Will stop other animations that have components running on the same submodels. instant always requires force
		void start(polymodel_instance* pmi, ModelAnimationDirection direction, bool force = false, bool instant = false, bool pause = false, const float* multiOverrideTime = nullptr);
		//Stops the animation. If cleanup is set, it will remove the animation from the list of running animations. Don't call without cleanup unless you know what you are doing
		void stop(polymodel_instance* pmi, bool cleanup = true);

		float getTime(int pmi_id) const;
		
		static void stepAnimations(float frametime, polymodel_instance* pmi);

		unsigned int id = 0;
		std::shared_ptr<ModelAnimationSegment> m_animation;
	};

	class ModelAnimationMoveable {
	protected:
		struct instance_data {
			std::shared_ptr<ModelAnimation> animation = nullptr;
		};
		//PMI ID -> Instance Data
		std::map<int, instance_data> m_instances;

	public:
		virtual ~ModelAnimationMoveable() = default;

		virtual void update(polymodel_instance* pmi, const std::vector<linb::any>& args) = 0;
		virtual void initialize(ModelAnimationSet* parentSet, polymodel_instance* pmi) = 0;
	};


	class ModelAnimationSet {
	public:
		static int SUBTYPE_DEFAULT;
		static std::map<unsigned int, std::shared_ptr<ModelAnimation>> s_animationById;

	private:
		struct RunningAnimationList { const ModelAnimationSet* parentSet; std::list<std::shared_ptr<ModelAnimation>> animationList; };
		//Polymodel Instance ID -> set + ModelAnimation* list (naturally ordered by beginning time))
		static std::map<int, RunningAnimationList> s_runningAnimations;

		std::vector< std::shared_ptr<ModelAnimationSubmodel>> m_submodels;
		SCP_string m_SIPname;

		struct ModelAnimationSubtrigger { 
			ModelAnimationTriggerType type;
			int subtype;
			friend constexpr bool operator<(const ModelAnimationSubtrigger& lhs, const ModelAnimationSubtrigger& rhs) {
				return lhs.type < rhs.type || (!(rhs.type < lhs.type) && lhs.subtype < rhs.subtype);
			}
		};
		// Trigger Type + Subtype -> (Trigger name -> list of Animation*)
		std::map <ModelAnimationSubtrigger, std::map <SCP_string, std::vector<std::shared_ptr<ModelAnimation>>>> m_animationSet;
		std::map <SCP_string, std::shared_ptr<ModelAnimationMoveable>> m_moveableSet;

		static void apply(polymodel_instance* pmi, const ModelAnimationSubmodelBuffer& applyBuffer);
		static void cleanRunning();

		void initializeSubmodelBuffer(polymodel_instance* pmi, ModelAnimationSubmodelBuffer& applyBuffer) const;

		friend class ModelAnimation;
		friend class ModelAnimationParseHelper;

	public:
		ModelAnimationSet(SCP_string SIPname = "");
		ModelAnimationSet(const ModelAnimationSet& other);
		ModelAnimationSet& operator=(ModelAnimationSet&& other) noexcept;
		ModelAnimationSet& operator=(const ModelAnimationSet& other);

		//Helper function to shorten animation emplaces
		void emplace(const std::shared_ptr<ModelAnimation>& animation, const SCP_string& name, ModelAnimationTriggerType type, int subtype, unsigned int uniqueId);

		void changeShipName(const SCP_string& name);

		static void stopAnimations(polymodel_instance* pmi = nullptr);

		void clearShipData(polymodel_instance* pmi);

		class AnimationList {
			std::vector<std::shared_ptr<ModelAnimation>> animations;
			polymodel_instance* pmi = nullptr;
			AnimationList(polymodel_instance* pmi_ = nullptr) : pmi(pmi_) {}
			friend class ModelAnimationSet;
		public:
			bool start(ModelAnimationDirection direction, bool forced = false, bool instant = false, bool pause = false) const;
			int getTime() const;
			void setFlag(Animation_Instance_Flags flag, bool set = true) const;
			AnimationList& operator+=(const AnimationList& rhs);
		};
		AnimationList get(polymodel_instance* pmi, ModelAnimationTriggerType type, const SCP_string& name, int subtype = SUBTYPE_DEFAULT) const;
		AnimationList getAll(polymodel_instance* pmi, ModelAnimationTriggerType type, int subtype = SUBTYPE_DEFAULT, bool strict = false) const;
		AnimationList getBlanket(polymodel_instance* pmi, ModelAnimationTriggerType type) const;
		AnimationList getDockBayDoors(polymodel_instance* pmi, int subtype) const;
		AnimationList parseScripted(polymodel_instance* pmi, ModelAnimationTriggerType type, const SCP_string& triggeredBy) const;

		bool start(polymodel_instance* pmi, ModelAnimationTriggerType type, const SCP_string& name, ModelAnimationDirection direction, bool forced = false, bool instant = false, bool pause = false, int subtype = SUBTYPE_DEFAULT) const;
		bool startAll(polymodel_instance* pmi, ModelAnimationTriggerType type, ModelAnimationDirection direction, bool forced = false, bool instant = false, bool pause = false, int subtype = SUBTYPE_DEFAULT, bool strict = false) const;
		bool startBlanket(polymodel_instance* pmi, ModelAnimationTriggerType type, ModelAnimationDirection direction, bool forced = false, bool instant = false, bool pause = false) const;
		bool startDockBayDoors(polymodel_instance* pmi, ModelAnimationDirection direction, bool forced, bool instant, bool pause, int subtype) const;

		int getTime(polymodel_instance* pmi, ModelAnimationTriggerType type, const SCP_string& name, int subtype = SUBTYPE_DEFAULT) const;
		int getTimeAll(polymodel_instance* pmi, ModelAnimationTriggerType type, int subtype = SUBTYPE_DEFAULT, bool strict = false) const;
		int getTimeDockBayDoors(polymodel_instance* pmi, int subtype) const;

		struct RegisteredTrigger { ModelAnimationTriggerType type; int subtype; const SCP_string& name; };
		std::vector<RegisteredTrigger> getRegisteredTriggers() const;


		bool updateMoveable(polymodel_instance* pmi, const SCP_string& name, const std::vector<linb::any>& args) const;
		void initializeMoveables(polymodel_instance* pmi);
		std::vector<SCP_string> getRegisteredMoveables() const;

		bool isEmpty() const;

		std::shared_ptr<ModelAnimationSubmodel> getSubmodel(SCP_string submodelName);
		std::shared_ptr<ModelAnimationSubmodel> getSubmodel(SCP_string submodelName, const SCP_string& SIP_name, bool findBarrel);
		std::shared_ptr<ModelAnimationSubmodel> getSubmodel(const std::shared_ptr<ModelAnimationSubmodel>& other);
	};

	//Start of parsing functions
	class ModelAnimationParseHelper {

		//Parsing Registrars
		using ModelAnimationSegmentParser = std::function<std::shared_ptr<ModelAnimationSegment>(ModelAnimationParseHelper*)>;
		static std::map<SCP_string, ModelAnimationSegmentParser> s_segmentParsers;

		using ModelAnimationMoveableParser = std::function<std::shared_ptr<ModelAnimationMoveable>()>;
		static std::map<SCP_string, ModelAnimationMoveableParser> s_moveableParsers;

		//Parsed Animations
		struct ParsedModelAnimation {
			std::shared_ptr<ModelAnimation> anim;
			ModelAnimationTriggerType type;
			SCP_string name;
			int subtype;
		};
		static std::map<SCP_string, ParsedModelAnimation> s_animationsById;
		static std::map<SCP_string, std::shared_ptr<ModelAnimationMoveable>> s_moveablesById;

		static unsigned int getUniqueAnimationID(const SCP_string& animName, char uniquePrefix, const SCP_string& parentName);

		//Internal Parsing Methods
		static void parseSingleAnimation();
		static void parseSingleMoveable();
		static void parseTableFile(const char* filename);


	public:
		std::shared_ptr<ModelAnimationSegment> parseSegment();
		//Per Animation parsing Data
		SCP_string m_animationName;
		std::shared_ptr<ModelAnimationSubmodel> parentSubmodel = nullptr;

		static std::shared_ptr<ModelAnimationSubmodel> parseSubmodel();

		static void parseTables();
		static void parseAnimsetInfo(ModelAnimationSet& set, ship_info* sip);
		static void parseAnimsetInfo(ModelAnimationSet& set, char uniqueTypePrefix, const SCP_string& uniqueParentName);
		static void parseMoveablesetInfo(ModelAnimationSet& set);
		//Parses the legacy animation table in ships.tbl of a single subsystem. Currently initial animations only
		static void parseLegacyAnimationTable(model_subsystem* sp, ship_info* sip);
	};
	//Start of section of helper functions, mostly to complement the old modelanim functions as required

	//Type -> Name + Requires reset flag (== will never be triggered in reverse)
	extern const std::map<animation::ModelAnimationTriggerType, std::pair<const char*, bool>> Animation_types;

	void anim_set_initial_states(ship* shipp);
	
	//Returns function bindings to start the targeted animation, and to get its runtime
	std::pair<std::function<bool(ModelAnimationDirection, bool, bool, bool)>, std::function<int()>> anim_parse_scripted_start(const ModelAnimationSet& set, polymodel_instance* pmi, ModelAnimationTriggerType type, const SCP_string& triggeredBy);

	ModelAnimationTriggerType anim_match_type(const char* p);

	SCP_string anim_name_from_subsys(model_subsystem* ss);
}