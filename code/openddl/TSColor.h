//
// This file is part of the Terathon Common Library, by Eric Lengyel.
// Copyright 1999-2021, Terathon Software LLC
//
// This software is licensed under the GNU General Public License version 3.
// Separate proprietary licenses are available from Terathon Software.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
// EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
//


#ifndef TSColor_h
#define TSColor_h


//# \component	Math Library
//# \prefix		Math/


#include "TSHalf.h"


#define TERATHON_COLORRGB 1
#define TERATHON_COLORRGBA 1
#define TERATHON_COLOR1U 1
#define TERATHON_COLOR2U 1
#define TERATHON_COLOR4U 1
#define TERATHON_COLOR1S 1
#define TERATHON_COLOR2S 1
#define TERATHON_COLOR4S 1
#define TERATHON_COLOR4H 1


namespace Terathon
{
	struct ConstColorRGBA;


	//# \class	ColorRGB	Encapsulates a floating-point RGB color.
	//
	//# The $ColorRGB$ class encapsulates a floating-point RGB color.
	//
	//# \def	class ColorRGB
	//
	//# \data	ColorRGB
	//
	//# \ctor	ColorRGB();
	//# \ctor	ColorRGB(float r, float g, float b);
	//
	//# \desc
	//# The $ColorRGB$ class encapsulates a color having floating-point red, green, and blue
	//# components in the range [0.0,&#x202F;1.0]. An additional alpha component is provided by the
	//# $@ColorRGBA@$ class. When a $ColorRGB$ object is converted to a $@ColorRGBA@$ object,
	//# the alpha component is assumed to be 1.0.
	//#
	//# The default constructor leaves the components of the color undefined. If the values
	//# $r$, $g$, and $b$ are supplied, then they are assigned to the red, green, and blue
	//# components of the color, respectively.
	//
	//# \operator	float& operator [](machine k);
	//#				Returns a reference to the $k$-th component of a color.
	//#				The value of $k$ must be 0, 1, or 2.
	//
	//# \operator	const float& operator [](machine k) const;
	//#				Returns a constant reference to the $k$-th component of a color.
	//#				The value of $k$ must be 0, 1, or 2.
	//
	//# \operator	ColorRGB& operator =(float s);
	//#				Sets all three components to the value $s$.
	//
	//# \operator	ColorRGB& operator +=(const ColorRGB& c);
	//#				Adds the color $c$.
	//
	//# \operator	ColorRGB& operator -=(const ColorRGB& c);
	//#				Subtracts the color $c$.
	//
	//# \operator	ColorRGB& operator *=(const ColorRGB& c);
	//#				Multiplies by the color $c$.
	//
	//# \operator	ColorRGB& operator *=(float s);
	//#				Multiplies all three components by $s$.
	//
	//# \operator	ColorRGB& operator /=(float s);
	//#				Divides all three components by $s$.
	//
	//# \action		ColorRGB operator -(const ColorRGB& c);
	//#				Returns the negation of the color $c$.
	//
	//# \action		ColorRGB operator +(const ColorRGB& c1, const ColorRGB& c2);
	//#				Returns the sum of the colors $c1$ and $c2$.
	//
	//# \action		ColorRGB operator -(const ColorRGB& c1, const ColorRGB& c2);
	//#				Returns the difference of the colors $c1$ and $c2$.
	//
	//# \action		ColorRGB operator *(const ColorRGB& c1, const ColorRGB& c2);
	//#				Returns the product of the colors $c1$ and $c2$.
	//
	//# \action		ColorRGB operator *(const ColorRGB& c, float s);
	//#				Returns the product of the color $c$ and the scalar $s$.
	//
	//# \action		ColorRGB operator *(float s, const ColorRGB& c);
	//#				Returns the product of the color $c$ and the scalar $s$.
	//
	//# \action		ColorRGB operator /(const ColorRGB& c, float s);
	//#				Returns the product of the color $c$ and the inverse of the scalar $s$.
	//
	//# \action		bool operator ==(const ColorRGB& c1, const ColorRGB& c2);
	//#				Returns a boolean value indicating whether the colors $c1$ and $c2$ are equal.
	//
	//# \action		bool operator !=(const ColorRGB& c1, const ColorRGB& c2);
	//#				Returns a boolean value indicating whether the colors $c1$ and $c2$ are not equal.
	//
	//# \action		float Luminance(const ColorRGB& c);
	//#				Returns the luminance value of the color $c$.
	//
	//# \also	$@ColorRGBA@$


	//# \function	ColorRGB::Set		Sets all three components of a color.
	//
	//# \proto	ColorRGB& Set(float r, float g, float b);
	//
	//# \param	r	The new red component.
	//# \param	g	The new green component.
	//# \param	b	The new blue component.
	//
	//# \desc
	//# The $Set$ function sets the red, green, and blue components of a color to the values
	//# given by the $r$, $g$, and $b$ parameters, respectively.
	//#
	//# The return value is a reference to the color object.


