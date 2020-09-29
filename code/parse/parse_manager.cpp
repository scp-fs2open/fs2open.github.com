#pragma
#include "globalincs/pstypes.h"
#include "parse_manager.h"

parse_item_builder::parse_item_builder()
{
	_product.reset_fields();
}

parse_item* parse_item_builder::finished_product()
{
	return &_product;
}

void parse_item::reset_fields()
{
	_table_string   = "FIX ME!";
	_description    = "Someone didn't initialize me correctly. Go get a coder.";

	_type   = Parse_Input_Types::USAGE_NOTE;
	
	_required		= false;
	_enforced_count = NO_ENFORCED_COUNT;
	_major_version_number_implemented = INVALID_VERSION;
	_minor_version_number_implemented = INVALID_VERSION;
	_revision_number_implemented	  = INVALID_VERSION;
	_major_version_number_deprecated  = INVALID_VERSION;
	_minor_version_number_deprecated  = INVALID_VERSION;
	_revision_number_deprecated		  = INVALID_VERSION;
	_deprecation_message			  = "";

}

// ====== Factory functions ======
parse_item_builder parse_item_builder::new_parse_item(SCP_string name)
{
	_product.reset_fields();
	_product._table_string = std::move(name);
	return *this;
}

parse_item_builder parse_item_builder::set_description(SCP_string description)
{
	_product._description = std::move(description);
	return *this;
}

parse_item_builder parse_item_builder::set_type(Parse_Input_Types type)
{
	_product._type = type;
	return *this;
}

parse_item_builder parse_item_builder::set_required(bool required)
{
	_product._required = required;
	return *this;
}

parse_item_builder parse_item_builder::set_enforced_count(int enforced_count)
{
	_product._enforced_count = enforced_count;
	return *this;
}

parse_item_builder parse_item_builder::set_implementation_version(int major_implementation_version, int minor_implementation_version, int revision_implementation_number)
{
	_product._major_version_number_implemented = major_implementation_version;
	_product._minor_version_number_implemented = minor_implementation_version;
	_product._revision_number_implemented = revision_implementation_number;

	return *this;
}

parse_item_builder parse_item_builder::set_deprecation_version(int major_deprecation_version, int minor_deprecation_version, int revision_deprecation_number)
{
	_product._major_version_number_deprecated = major_deprecation_version;
	_product._minor_version_number_deprecated = minor_deprecation_version;
	_product._revision_number_deprecated = revision_deprecation_number;
	return *this;
}

parse_item_builder parse_item_builder::set_deprecation_message(SCP_string deprecation_message)
{
	_product._deprecation_message = std::move(deprecation_message);
	return *this;
}






