#pragma once

#include "Value.h"

namespace actions {
namespace expression {

using FunctionImplementation = std::function<Value(const SCP_vector<Value>& parameters)>;

struct FunctionDefinition {
	SCP_string name;

	SCP_vector<ValueType> parameterTypes;

	ValueType returnType;

	FunctionImplementation implementation;
};

class FunctionManager {
  public:
	// This type is supposed to be global so it may not be copied
	FunctionManager(const FunctionManager&) = delete;
	FunctionManager& operator=(const FunctionManager&) = delete;

	FunctionManager(FunctionManager&&) noexcept = default;
	FunctionManager& operator=(FunctionManager&&) noexcept = default;

	void addOperator(const SCP_string& name,
		std::initializer_list<ValueType> parameterTypes,
		ValueType returnType,
		FunctionImplementation implementation);

	const FunctionDefinition* findOperator(const SCP_string& name, const SCP_vector<ValueType>& parameterTypes) const;

	static const FunctionManager& instance();

  private:
	FunctionManager();

	SCP_unordered_map<SCP_string, SCP_vector<FunctionDefinition>> m_operators;
};

} // namespace expression
} // namespace actions
