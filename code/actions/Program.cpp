
#include "Program.h"

#include "Action.h"

#include "actions/ActionDefinitionManager.h"
#include "executor/global_executors.h"
#include "tracing/Monitor.h"
#include "tracing/categories.h"
#include "tracing/tracing.h"

namespace {
using namespace actions;

tracing::Monitor<int> RunningProgramsMonitor("RunningPrograms", 0);

struct ProgramRunner {
	tracing::RunningCounter runningCounter{RunningProgramsMonitor};

	ProgramInstance instance;

	explicit ProgramRunner(const ProgramInstance& _instance) : instance(_instance) {}

	executor::Executor::CallbackResult operator()()
	{
		TRACE_SCOPE(tracing::ProgramStepOne);

		auto res = instance.step();

		switch (res) {
		case ProgramState::Running:
			// If the program is still running then we need to reschedule our execution
			return executor::Executor::CallbackResult::Reschedule;
		case ProgramState::Done:
		case ProgramState::Errored:
		default:
			return executor::Executor::CallbackResult::Done;
		}
	}
};

} // namespace

namespace actions {

ProgramInstance::ProgramInstance(const actions::Program* program) : _program(program) {}

ProgramState ProgramInstance::step()
{
	if (!_locals.host.IsValid()) {
		return ProgramState::Done;
	}

	auto& actions = _program->getActions();

	for (; _currentInstruction < actions.size(); ++_currentInstruction) {
		auto& action = actions[_currentInstruction];

		auto result = action->execute(_locals);

		if (result == ActionResult::Errored) {
			return ProgramState::Errored;
		}

		if (result == ActionResult::Wait) {
			// Action is not done yet, call again next frame
			return ProgramState::Running;
		}

		// Action finished, run next action in program
	}

	// End of program reached, we are done now
	return ProgramState::Done;
}
ProgramLocals& ProgramInstance::locals()
{
	return _locals;
}
const ProgramLocals& ProgramInstance::locals() const
{
	return _locals;
}

const SCP_vector<std::unique_ptr<actions::Action>>& Program::getActions() const
{
	return _actions;
}

Program::Program() = default;

ProgramInstance Program::newInstance() const
{
	return ProgramInstance(this);
}

void Program::parseActions(const flagset<ProgramContextFlags>& context_flags, const expression::ParseContext& context)
{
	while (true) {
		auto action = ActionDefinitionManager::instance().parseAction(context_flags, context);

		if (!action) {
			break;
		}

		_actions.push_back(std::move(action));
	}
}

Program::Program(const Program& other) { *this = other; }
Program& Program::operator=(const Program& other)
{
	_actions.clear();
	_actions.reserve(other._actions.size());

	for (auto& action : other._actions) {
		_actions.emplace_back(action->clone());
	}

	return *this;
}

Program& Program::operator=(Program&&) noexcept = default;
Program::Program(Program&&) noexcept = default;

bool ProgramSet::isEmpty() const
{
	return _programs.empty();
}

ProgramSet::ProgramSet() = default;

ProgramSet::ProgramSet(const flagset<ProgramContextFlags>& context_flags) : _contextFlags(context_flags) {}

void ProgramSet::start(object* objp, const vec3d* local_pos, const matrix* local_orient, int submodel) const
{
	if (_contextFlags[ProgramContextFlags::HasObject]) {
		Assertion(objp != nullptr, "Invalid object pointer supplied.");
	}
	if (_contextFlags[ProgramContextFlags::HasSubobject]) {
		Assertion(submodel >= 0, "Invalid subobject supplied.");
	}

	for (const auto& prog : _programs) {
		auto instance = prog.newInstance();

		instance.locals().host = object_h(objp);
		instance.locals().localPosition = *local_pos;
		instance.locals().localOrient = *local_orient;
		instance.locals().hostSubobject = submodel;
		instance.locals().variables = getDefaultTableVariables();

		// Just use the executor system to take care of running this program
		executor::OnSimulationExecutor->post(ProgramRunner(instance));
	}
}

ProgramSet ProgramSet::parseProgramSet(const char* tag, const flagset<ProgramContextFlags>& context_flags)
{
	ProgramSet set(context_flags);

	const auto parseContext = getTableParseContext();

	while (optional_string(tag)) {
		Program p;
		p.parseActions(context_flags, parseContext);

		set._programs.push_back(std::move(p));
	}

	return set;
}

} // namespace actions
