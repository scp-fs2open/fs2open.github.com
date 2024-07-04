
#include "bitops.h"
#include "scripting/ade_args.h"
#include "scripting/api/objs/enums.h"

namespace scripting {
namespace api {

//**********LIBRARY: Bitwise Ops
ADE_LIB(l_BitOps, "BitOps", "bit", "Bitwise Operations library");

ADE_FUNC(AND, l_BitOps, "number, number, [number, number, number, number, number, number, number, number]", "Values for which bitwise boolean AND operation is performed", "number", "Result of the AND operation")
{
	int a[10], c;
	int n = ade_get_args(L, "ii|iiiiiiii", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8], &a[9]);
	if (n < 2)
		return ade_set_error(L, "i", 0);

	c = a[0];
	for (int i = 1; i < n; ++i)
		c &= a[i];

	return ade_set_args(L, "i", c);
}

ADE_FUNC(OR, l_BitOps, "number, number, [number, number, number, number, number, number, number, number]", "Values for which bitwise boolean OR operation is performed", "number", "Result of the OR operation")
{
	int a[10], c;
	int n = ade_get_args(L, "ii|iiiiiiii", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8], &a[9]);
	if (n < 2)
		return ade_set_error(L, "i", 0);

	c = a[0];
	for (int i = 1; i < n; ++i)
		c |= a[i];

	return ade_set_args(L, "i", c);
}

ADE_FUNC(EnumAND, l_BitOps, "enumeration, enumeration, [enumeration, enumeration, enumeration, enumeration, enumeration, enumeration, enumeration, enumeration]", "Values for which bitwise boolean AND operation is performed", "number", "Result of the AND operation")
{
	enum_h a[10];
	enum_h c;
	int n = ade_get_args(L, "oo|oooooooo", l_Enum.Get(&a[0]), l_Enum.Get(&a[1]), l_Enum.Get(&a[2]), l_Enum.Get(&a[3]), l_Enum.Get(&a[4]), l_Enum.Get(&a[5]), l_Enum.Get(&a[6]), l_Enum.Get(&a[7]), l_Enum.Get(&a[8]), l_Enum.Get(&a[9]));
	if (n < 2)
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	c = a[0];
	for (int i = 1; i < n; ++i)
		c = c & a[i];

	return ade_set_args(L, "o", l_Enum.Set(c));
}

ADE_FUNC(EnumOR, l_BitOps, "enumeration, enumeration, [enumeration, enumeration, enumeration, enumeration, enumeration, enumeration, enumeration, enumeration]", "Values for which bitwise boolean OR operation is performed", "number", "Result of the OR operation")
{
	enum_h a[10];
	enum_h c;
	int n = ade_get_args(L, "oo|oooooooo", l_Enum.Get(&a[0]), l_Enum.Get(&a[1]), l_Enum.Get(&a[2]), l_Enum.Get(&a[3]), l_Enum.Get(&a[4]), l_Enum.Get(&a[5]), l_Enum.Get(&a[6]), l_Enum.Get(&a[7]), l_Enum.Get(&a[8]), l_Enum.Get(&a[9]));
	if (n < 2)
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	c = a[0];
	for (int i = 1; i < n; ++i)
		c = c | a[i];

	return ade_set_args(L, "o", l_Enum.Set(c));
}

ADE_FUNC(XOR, l_BitOps, "number, number", "Values for which bitwise boolean XOR operation is performed", "number", "Result of the XOR operation")
{
	int a, b, c;
	if(!ade_get_args(L, "ii", &a,&b))
		return ade_set_error(L, "i", 0);

	c = (a ^ b);

	return ade_set_args(L, "i", c);
}

ADE_FUNC(toggleBit, l_BitOps, "number baseNumber, number bit", "Toggles the value of the set bit in the given number for 32 bit integer", "number", "Result of the operation")
{
	int a, b, c;
	if(!ade_get_args(L, "ii", &a,&b))
		return ade_set_error(L, "i", 0);

	if (!((b >= 0) && (b < 32)))
		return ade_set_error(L, "i", 0);

	c = a ^ (1 << b);

	return ade_set_args(L, "i", c);
}

ADE_FUNC(checkBit, l_BitOps, "number baseNumber, number bit", "Checks the value of the set bit in the given number for 32 bit integer", "boolean", "Was the bit true of false")
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

ADE_FUNC_DEPRECATED(addBit,
	l_BitOps,
	"number baseNumber, number bit",
	"Performs inclusive or (OR) operation on the set bit of the value",
	"number",
	"Result of the operation",
	gameversion::version(21, 4),
	"BitOps.addBit has been replaced by BitOps.setBit")
{
	int a, b, c;
	if(!ade_get_args(L, "ii", &a,&b))
		return ade_set_error(L, "i", 0);

	if (!((b >= 0) && (b < 32)))
		return ade_set_error(L, "i", 0);

	c = (a | (1<<b));

	return ade_set_args(L, "i", c);
}

ADE_FUNC(setBit, l_BitOps, "number baseNumber, number bit", "Turns on the specified bit of baseNumber (sets it to 1)", "number", "Result of the operation")
{
	int a, b, c;
	if (!ade_get_args(L, "ii", &a, &b))
		return ade_set_error(L, "i", 0);

	if (!((b >= 0) && (b < 32)))
		return ade_set_error(L, "i", 0);

	c = (a | (1 << b));

	return ade_set_args(L, "i", c);
}

ADE_FUNC(unsetBit, l_BitOps, "number baseNumber, number bit", "Turns off the specified bit of baseNumber (sets it to 0)", "number", "Result of the operation")
{
	int a, b, c;
	if (!ade_get_args(L, "ii", &a, &b))
		return ade_set_error(L, "i", 0);

	if (!((b >= 0) && (b < 32)))
		return ade_set_error(L, "i", 0);

	c = (a & ~(1 << b));

	return ade_set_args(L, "i", c);
}

}
}
