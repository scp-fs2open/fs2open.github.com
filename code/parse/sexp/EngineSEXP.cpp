#include "EngineSEXP.h"

#include "sexp_lookup.h"

#include "parse/sexp.h"

namespace sexp {
namespace {

int get_subcategory(const SCP_string& name)
{
	for (auto& subcat : op_submenu) {
		if (subcat.name == name) {
			return subcat.id;
		}
	}

	return -1;
}

} // namespace

EngineSEXPFactory::ArgumentListBuilder::ArgumentListBuilder(EngineSEXPFactory* parent) : _parent(parent) {}
EngineSEXPFactory::ArgumentListBuilder& EngineSEXPFactory::ArgumentListBuilder::arg(int type, SCP_string help_text)
{
	argument arg;
	arg.type = type;
	arg.help_text = std::move(help_text);

	_parent->_arguments.emplace_back(std::move(arg));
	return *this;
}
EngineSEXPFactory::ArgumentListBuilder& EngineSEXPFactory::ArgumentListBuilder::beginOptional()
{
	Assertion(std::find_if(_parent->_arguments.begin(),
				  _parent->_arguments.end(),
				  [](const argument& arg) { return arg.optional_marker; }) == _parent->_arguments.end(),
		"Optional marker was already added!");
	Assertion(std::find_if(_parent->_arguments.begin(),
				  _parent->_arguments.end(),
				  [](const argument& arg) { return arg.varargs_marker; }) == _parent->_arguments.end(),
		"Adding optional arguments after varags specifier is not allowed!");

	argument arg;
	arg.optional_marker = true;

	_parent->_arguments.emplace_back(std::move(arg));
	return *this;
}
EngineSEXPFactory::ArgumentListBuilder& EngineSEXPFactory::ArgumentListBuilder::beginVarargs()
{
	Assertion(std::find_if(_parent->_arguments.begin(),
				  _parent->_arguments.end(),
				  [](const argument& arg) { return arg.varargs_marker; }) == _parent->_arguments.end(),
		"Varargs marker was already added!");

	argument arg;
	arg.varargs_marker = true;

	_parent->_arguments.emplace_back(std::move(arg));
	return *this;
}
EngineSEXPFactory& EngineSEXPFactory::ArgumentListBuilder::finishArguments() { return *_parent; }

EngineSEXPFactory::EngineSEXPFactory(std::unique_ptr<EngineSEXP> sexp) : m_sexp(std::move(sexp)) {}

EngineSEXPFactory& EngineSEXPFactory::category(int cat)
{
	_category = cat;
	return *this;
}
EngineSEXPFactory& EngineSEXPFactory::subcategory(int cat)
{
	_category = cat;
	return *this;
}
EngineSEXPFactory& EngineSEXPFactory::subcategory(const SCP_string& subcat)
{
	_subcategoryName = subcat;
	return *this;
}
EngineSEXPFactory::ArgumentListBuilder EngineSEXPFactory::beginArgList() { return {this}; }

dummy_return EngineSEXPFactory::finish()
{
	Assertion(_category >= 0, "Engine SEXP %s: A category has to be specified!", m_sexp->getName().c_str());
	Assertion(_returnType >= 0, "Engine SEXP %s: A return type has to be specified!", m_sexp->getName().c_str());

	m_sexp->setCategory(_category);
	if (_subcategory >= 0) {
		m_sexp->setSubcategory(_subcategory);
	} else {
		Assertion(!_subcategoryName.empty(), "A subcategory has to be specified!");
		m_sexp->setSubcategoryName(_subcategoryName);
	}

	m_sexp->setReturnType(_returnType);

	SCP_stringstream helpStream;
	helpStream << m_sexp->getName() << "\r\n";
	helpStream << "\t" << _helpText << "\r\n";

	// Now build the argument help text and min and max argument count
	int min_args_counter = 0;
	int max_args_counter = 0;
	SCP_vector<int> argument_types;
	SCP_vector<int> variable_argument_types;

	int arg_counter = 1;
	bool varargs = false;
	bool optional = false;
	for (const auto& arg : _arguments) {
		if (arg.varargs_marker) {
			helpStream << "Rest: (The following pattern repeats)\r\n";
			// Number non-vararg and vararg parameters separately
			arg_counter = 1;
			varargs = true;
			continue;
		} else if (arg.optional_marker) {
			optional = true;
			continue;
		}

		helpStream << arg_counter << ": " << arg.help_text << "\r\n";
		++arg_counter;

		if (!varargs && !optional) {
			// Keep track of minimum number of arguments. If we are either in the varargs part or the optional part then
			// we have found the minimum number and should not increment it any further.
			min_args_counter++;
		}
		// Always increment max. varargs are handled after the loop
		max_args_counter++;

		// Collect argument types
		if (varargs) {
			variable_argument_types.push_back(arg.type);
		} else {
			argument_types.push_back(arg.type);
		}
	}

	if (varargs) {
		max_args_counter = std::numeric_limits<int>::max();
	}

	m_sexp->initArguments(min_args_counter,
		max_args_counter,
		std::move(argument_types),
		std::move(variable_argument_types));
	m_sexp->setHelpText(helpStream.str());
	m_sexp->setAction(std::move(_action));

	add_dynamic_sexp(std::move(m_sexp));

	return {};
}
EngineSEXPFactory::~EngineSEXPFactory()
{
	Assertion(m_sexp == nullptr, "Did not call finish on a EngineSEXP factory!");
}
EngineSEXPFactory& EngineSEXPFactory::helpText(SCP_string text)
{
	_helpText = std::move(text);
	return *this;
}
EngineSEXPFactory& EngineSEXPFactory::returnType(int type)
{
	_returnType = type;
	return *this;
}
EngineSEXPFactory& EngineSEXPFactory::action(EngineSexpAction act)
{
	_action = std::move(act);
	return *this;
}
EngineSEXP::EngineSEXP(const SCP_string& name) : DynamicSEXP(name) {}

EngineSEXPFactory EngineSEXP::create(const SCP_string& name)
{
	return { std::unique_ptr<EngineSEXP>(new EngineSEXP(name)) };
}
void EngineSEXP::initialize()
{
	// Initialize subcategory now that we know that it is safe to do so
	if (_subcategory < 0) {
		_subcategory = get_subcategory(_subcategoryName);

		if (_subcategory < 0) {
			_subcategory = add_subcategory(_category, _subcategoryName);
		}
	}
}
int EngineSEXP::getMinimumArguments() { return _minArgs; }
int EngineSEXP::getMaximumArguments() { return _maxArgs; }
int EngineSEXP::getArgumentType(int argnum) const
{
	if (argnum >= static_cast<int>(_argumentTypes.size())) {
		// We are in the varargs part. Adjust the number and then fit it into the varargs types array
		argnum -= static_cast<int>(_argumentTypes.size());

		// Error checking to avoid division by zero
		if (_variableArgumentsTypes.empty()) {
			return OPF_NONE;
		}

		argnum = argnum % _variableArgumentsTypes.size();

		return _variableArgumentsTypes[argnum];
	} else {
		// Normal arguments so just look up the type
		return _argumentTypes[argnum];
	}
}
int EngineSEXP::execute(int node)
{
	SEXPParameterExtractor extractor(node);
	return _action(&extractor);
}
int EngineSEXP::getReturnType() { return _returnType; }
int EngineSEXP::getSubcategory() { return _subcategory; }
int EngineSEXP::getCategory() { return _category; }

void EngineSEXP::setCategory(int category) { _category = category; }
void EngineSEXP::setSubcategory(int subcategory) { _subcategory = subcategory; }
void EngineSEXP::setSubcategoryName(SCP_string subcategory) { _subcategoryName = std::move(subcategory); }
void EngineSEXP::setHelpText(SCP_string helpText) { _help_text = std::move(helpText); }
void EngineSEXP::setReturnType(int returnType) { _returnType = returnType; }
void EngineSEXP::initArguments(int minArgs, int maxArgs, SCP_vector<int> argTypes, SCP_vector<int> varargsTypes)
{
	_minArgs = minArgs;
	_maxArgs = maxArgs;

	_argumentTypes = std::move(argTypes);
	_variableArgumentsTypes = std::move(varargsTypes);
}
void EngineSEXP::setAction(EngineSexpAction action) { _action = std::move(action); }

} // namespace sexp
