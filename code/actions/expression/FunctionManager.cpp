
#include "FunctionManager.h"

#include "TypeDefinition.h"

#include "math/vecmat.h"

namespace actions {
namespace expression {
namespace {
template <typename T>
Value value_plus(const SCP_vector<Value>& parameters)
{
	return Value(parameters[0].get<T>() + parameters[1].get<T>());
}
template <typename T>
Value value_minus(const SCP_vector<Value>& parameters)
{
	return Value(parameters[0].get<T>() - parameters[1].get<T>());
}
} // namespace

const FunctionManager& FunctionManager::instance()
{
	static const FunctionManager manager;
	return manager;
}
void FunctionManager::addOperator(const SCP_string& name,
	std::initializer_list<ValueType> parameterTypes,
	ValueType returnType,
	FunctionImplementation implementation)
{
	m_operators[name].push_back({name, parameterTypes, returnType, std::move(implementation)});
}
const FunctionDefinition* FunctionManager::findOperator(const SCP_string& name,
	const SCP_vector<ValueType>& parameters) const
{
	const auto iter = m_operators.find(name);

	if (iter == m_operators.end()) {
		return nullptr;
	}

	// Prefer exact matches
	for (const auto& funcDef : iter->second) {
		if (funcDef.parameterTypes == parameters) {
			return &funcDef;
		}
	}

	// Now check if there are matches with implicit conversions
	for (const auto& funcDef : iter->second) {
		// Reject functions with different amount of elements
		if (funcDef.parameterTypes.size() != parameters.size()) {
			continue;
		}

		auto funcParamIter = funcDef.parameterTypes.cbegin();
		auto actualParamIter = parameters.cbegin();

		bool match = true;
		for (; funcParamIter != funcDef.parameterTypes.cend(); ++funcParamIter, ++actualParamIter) {
			if (!checkTypeWithImplicitConversion(*actualParamIter, *funcParamIter)) {
				// Also doesn't match with implicit conversions
				match = false;
				break;
			}
		}

		if (match) {
			return &funcDef;
		}
	}

	return nullptr;
}

FunctionManager::FunctionManager()
{
	addOperator("+", {ValueType::Integer, ValueType::Integer}, ValueType::Integer, value_plus<int>);
	addOperator("+", {ValueType::Float, ValueType::Float}, ValueType::Float, value_plus<float>);
	addOperator("+", {ValueType::Vector, ValueType::Vector}, ValueType::Vector, value_plus<vec3d>);

	addOperator("-", {ValueType::Integer, ValueType::Integer}, ValueType::Integer, value_minus<int>);
	addOperator("-", {ValueType::Float, ValueType::Float}, ValueType::Float, value_minus<float>);
	addOperator("-", {ValueType::Vector, ValueType::Vector}, ValueType::Vector, value_minus<vec3d>);
}

} // namespace expression
} // namespace actions
