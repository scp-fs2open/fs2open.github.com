#pragma once

#include "globalincs/pstypes.h"

namespace sexp {

/**
 * @brief An abstract SEXP instance
 *
 * This class represents an implementation of a SEXP that can be added dynamically to the SEXP system
 */
class DynamicSEXP {
 protected:
	SCP_string _name; //!< The operator name of this SEXP
	SCP_string _help_text; //!< The help text to be displayed in FRED

	explicit DynamicSEXP(const SCP_string& name);
 public:
	virtual ~DynamicSEXP() = default;

	const SCP_string& getName() const;

	const SCP_string& getHelpText() const;

	/**
	 * @brief Called when the sexp is added to the SEXP system
	 *
	 * This can be used for executing operations that can only be run once the SEXP is added to the system
	 */
	virtual void initialize() = 0;

	/**
	 * @brief Retrieves the minimum number of parameters this SEXP needs
	 *
	 * All remaining parameters are optional and the implementation must handle non-existing parameters gracefully
	 *
	 * @return The minimum amount of arguments
	 */
	virtual int getMinimumArguments() const = 0;

	/**
	 * @brief Gets the maximum number of how many arguments this SEXP may be called with
	 *
	 * If this returns INT_MAX then there will be no limit to how many parameters may be specified.
	 *
	 * @return The maximum amount of arguments for this SEXP
	 */
	virtual int getMaximumArguments() const = 0;

	/**
	 * @brief Retrieves the type of the specified argument (0-base)
	 * @param argnum The argument to query the type for
	 * @return The argument type
	 */
	virtual int getArgumentType(int argnum) const = 0;

	/**
	 * @brief Executes the SEXP implementation
	 * @param node The SEXP node containing the first parameter of the parameter list
	 * @return The return value of the SEXP
	 */
	virtual int execute(int node, int parent_node = -1) = 0;

	/**
	 * @brief Gets the return type of the SEXP
	 *
	 * See the OPR_* defines for what values this function may return.
	 *
	 * @return The return type
	 */
	virtual int getReturnType() = 0;

	/**
	 * @brief The subcategory identifier this SEXP belongs to
	 *
	 * This is used by FRED to group SEXPs with similar purposes together. This can be one of the *_SUBCATEGORY_*
	 * defines or a new id which has been added dynamically.
	 *
	 * @return The subcategory id
	 */
	virtual int getSubcategory() = 0;

	/**
	 * @brief The general category of this SEXP
	 *
	 * This can be any of the OP_CATEGORY_* defines.
	 *
	 * @return The category id.
	 */
	virtual int getCategory() = 0;
};

}

