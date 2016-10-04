
#include "scripting/api/bitops.h"

namespace scripting {
namespace api {

//**********LIBRARY: Bitwise Ops
ADE_LIB(l_BitOps, "BitOps", "bit", "Bitwise Operations library")

ADE_FUNC(AND, l_BitOps, "number, number", "Values for which bitwise boolean AND operation is performed", "number", "Result of the AND operation")
{

	int a, b, c;
	if(!ade_get_args(L, "ii", &a,&b))
		return ade_set_error(L, "i", 0);

	c = (a & b);

	return ade_set_args(L, "i", c);
}

ADE_FUNC(OR, l_BitOps, "number, number", "Values for which bitwise boolean OR operation is performed", "number", "Result of the OR operation")
{
	int a, b, c;
	if(!ade_get_args(L, "ii", &a,&b))
		return ade_set_error(L, "i", 0);

	c = (a | b);

	return ade_set_args(L, "i", c);
}

ADE_FUNC(XOR, l_BitOps, "number, number", "Values for which bitwise boolean XOR operation is performed", "number", "Result of the XOR operation")
{
	int a, b, c;
	if(!ade_get_args(L, "ii", &a,&b))
		return ade_set_error(L, "i", 0);

	c = (a ^ b);

	return ade_set_args(L, "i", c);
}

ADE_FUNC(toggleBit, l_BitOps, "number, number (bit)", "Toggles the value of the set bit in the given number for 32 bit integer", "number", "Result of the operation")
{
	int a, b, c;
	if(!ade_get_args(L, "ii", &a,&b))
		return ade_set_error(L, "i", 0);

	if (!((b >= 0) && (b < 32)))
		return ade_set_error(L, "i", 0);

	c = a ^ (1 << b);

	return ade_set_args(L, "i", c);
}

ADE_FUNC(checkBit, l_BitOps, "number, number (bit)", "Checks the value of the set bit in the given number for 32 bit integer", "boolean", "Was the bit true of false")
{
	int a, b;
	if(!ade_get_args(L, "ii", &a,&b))
		return ade_set_error(L, "i", 0);

	if (!((b >= 0) && (b < 32)))
		return ade_set_error(L, "i", 0);

	if(a & (1<<b))
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(addBit, l_BitOps, "number, number (bit)", "Performs inclusive or (OR) operation on the set bit of the value", "number", "Result of the operation")
{
	int a, b, c;
	if(!ade_get_args(L, "ii", &a,&b))
		return ade_set_error(L, "i", 0);

	if (!((b >= 0) && (b < 32)))
		return ade_set_error(L, "i", 0);

	c = (a | (1<<b));

	return ade_set_args(L, "i", c);
}

}
}