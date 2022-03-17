#pragma once

#include "parse/sexp/DynamicSEXP.h"
#include "parse/sexp.h"

namespace sexp {

/**
 * @brief Adds the sexp to the dynamic SEXP system
 *
 * The pointer is transferred to the ownership of the SEXP system and may not be deleted by outside code.
 *
 * @warning The new SEXP name is not validated! If the name conflicts with an existing name then this SEXP will not be
 * usable!
 *
 * @param sexp The sexp to add
 * @param type The type of the SEXP
 * 
 * @return The operator id of the SEXP
 */
int add_dynamic_sexp(std::unique_ptr<DynamicSEXP>&& sexp, int type = SEXP_ACTION_OPERATOR);

/**
 * @brief Given an operator constant, return the associated dynamic SEXP
 * @param operator_const The operator constant to check
 * @return The SEXP pointer or @c nullptr if the constant is not known.
 */
DynamicSEXP* get_dynamic_sexp(int operator_const);

/**
 * @brief Dynamically add a new subcategory to the SEXP system
 *
 * @warning Subcategory names must be globally unique! This is not validated by this function.
 *
 * @param parent_category The parent category of this category
 * @param name The display name of the category
 * @return The new category identifier or -1 on error
 */
int add_subcategory(int parent_category, const SCP_string& name);

/**
 * @brief Initializes the dynamic SEXP system
 *
 * This parses the SEXP table and initializes the resources required for dynamic SEXPs.
 */
void dynamic_sexp_init();

/**
 * @brief Frees the registered dynamic SEXPs
 *
 * @warning This just makes sure that the resources of the SEXPs are freed in an orderly fashion. This does not clean up
 * the Operators vector of the SEXP system so it should only be used at game shutdown.
 */
void dynamic_sexp_shutdown();

}
