
#include "FunctionManager.h"

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
	static const FunctionManager manager = []() {
		FunctionManager newManager;

		newManager.addOperator("+", {ValueType::Integer, ValueType::Integer}, ValueType::Integer, value_plus<int>);
		newManager.addOperator("+", {ValueType::Float, ValueType::Float}, ValueType::Float, value_plus<float>);
		newManager.addOperator("+", {ValueType::Vector, ValueType::Vector}, ValueType::Vector, value_plus<vec3d>);

		newManager.addOperator("-", {ValueType::Integer, ValueType::Integer}, ValueType::Integer, value_minus<int>);
		newManager.addOperator("-", {ValueType::Float, ValueType::Float}, ValueType::Float, value_minus<float>);
		newManager.addOperator("-", {ValueType::Vector, ValueType::Vector}, ValueType::Vector, value_minus<vec3d>);

		return newManager;
	}();
	return manager;
}
void FunctionManager::addOperator(SCP_string name,
	std::initializer_list<ValueType> parameterTypes,
	ValueType returnType,
	FunctionImplementation implementation)
{
	m_operators[name].push_back({std::move(name), parameterTypes, returnType, std::move(implementation)});
}
const FunctionDefinition* FunctionManager::findOperator(const SCP_string& name,
	const SCP_vector<ValueType>& parameters) const
{
	const auto iter = m_operators.find(name);

	if (iter == m_operators.end()) {
		return nullptr;
	}

	for (const auto& funcDef : iter->second) {
		if (funcDef.parameterTypes == parameters) {
			return &funcDef;
		}
	}

	return nullptr;
}

FunctionManager::FunctionManager() = default;

} // namespace expression
} // namespace actions
