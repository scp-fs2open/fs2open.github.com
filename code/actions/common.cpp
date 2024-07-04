
#include "common.h"

namespace actions {

expression::ParseContext getTableParseContext()
{
	expression::ParseContext ctx;

	auto& locals = ctx.variables.addScope("locals");

	locals.addMember("position", expression::ValueType::Vector);
	locals.addMember("direction", expression::ValueType::Vector);

	return ctx;
}
expression::ProgramVariables getDefaultTableVariables()
{
	expression::ProgramVariables vars;

	vars.setValue({"locals", "position"}, expression::Value(vmd_zero_vector));
	vars.setValue({"locals", "direction"}, expression::Value(vmd_zero_vector));

	return vars;
}

} // namespace actions
