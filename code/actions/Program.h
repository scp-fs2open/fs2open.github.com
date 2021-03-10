#pragma once

#include "globalincs/pstypes.h"

#include "Action.h"
#include "common.h"

namespace actions {

class Program;
class ProgramSet;

enum class ProgramState
{
	Running,
	Done,
	Errored
};

/**
 * @brief An instance of a program execution
 *
 * A program instance contains all information related to a single execution of a program. A program instance is
 * independent from other instances. Local variables used by actions are stored in a specific structure which can be
 * used by actions to keep track of state in this specific instance.
 */
class ProgramInstance {
	size_t _currentInstruction = 0;
	ProgramLocals _locals;
	const Program* _program = nullptr;

  public:
	explicit ProgramInstance(const Program* program);

	ProgramState step();

	ProgramLocals& locals();
	const ProgramLocals& locals() const;
};

/**
 * @brief A program specified in a table
 *
 * A program is a sequential collection of Actions which are executed in sequence.
 */
class Program {
	SCP_vector<std::unique_ptr<Action>> _actions;

  public:
	Program();

	Program(const Program& other);
	Program& operator=(const Program& other);

	Program(Program&& other) noexcept;
	Program& operator=(Program&& other) noexcept;

	const SCP_vector<std::unique_ptr<actions::Action>>& getActions() const;

	/**
	 * @brief Creates a new program instance from this program
	 *
	 * The program instance is initialized to its initial values.
	 *
	 * @return The new program instance
	 */
	ProgramInstance newInstance() const;

	void parseActions(const flagset<ProgramContextFlags>& context_flags, const expression::ParseContext& context);
};

/**
 * @brief A set of parsed programs
 *
 * Programs are parsed in sets of zero or more programs to make it possible for the content creator to split up
 * programs.
 */
class ProgramSet {
	flagset<ProgramContextFlags> _contextFlags;

	SCP_vector<Program> _programs;

  public:
	ProgramSet();
	explicit ProgramSet(const flagset<ProgramContextFlags>& context_flags);

	bool isEmpty() const;

	/**
	 * @brief Starts the execution of all programs within this set with the specified parameters
	 * @param objp The object on which this program is executed
	 * @param local_pos The local position relative to the object or the subsystem if specified
	 * @param local_orien The local orientation relative to the object or the subsystem if specified
	 * @param submodel The submodel of the model to which this program is attached to.
	 */
	void start(object* objp, const vec3d* local_pos, const matrix* local_orient, int submodel = -1) const;

	/**
	 * @brief Parses a program set of zero or more programs
	 *
	 * The tag is entirely optional and if it doesn't appear in the current table then an empty set will be returned
	 * which does nothing if started.
	 *
	 * @param tag The tag which is required before a program can be specified
	 * @param context_flags A set of flags which specify in which environment the program will be used. This is used
	 * mainly for validating the used actions.
	 * @return The parsed program set.
	 */
	static ProgramSet parseProgramSet(const char* tag, const flagset<ProgramContextFlags>& context_flags = {});
};

} // namespace actions