	//# \member		ColorRGB

	class ColorRGB
	{
		public:

			float		red;		//## The red component.
			float		green;		//## The green component.
			float		blue;		//## The blue component.

			inline ColorRGB() = default;

			ColorRGB(float r, float g, float b)
			{
				red = r;
				green = g;
				blue = b;
			}

			ColorRGB& Set(float r, float g, float b)
			{
				red = r;
				green = g;
				blue = b;
				return (*this);
			}

			void Set(float r, float g, float b) volatile
			{
				red = r;
				green = g;
				blue = b;
			}

			float& operator [](machine k)
			{
				return ((&red)[k]);
			}

			const float& operator [](machine k) const
			{
				return ((&red)[k]);
			}

			ColorRGB& operator =(const ColorRGB& c)
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				return (*this);
			}

			void operator =(const ColorRGB& c) volatile
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
			}

			ColorRGB& operator =(float s)
			{
				red = s;
				green = s;
				blue = s;
				return (*this);
			}

			void operator =(float s) volatile
			{
				red = s;
				green = s;
				blue = s;
			}

			ColorRGB& operator +=(const ColorRGB& c)
			{
				red += c.red;
				green += c.green;
				blue += c.blue;
				return (*this);
			}

			ColorRGB& operator -=(const ColorRGB& c)
			{
				red -= c.red;
				green -= c.green;
				blue -= c.blue;
				return (*this);
			}

			ColorRGB& operator *=(const ColorRGB& c)
			{
				red *= c.red;
				green *= c.green;
				blue *= c.blue;
				return (*this);
			}

			ColorRGB& operator *=(float s)
			{
				red *= s;
				green *= s;
				blue *= s;
				return (*this);
			}

			ColorRGB& operator /=(float s)
			{
				s = 1.0F / s;
				red *= s;
				green *= s;
				blue *= s;
				return (*this);
			}

			TERATHON_API void GetHexString(char *string) const;
			TERATHON_API ColorRGB& SetHexString(const char *string);
	};


	inline ColorRGB operator -(const ColorRGB& c)
	{
		return (ColorRGB(-c.red, -c.green, -c.blue));
	}

	inline ColorRGB operator +(const ColorRGB& c1, const ColorRGB& c2)
	{
		return (ColorRGB(c1.red + c2.red, c1.green + c2.green, c1.blue + c2.blue));
	}

	inline ColorRGB operator -(const ColorRGB& c1, const ColorRGB& c2)
	{
		return (ColorRGB(c1.red - c2.red, c1.green - c2.green, c1.blue - c2.blue));
	}

	inline ColorRGB operator *(const ColorRGB& c1, const ColorRGB& c2)
	{
		return (ColorRGB(c1.red * c2.red, c1.green * c2.green, c1.blue * c2.blue));
	}

	inline ColorRGB operator *(const ColorRGB& c, float s)
	{
		return (ColorRGB(c.red * s, c.green * s, c.blue * s));
	}

	inline ColorRGB operator *(float s, const ColorRGB& c)
	{
		return (ColorRGB(s * c.red, s * c.green, s * c.blue));
	}

	inline ColorRGB operator /(const ColorRGB& c, float s)
	{
		s = 1.0F / s;
		return (ColorRGB(c.red * s, c.green * s, c.blue * s));
	}

	inline bool operator ==(const ColorRGB& c1, const ColorRGB& c2)
	{
		return ((c1.red == c2.red) && (c1.green == c2.green) && (c1.blue == c2.blue));
	}

	inline bool operator !=(const ColorRGB& c1, const ColorRGB& c2)
	{
		return ((c1.red != c2.red) || (c1.green != c2.green) || (c1.blue != c2.blue));
	}

	inline float Luminance(const ColorRGB& c)
	{
		return (c.red * 0.212639F + c.green * 0.715169F + c.blue * 0.072192F);
	}


	//# \class	ColorRGBA	Encapsulates a floating-point RGBA color.
	//
	//# The $ColorRGBA$ class encapsulates a floating-point RGBA color.
	//
	//# \def	class ColorRGBA
	//
	//# \data	ColorRGBA
	//
	//# \ctor	ColorRGBA();
	//# \ctor	ColorRGBA(const ColorRGB& c, float a = 1.0F);
	//# \ctor	ColorRGBA(float r, float g, float b, float a = 1.0F);
	//
	//# \desc
	//# The $ColorRGBA$ class encapsulates a color having floating-point red, green, blue, and
	//# alpha components in the range [0.0,&#x202F;1.0]. When a $@ColorRGB@$ object is converted
	//# to a $ColorRGBA$ object, the alpha component is assumed to be 1.0.
	//#
	//# The default constructor leaves the components of the color undefined. If the values
	//# $r$, $g$, $b$, and $a$ are supplied, then they are assigned to the red, green, blue,
	//# and alpha components of the color, respectively.
	//
	//# \operator	float& operator [](machine k);
	//#				Returns a reference to the $k$-th component of a color.
	//#				The value of $k$ must be 0, 1, 2, or 3.
	//
	//# \operator	const float& operator [](machine k) const;
	//#				Returns a constant reference to the $k$-th component of a color.
	//#				The value of $k$ must be 0, 1, 2, or 3.
	//
	//# \operator	ColorRGBA& operator =(const ColorRGB& c);
	//#				Copies the red, green, and blue components of $c$, and assigns
	//#				the alpha component a value of 1.
	//
	//# \operator	ColorRGBA& operator =(float s);
	//#				Assigns the value $s$ to the red, green, and blue components, and
	//#				assigns the alpha component a value of 1.
	//
	//# \operator	ColorRGBA& operator +=(const ColorRGBA& c);
	//#				Adds the color $c$.
	//
	//# \operator	ColorRGBA& operator +=(const ColorRGB& c);
	//#				Adds the color $c$. The alpha component is not modified.
	//
	//# \operator	ColorRGBA& operator -=(const ColorRGBA& c);
	//#				Subtracts the color $c$.
	//
	//# \operator	ColorRGBA& operator -=(const ColorRGB& c);
	//#				Subtracts the color $c$. The alpha component is not modified.
	//
	//# \operator	ColorRGBA& operator *=(const ColorRGBA& c);
	//#				Multiplies by the color $c$.
	//
	//# \operator	ColorRGBA& operator *=(const ColorRGB& c);
	//#				Multiplies by the color $c$. The alpha component is not modified.
	//
	//# \operator	ColorRGBA& operator *=(float s);
	//#				Multiplies all four components by $s$.
	//
	//# \operator	ColorRGBA& operator /=(float s);
	//#				Divides all four components by $s$.
	//
	//# \action		ColorRGBA operator -(const ColorRGBA& c);
	//#				Returns the negation of the color $c$.
	//
	//# \action		ColorRGBA operator +(const ColorRGBA& c1, const ColorRGBA& c2);
	//#				Returns the sum of the colors $c1$ and $c2$.
	//
	//# \action		ColorRGBA operator +(const ColorRGBA& c1, const ColorRGB& c2);
	//#				Returns the sum of the colors $c1$ and $c2$. The alpha component of $c2$ is assumed to be 0.
	//
	//# \action		ColorRGBA operator -(const ColorRGBA& c1, const ColorRGBA& c2);
	//#				Returns the difference of the colors $c1$ and $c2$.
	//
	//# \action		ColorRGBA operator -(const ColorRGBA& c1, const ColorRGB& c2);
	//#				Returns the difference of the colors $c1$ and $c2$. The alpha component of $c2$ is assumed to be 0.
	//
	//# \action		ColorRGBA operator *(const ColorRGBA& c1, const ColorRGBA& c2);
	//#				Returns the product of the colors $c1$ and $c2$.
	//
	//# \action		ColorRGBA operator *(const ColorRGBA& c1, const ColorRGB& c2);
	//#				Returns the product of the colors $c1$ and $c2$. The alpha component of $c2$ is assumed to be 1.
	//
	//# \action		ColorRGBA operator *(const ColorRGB& c1, const ColorRGBA& c2);
	//#				Returns the product of the colors $c1$ and $c2$. The alpha component of $c1$ is assumed to be 1.
	//
	//# \action		ColorRGBA operator *(const ColorRGBA& c, float s);
	//#				Returns the product of the color $c$ and the scalar $s$.
	//
	//# \action		ColorRGBA operator *(float s, const ColorRGBA& c);
	//#				Returns the product of the color $c$ and the scalar $s$.
	//
	//# \action		ColorRGBA operator /(const ColorRGBA& c, float s);
	//#				Returns the product of the color $c$ and the inverse of the scalar $s$.
	//
	//# \action		bool operator ==(const ColorRGBA& c1, const ColorRGBA& c2);
	//#				Returns a boolean value indicating whether the two colors $c1$ and $c2$ are equal.
	//
	//# \action		bool operator ==(const ColorRGBA& c1, const ColorRGB& c2);
	//#				Returns a boolean value indicating whether the two colors $c1$ and $c2$ are equal. The alpha component of $c2$ is assumed to be 1.
	//
	//# \action		bool operator !=(const ColorRGBA& c1, const ColorRGBA& c2);
	//#				Returns a boolean value indicating whether the two colors $c1$ and $c2$ are not equal.
	//
	//# \action		bool operator !=(const ColorRGBA& c1, const ColorRGB& c2);
	//#				Returns a boolean value indicating whether the two colors $c1$ and $c2$ are not equal. The alpha component of $c2$ is assumed to be 1.
	//
	//# \action		float Luminance(const ColorRGBA& c);
	//#				Returns the luminance value of the color $c$.
	//
	//# \also	$@ColorRGB@$


	//# \function	ColorRGBA::Set		Sets all four components of a color.
	//
	//# \proto	ColorRGBA& Set(float r, float g, float b, float a = 1.0F);
	//
	//# \param	r	The new red component.
	//# \param	g	The new green component.
	//# \param	b	The new blue component.
	//# \param	a	The new alpha component.
	//
	//# \desc
	//# The $Set$ function sets the red, green, blue, and alpha components of a color to the values
	//# given by the $r$, $g$, $b$, and $a$ parameters, respectively.
	//#
	//# The return value is a reference to the color object.


	//# \function	ColorRGBA::GetColorRGB		Returns a reference to a $@ColorRGB@$ object.
	//
	//# \proto	ColorRGB& GetColorRGB(void);
	//# \proto	const ColorRGB& GetColorRGB(void) const;
	//
	//# \desc
	//# The $GetColorRGB$ function returns a reference to a $@ColorRGB@$ object that refers to
	//# the same data contained within the $ColorRGBA$ object.


	//# \member		ColorRGBA

	class ColorRGBA
	{
		public:

			float		red;		//## The red component.
			float		green;		//## The green component.
			float		blue;		//## The blue component.
			float		alpha;		//## The alpha component.

			inline ColorRGBA() = default;

			ColorRGBA(const ColorRGB& c, float a = 1.0F)
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = a;
			}

			ColorRGBA(float r, float g, float b, float a = 1.0F)
			{
				red = r;
				green = g;
				blue = b;
				alpha = a;
			}

			ColorRGBA& Set(const ColorRGB& c, float a)
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = a;
				return (*this);
			}

			void Set(const ColorRGB& c, float a) volatile
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = a;
			}

			ColorRGBA& Set(float r, float g, float b, float a = 1.0F)
			{
				red = r;
				green = g;
				blue = b;
				alpha = a;
				return (*this);
			}

			void Set(float r, float g, float b, float a = 1.0F) volatile
			{
				red = r;
				green = g;
				blue = b;
				alpha = a;
			}

			float& operator [](machine k)
			{
				return ((&red)[k]);
			}

			const float& operator [](machine k) const
			{
				return ((&red)[k]);
			}

			operator const ColorRGB&(void) const
			{
				return (reinterpret_cast<const ColorRGB&>(*this));
			}

			ColorRGB& GetColorRGB(void)
			{
				return (reinterpret_cast<ColorRGB&>(*this));
			}

			const ColorRGB& GetColorRGB(void) const
			{
				return (reinterpret_cast<const ColorRGB&>(*this));
			}

			ColorRGBA& operator =(const ColorRGBA& c)
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = c.alpha;
				return (*this);
			}

			void operator =(const ColorRGBA& c) volatile
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = c.alpha;
			}

			ColorRGBA& operator =(const ColorRGB& c)
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = 1.0F;
				return (*this);
			}

			void operator =(const ColorRGB& c) volatile
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = 1.0F;
			}

			ColorRGBA& operator =(float s)
			{
				red = s;
				green = s;
				blue = s;
				alpha = 1.0F;
				return (*this);
			}

			void operator =(float s) volatile
			{
				red = s;
				green = s;
				blue = s;
				alpha = 1.0F;
			}

			ColorRGBA& operator +=(const ColorRGBA& c)
			{
				red += c.red;
				green += c.green;
				blue += c.blue;
				alpha += c.alpha;
				return (*this);
			}

			ColorRGBA& operator +=(const ColorRGB& c)
			{
				red += c.red;
				green += c.green;
				blue += c.blue;
				return (*this);
			}

			ColorRGBA& operator -=(const ColorRGBA& c)
			{
				red -= c.red;
				green -= c.green;
				blue -= c.blue;
				alpha -= c.alpha;
				return (*this);
			}

			ColorRGBA& operator -=(const ColorRGB& c)
			{
				red -= c.red;
				green -= c.green;
				blue -= c.blue;
				return (*this);
			}

			ColorRGBA& operator *=(const ColorRGBA& c)
			{
				red *= c.red;
				green *= c.green;
				blue *= c.blue;
				alpha *= c.alpha;
				return (*this);
			}

			ColorRGBA& operator *=(const ColorRGB& c)
			{
				red *= c.red;
				green *= c.green;
				blue *= c.blue;
				return (*this);
			}

			ColorRGBA& operator *=(float s)
			{
				red *= s;
				green *= s;
				blue *= s;
				alpha *= s;
				return (*this);
			}

			ColorRGBA& operator /=(float s)
			{
				s = 1.0F / s;
				red *= s;
				green *= s;
				blue *= s;
				alpha *= s;
				return (*this);
			}

			TERATHON_API void GetHexString(char *string) const;
			TERATHON_API ColorRGBA& SetHexString(const char *string);
	};


	inline ColorRGBA operator -(const ColorRGBA& c)
	{
		return (ColorRGBA(-c.red, -c.green, -c.blue, -c.alpha));
	}

	inline ColorRGBA operator +(const ColorRGBA& c1, const ColorRGBA& c2)
	{
		return (ColorRGBA(c1.red + c2.red, c1.green + c2.green, c1.blue + c2.blue, c1.alpha + c2.alpha));
	}

	inline ColorRGBA operator +(const ColorRGBA& c1, const ColorRGB& c2)
	{
		return (ColorRGBA(c1.red + c2.red, c1.green + c2.green, c1.blue + c2.blue, c1.alpha));
	}

	inline ColorRGBA operator -(const ColorRGBA& c1, const ColorRGBA& c2)
	{
		return (ColorRGBA(c1.red - c2.red, c1.green - c2.green, c1.blue - c2.blue, c1.alpha - c2.alpha));
	}

	inline ColorRGBA operator -(const ColorRGBA& c1, const ColorRGB& c2)
	{
		return (ColorRGBA(c1.red - c2.red, c1.green - c2.green, c1.blue - c2.blue, c1.alpha));
	}

	inline ColorRGBA operator *(const ColorRGBA& c1, const ColorRGBA& c2)
	{
		return (ColorRGBA(c1.red * c2.red, c1.green * c2.green, c1.blue * c2.blue, c1.alpha * c2.alpha));
	}

	inline ColorRGBA operator *(const ColorRGBA& c1, const ColorRGB& c2)
	{
		return (ColorRGBA(c1.red * c2.red, c1.green * c2.green, c1.blue * c2.blue, c1.alpha));
	}

	inline ColorRGBA operator *(const ColorRGB& c1, const ColorRGBA& c2)
	{
		return (ColorRGBA(c1.red * c2.red, c1.green * c2.green, c1.blue * c2.blue, c2.alpha));
	}

	inline ColorRGBA operator *(const ColorRGBA& c, float s)
	{
		return (ColorRGBA(c.red * s, c.green * s, c.blue * s, c.alpha * s));
	}

	inline ColorRGBA operator *(float s, const ColorRGBA& c)
	{
		return (ColorRGBA(s * c.red, s * c.green, s * c.blue, s * c.alpha));
	}

	inline ColorRGBA operator /(const ColorRGBA& c, float s)
	{
		s = 1.0F / s;
		return (ColorRGBA(c.red * s, c.green * s, c.blue * s, c.alpha * s));
	}

	inline bool operator ==(const ColorRGBA& c1, const ColorRGBA& c2)
	{
		return ((c1.red == c2.red) && (c1.green == c2.green) && (c1.blue == c2.blue) && (c1.alpha == c2.alpha));
	}

	inline bool operator ==(const ColorRGBA& c1, const ColorRGB& c2)
	{
		return ((c1.red == c2.red) && (c1.green == c2.green) && (c1.blue == c2.blue) && (c1.alpha == 1.0F));
	}

	inline bool operator !=(const ColorRGBA& c1, const ColorRGBA& c2)
	{
		return ((c1.red != c2.red) || (c1.green != c2.green) || (c1.blue != c2.blue) || (c1.alpha != c2.alpha));
	}

	inline bool operator !=(const ColorRGBA& c1, const ColorRGB& c2)
	{
		return ((c1.red != c2.red) || (c1.green != c2.green) || (c1.blue != c2.blue) || (c1.alpha != 1.0F));
	}

	inline float Luminance(const ColorRGBA& c)
	{
		return (c.red * 0.212639F + c.green * 0.715169F + c.blue * 0.072192F);
	}


	struct ConstColorRGB
	{
		float		red;
		float		green;
		float		blue;

		operator const ColorRGB&(void) const
		{
			return (reinterpret_cast<const ColorRGB&>(*this));
		}

		const ColorRGB *operator &(void) const
		{
			return (reinterpret_cast<const ColorRGB *>(this));
		}

		const ColorRGB *operator ->(void) const
		{
			return (reinterpret_cast<const ColorRGB *>(this));
		}
	};


	struct ConstColorRGBA
	{
		float		red;
		float		green;
		float		blue;
		float		alpha;

		operator const ColorRGBA&(void) const
		{
			return (reinterpret_cast<const ColorRGBA&>(*this));
		}

		const ColorRGBA *operator &(void) const
		{
			return (reinterpret_cast<const ColorRGBA *>(this));
		}

		const ColorRGBA *operator ->(void) const
		{
			return (reinterpret_cast<const ColorRGBA *>(this));
		}
	};


	typedef uint8	Color1U;
	typedef int8	Color1S;


	//# \class	Color2U		Encapsulates a two-component unsigned integer color.
	//
	//# The $Color2U$ class encapsulates a two-component unsigned integer color.
	//
	//# \def	class Color2U
	//
	//# \ctor	Color2U();
	//# \ctor	Color2U(uint32 r, uint32 g);
	//
	//# \desc
	//# The $Color2U$ class encapsulates a color having two unsigned integer
	//# components in the range [0,&#x202F;255].
	//#
	//# The default constructor leaves the components of the color undefined.
	//
	//# \also	$@Color2S@$
	//# \also	$@Color4U@$
	//# \also	$@Color4S@$


	class Color2U
	{
		public:

			uint8		red;
			uint8		green;

			inline Color2U() = default;

			Color2U(uint32 r, uint32 g)
			{
				red = uint8(r);
				green = uint8(g);
			}

			Color2U& Set(uint32 r, uint32 g)
			{
				red = uint8(r);
				green = uint8(g);
				return (*this);
			}

			Color2U& Clear(void)
			{
				reinterpret_cast<uint16&>(*this) = 0;
				return (*this);
			}

			uint16 GetPackedColor(void) const
			{
				return (reinterpret_cast<const uint16&>(*this));
			}

			Color2U& SetPackedColor(uint16 c)
			{
				reinterpret_cast<uint16&>(*this) = c;
				return (*this);
			}

			void SetPackedColor(uint16 c) volatile
			{
				reinterpret_cast<volatile uint16&>(*this) = c;
			}

			Color2U& operator =(const Color2U& c)
			{
				reinterpret_cast<uint16&>(*this) = c.GetPackedColor();
				return (*this);
			}

			void operator =(const Color2U& c) volatile
			{
				reinterpret_cast<volatile uint16&>(*this) = c.GetPackedColor();
			}

			bool operator ==(const Color2U& c) const
			{
				return (GetPackedColor() == c.GetPackedColor());
			}

			bool operator !=(const Color2U& c) const
			{
				return (GetPackedColor() != c.GetPackedColor());
			}
	};


	//# \class	Color2S		Encapsulates a two-component signed integer color.
	//
	//# The $Color2S$ class encapsulates a two-component signed integer color.
	//
	//# \def	class Color2S
	//
	//# \ctor	Color2S();
	//# \ctor	Color2S(int32 r, int32 g);
	//
	//# \desc
	//# The $Color2S$ class encapsulates a color having two signed integer
	//# components in the range [&minus;127,&#x202F;+127].
	//#
	//# The default constructor leaves the components of the color undefined.
	//
	//# \also	$@Color2U@$
	//# \also	$@Color4S@$
	//# \also	$@Color4U@$


	class Color2S
	{
		public:

			int8		red;
			int8		green;

			inline Color2S() = default;

			Color2S(int32 r, int32 g)
			{
				red = int8(r);
				green = int8(g);
			}

			Color2S& Set(int32 r, int32 g)
			{
				red = int8(r);
				green = int8(g);
				return (*this);
			}

			Color2S& Clear(void)
			{
				reinterpret_cast<int16&>(*this) = 0;
				return (*this);
			}

			uint16 GetPackedColor(void) const
			{
				return (reinterpret_cast<const uint16&>(*this));
			}

			Color2S& SetPackedColor(uint16 c)
			{
				reinterpret_cast<uint16&>(*this) = c;
				return (*this);
			}

			void SetPackedColor(uint16 c) volatile
			{
				reinterpret_cast<volatile uint16&>(*this) = c;
			}

			Color2S& operator =(const Color2S& c)
			{
				reinterpret_cast<uint16&>(*this) = c.GetPackedColor();
				return (*this);
			}

			void operator =(const Color2S& c) volatile
			{
				reinterpret_cast<volatile uint16&>(*this) = c.GetPackedColor();
			}

			bool operator ==(const Color2S& c) const
			{
				return (GetPackedColor() == c.GetPackedColor());
			}

			bool operator !=(const Color2S& c) const
			{
				return (GetPackedColor() != c.GetPackedColor());
			}
	};


	//# \class	Color4U		Encapsulates a four-component unsigned integer color.
	//
	//# The $Color4U$ class encapsulates a four-component unsigned integer color.
	//
	//# \def	class Color4U
	//
	//# \ctor	Color4U();
	//# \ctor	Color4U(uint32 r, uint32 g, uint32 b, uint32 a = 255);
	//# \ctor	explicit Color4U(const ColorRGBA& c);
	//
	//# \desc
	//# The $Color4U$ class encapsulates a color having unsigned integer red, green, blue, and alpha
	//# components in the range [0,&#x202F;255].
	//#
	//# The default constructor leaves the components of the color undefined.
	//
	//# \also	$@Color4S@$
	//# \also	$@Color2U@$
	//# \also	$@Color2S@$


	class Color4U
	{
		public:

			uint8		red;
			uint8		green;
			uint8		blue;
			uint8		alpha;

			inline Color4U() = default;

			Color4U(uint32 r, uint32 g, uint32 b, uint32 a = 255)
			{
				red = uint8(r);
				green = uint8(g);
				blue = uint8(b);
				alpha = uint8(a);
			}

			explicit Color4U(const ColorRGBA& c)
			{
				red = uint8(c.red * 255.0F);
				green = uint8(c.green * 255.0F);
				blue = uint8(c.blue * 255.0F);
				alpha = uint8(c.alpha * 255.0F);
			}

			Color4U& Set(uint32 r, uint32 g, uint32 b, uint32 a = 255)
			{
				red = uint8(r);
				green = uint8(g);
				blue = uint8(b);
				alpha = uint8(a);
				return (*this);
			}

			void Set(uint32 r, uint32 g, uint32 b, uint32 a = 255) volatile
			{
				red = uint8(r);
				green = uint8(g);
				blue = uint8(b);
				alpha = uint8(a);
			}

			Color4U& Set(const ColorRGBA& c)
			{
				red = uint8(c.red * 255.0F);
				green = uint8(c.green * 255.0F);
				blue = uint8(c.blue * 255.0F);
				alpha = uint8(c.alpha * 255.0F);
				return (*this);
			}

			void Set(const ColorRGBA& c) volatile
			{
				red = uint8(c.red * 255.0F);
				green = uint8(c.green * 255.0F);
				blue = uint8(c.blue * 255.0F);
				alpha = uint8(c.alpha * 255.0F);
			}

			Color4U& Clear(void)
			{
				reinterpret_cast<uint32&>(*this) = 0;
				return (*this);
			}

			Color4U& ClearMaxAlpha(void)
			{
				reinterpret_cast<uint32&>(*this) = 0xFF000000;
				return (*this);
			}

			uint32 GetPackedColor(void) const
			{
				return (reinterpret_cast<const uint32&>(*this));
			}

			uint32 GetPackedRGBColor(void) const
			{
				return (reinterpret_cast<const uint32&>(*this) & 0x00FFFFFF);
			}

			Color4U& SetPackedColor(uint32 c)
			{
				reinterpret_cast<uint32&>(*this) = c;
				return (*this);
			}

			void SetPackedColor(uint32 c) volatile
			{
				reinterpret_cast<volatile uint32&>(*this) = c;
			}

			Color4U& operator =(const Color4U& c)
			{
				reinterpret_cast<uint32&>(*this) = c.GetPackedColor();
				return (*this);
			}

			void operator =(const Color4U& c) volatile
			{
				reinterpret_cast<volatile uint32&>(*this) = c.GetPackedColor();
			}

			Color4U& operator =(const ColorRGB& c)
			{
				red = uint8(c.red * 255.0F);
				green = uint8(c.green * 255.0F);
				blue = uint8(c.blue * 255.0F);
				alpha = 0xFF;
				return (*this);
			}

			void operator =(const ColorRGB& c) volatile
			{
				red = uint8(c.red * 255.0F);
				green = uint8(c.green * 255.0F);
				blue = uint8(c.blue * 255.0F);
				alpha = 0xFF;
			}

			Color4U& operator =(const ColorRGBA& c)
			{
				red = uint8(c.red * 255.0F);
				green = uint8(c.green * 255.0F);
				blue = uint8(c.blue * 255.0F);
				alpha = uint8(c.alpha * 255.0F);
				return (*this);
			}

			void operator =(const ColorRGBA& c) volatile
			{
				red = uint8(c.red * 255.0F);
				green = uint8(c.green * 255.0F);
				blue = uint8(c.blue * 255.0F);
				alpha = uint8(c.alpha * 255.0F);
			}

			bool operator ==(const Color4U& c) const
			{
				return (GetPackedColor() == c.GetPackedColor());
			}

			bool operator !=(const Color4U& c) const
			{
				return (GetPackedColor() != c.GetPackedColor());
			}
	};


	//# \class	Color4S		Encapsulates a four-component signed integer color.
	//
	//# The $Color4S$ class encapsulates a four-component signed integer color.
	//
	//# \def	class Color4S
	//
	//# \ctor	Color4S();
	//# \ctor	Color4S(int32 r, int32 g, int32 b, int32 a = 0);
	//
	//# \desc
	//# The $Color4S$ class encapsulates a color having signed integer red, green, blue, and alpha
	//# components in the range [&minus;127,&#x202F;+127].
	//#
	//# The default constructor leaves the components of the color undefined.
	//
	//# \also	$@Color4U@$
	//# \also	$@Color2S@$
	//# \also	$@Color2U@$


	class Color4S
	{
		public:

			int8		red;
			int8		green;
			int8		blue;
			int8		alpha;

			inline Color4S() = default;

			Color4S(int32 r, int32 g, int32 b, int32 a = 0)
			{
				red = int8(r);
				green = int8(g);
				blue = int8(b);
				alpha = int8(a);
			}

			Color4S& Set(int32 r, int32 g, int32 b, int32 a = 0)
			{
				red = int8(r);
				green = int8(g);
				blue = int8(b);
				alpha = int8(a);
				return (*this);
			}

			void Set(int32 r, int32 g, int32 b, int32 a = 0) volatile
			{
				red = int8(r);
				green = int8(g);
				blue = int8(b);
				alpha = int8(a);
			}

			Color4S& Clear(void)
			{
				reinterpret_cast<uint32&>(*this) = 0;
				return (*this);
			}

			uint32 GetPackedColor(void) const
			{
				return (reinterpret_cast<const uint32&>(*this));
			}

			Color4S& SetPackedColor(uint32 c)
			{
				reinterpret_cast<uint32&>(*this) = c;
				return (*this);
			}

			void SetPackedColor(uint32 c) volatile
			{
				reinterpret_cast<volatile uint32&>(*this) = c;
			}

			Color4S& operator =(const Color4S& c)
			{
				reinterpret_cast<uint32&>(*this) = c.GetPackedColor();
				return (*this);
			}

			void operator =(const Color4S& c) volatile
			{
				reinterpret_cast<volatile uint32&>(*this) = c.GetPackedColor();
			}

			bool operator ==(const Color4S& c) const
			{
				return (GetPackedColor() == c.GetPackedColor());
			}

			bool operator !=(const Color4S& c) const
			{
				return (GetPackedColor() != c.GetPackedColor());
			}
	};


	struct ConstColor4U
	{
		uint8		red;
		uint8		green;
		uint8		blue;
		uint8		alpha;

		operator const Color4U&(void) const
		{
			return (reinterpret_cast<const Color4U&>(*this));
		}

		const Color4U *operator &(void) const
		{
			return (reinterpret_cast<const Color4U *>(this));
		}

		const Color4U *operator ->(void) const
		{
			return (reinterpret_cast<const Color4U *>(this));
		}
	};


	class Color4H
	{
		public:

			Half		red;
			Half		green;
			Half		blue;
			Half		alpha;

			inline Color4H() = default;

			Color4H(float r, float g, float b, float a = 0.0F)
			{
				red = r;
				green = g;
				blue = b;
				alpha = a;
			}

			explicit Color4H(const ColorRGBA& c)
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = c.alpha;
			}

			Color4H& Set(float r, float g, float b, float a = 0.0F)
			{
				red = r;
				green = g;
				blue = b;
				alpha = a;
				return (*this);
			}

			void Set(float r, float g, float b, float a = 0.0F) volatile
			{
				red = r;
				green = g;
				blue = b;
				alpha = a;
			}

			Color4H& Set(const ColorRGBA& c)
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = c.alpha;
				return (*this);
			}

			void Set(const ColorRGBA& c) volatile
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = c.alpha;
			}

			Color4H& operator =(const Color4H& c)
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = c.alpha;
				return (*this);
			}

			void operator =(const Color4H& c) volatile
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = c.alpha;
			}

			Color4H& operator =(const ColorRGBA& c)
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = c.alpha;
				return (*this);
			}

			void operator =(const ColorRGBA& c) volatile
			{
				red = c.red;
				green = c.green;
				blue = c.blue;
				alpha = c.alpha;
			}

			bool operator ==(const Color4H& c) const
			{
				return ((red == c.red) && (green == c.green) && (blue == c.blue) && (alpha == c.alpha));
			}

			bool operator !=(const Color4H& c) const
			{
				return ((red != c.red) || (green != c.green) || (blue != c.blue) || (alpha != c.alpha));
			}
	};


	namespace Color
	{
		TERATHON_API extern const ConstColorRGBA black;
		TERATHON_API extern const ConstColorRGBA white;
		TERATHON_API extern const ConstColorRGBA transparent;
		TERATHON_API extern const ConstColorRGBA red;
		TERATHON_API extern const ConstColorRGBA green;
		TERATHON_API extern const ConstColorRGBA blue;
		TERATHON_API extern const ConstColorRGBA yellow;
		TERATHON_API extern const ConstColorRGBA cyan;
		TERATHON_API extern const ConstColorRGBA magenta;

		TERATHON_API extern const uint8 srgbLinearizationTable[256];
		TERATHON_API extern const uint8 srgbDelinearizationTable[256];
		TERATHON_API extern const float srgbFloatLinearizationTable[256];

		TERATHON_API float Linearize(float color);
		TERATHON_API float Delinearize(float color);
		TERATHON_API ColorRGB Linearize(const ColorRGB& color);
		TERATHON_API ColorRGB Delinearize(const ColorRGB& color);
		TERATHON_API ColorRGBA Linearize(const ColorRGBA& color);
		TERATHON_API ColorRGBA Delinearize(const ColorRGBA& color);
	}
}


#endif
