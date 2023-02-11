#pragma once

#include "globalincs/pstypes.h"

#include "DynamicSEXP.h"
#include "SEXPParameterExtractor.h"

namespace sexp {

class EngineSEXP;

struct dummy_return {
};

using EngineSexpAction = std::function<int(SEXPParameterExtractor* extractor)>;

class EngineSEXPFactory {
	std::unique_ptr<EngineSEXP> m_sexp;

	SCP_string _helpText;

	int _category = OP_CATEGORY_NONE;

	int _subcategory = OP_SUBCATEGORY_NONE;
	SCP_string _subcategoryName;

	int _returnType = -1;

	EngineSexpAction _action;

	struct argument {
		int type = -1;
		SCP_string help_text;

		bool optional_marker = false;
		bool varargs_marker = false;
	};

	SCP_vector<argument> _arguments;

	class ArgumentListBuilder {
		EngineSEXPFactory* _parent;

	  public:
		ArgumentListBuilder(EngineSEXPFactory* parent);

		/**
		 * @brief Adds a parameter of the specified type with help text.
		 * @param type The type of this parameter
		 * @param help_text A helpful text for this parameter
		 * @return The arglist builder
		 */
		ArgumentListBuilder& arg(int type, SCP_string help_text);
		/**
		 * @brief Specifies that all following arguments are optional
		 * @return The arglist builder
		 */
		ArgumentListBuilder& beginOptional();
		/**
		 * @brief Begins the part of the parameter list which can be repeated
		 * @return The arglist builder
		 */
		ArgumentListBuilder& beginVarargs();

		/**
		 * @brief Finishes the argument list
		 * @return The parent engine SEXP factory
		 */
		EngineSEXPFactory& finishArguments();
	};

	friend ArgumentListBuilder;

  public:
	EngineSEXPFactory(std::unique_ptr<EngineSEXP> sexp);
	virtual ~EngineSEXPFactory();

	EngineSEXPFactory(EngineSEXPFactory&&) noexcept = default;
	EngineSEXPFactory& operator=(EngineSEXPFactory&&) noexcept = default;

	/**
	 * @brief Specifies the help text of the SEXP
	 * @param text The help text
	 * @return The factory
	 */
	EngineSEXPFactory& helpText(SCP_string text);

	/**
	 * @brief The return type of the SEXP
	 *
	 * This is a required value!
	 *
	 * @param type The OPR_ value
	 * @return The factory
	 */
	EngineSEXPFactory& returnType(int type);

	/**
	 * @brief The category where this SEXP will be located
	 *
	 * This is a required value!
	 *
	 * @param cat The OP_CATEGORY_ value
	 * @return The factory
	 */
	EngineSEXPFactory& category(int cat);

	/**
	 * @brief The subcategory of this SEXP
	 *
	 * This or the string variant is a required value!
	 *
	 * @param subcat The *_SUBCATEGORY_* value
	 * @return The factory
	 */
	EngineSEXPFactory& subcategory(int subcat);
	/**
	 * @brief Sets the subcategory of the SEXP.
	 *
	 * This can be a new subcategory which will be created automatically.
	 *
	 * This or the number variant is a required value!
	 *
	 * @param subcat The name of the subcategory
	 * @return The factory
	 */
	EngineSEXPFactory& subcategory(const SCP_string& subcat);

	/**
	 * @brief Begins specifying arguments for the SEXP
	 * @return
	 */
	ArgumentListBuilder beginArgList();

	/**
	 * @brief The function that will be called for executing the SEXP
	 * @param act The function to execute
	 * @return The factory
	 */
	EngineSEXPFactory& action(EngineSexpAction act);

	/**
	 * @brief Finishes the construction of the SEXP
	 * @return A dummy value to allow SEXPs to be added statically at application start
	 */
	dummy_return finish();
};

class EngineSEXP : public DynamicSEXP {
	EngineSEXP(const SCP_string& name);

  public:
	void initialize() override;
	int getMinimumArguments() const override;
	int getMaximumArguments() const override;
	int getArgumentType(int argnum) const override;
	int execute(int node) override;
	int getReturnType() override;
	int getSubcategory() override;
	int getCategory() override;

	/**
	 * @brief Start creating an engine SEXP.
	 *
	 * This returns a factory with which the various parameters of the SEXP can be configured
	 *
	 * @param name The name of the SEXP
	 * @return The engine SEXP factory
	 */
	static EngineSEXPFactory create(const SCP_string& name);

  private:
	void setCategory(int category);
	void setSubcategory(int subcategory);
	void setSubcategoryName(SCP_string subcategory);
	void setHelpText(SCP_string helpText);
	void setReturnType(int returnType);
	void initArguments(int minArgs, int maxArgs, SCP_vector<int> argTypes, SCP_vector<int> varargsTypes);
	void setAction(EngineSexpAction action);

	int _category = OP_CATEGORY_NONE;

	int _subcategory = OP_SUBCATEGORY_NONE;
	SCP_string _subcategoryName;

	int _returnType = -1;

	int _minArgs = -1;
	int _maxArgs = -1;
	SCP_vector<int> _argumentTypes;
	SCP_vector<int> _variableArgumentsTypes;

	EngineSexpAction _action;

	friend EngineSEXPFactory;
};

} // namespace sexp
