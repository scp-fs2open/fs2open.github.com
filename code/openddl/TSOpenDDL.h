//
// This file is part of the Terathon OpenDDL Library, by Eric Lengyel.
// Copyright 1999-2021, Terathon Software LLC
//
// This software is licensed under the GNU General Public License version 3.
// Separate proprietary licenses are available from Terathon Software.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
// EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
//


#ifndef TSOpenDDL_h
#define TSOpenDDL_h


//# \component	System Utilities
//# \prefix		System/


#include "TSData.h"
#include "TSTree.h"
#include "TSMap.h"


#define TERATHON_OPENDDL 1


namespace Terathon
{
	typedef uint32		StructureType;


	enum
	{
		kDataMaxPrimitiveArraySize			= 256
	};


	enum : StructureType
	{
		kStructureRoot						= 0,
		kStructurePrimitive					= 'PRIM',
		kStructureUnknown					= '!UNK'
	};


	//# \enum	DataProcessResult

	enum : DataResult
	{
		kDataMissingSubstructure			= 'MSSB',		//## A structure is missing a substructure of a required type.
		kDataExtraneousSubstructure			= 'EXSB',		//## A structure contains too many substructures of a legal type.
		kDataInvalidDataFormat				= 'IVDF',		//## The primitive data contained in a structure uses an invalid format (type, subarray size, or state data).
		kDataBrokenRef						= 'BREF'		//## The target of a reference does not exist.
	};


	class DataDescription;


	//# \class	Structure		Represents a data structure in an OpenDDL file.
	//
	//# The $Structure$ class represents a data structure in an OpenDDL file.
	//
	//# \def	class Structure : public Tree<Structure>, public MapElement<Structure>
	//
	//# \ctor	Structure(StructureType type);
	//
	//# \param	type	The type of the structure.
	//
	//# \desc
	//# The $Structure$ class is the base class for objects that represent data structures in an Open Data
	//# Description Language (OpenDDL) file. Structures of a specific data type are represented by objects whose
	//# types are subclasses of the $Structure$ class.
	//#
	//# Structures corresponding to built-in primitive data types are represented by objects whose type is
	//# a specialization of the $@DataStructure@$ class template, and these all have a common base class of
	//# type $@PrimitiveStructure@$ that is a direct subclass of the $Structure$ class.
	//#
	//# Custom data structures defined by specific OpenDDL-based file formats are represented by application-defined
	//# subclasses of the $Structure$ class. When an OpenDDL file is parsed, the $@DataDescription::CreateStructure@$
	//# function is called to construct the proper subclass for a given type identifier.
	//#
	//# $Structure$ objects are organized into a tree hierarchy that can be traversed using the functions of the
	//# $@Utilities/Tree@$ base class. The tree hierarchy corresponds to the data layout in the OpenDDL file.
	//#
	//# Subclasses for custom data structures should specify a unique 32-bit integer for the $type$ parameter, normally
	//# represented by a four-character code. All four-character codes consisting only of uppercase letters and decimal
	//# digits are reserved for use by the engine.
	//
	//# \base	Utilities/Tree<Structure>			$Structure$ objects are organized in a tree hierarchy.
	//# \base	Utilities/MapElement<Structure>		Used internally by the $DataDescription$ class.
	//
	//# \also	$@PrimitiveStructure@$
	//# \also	$@DataStructure@$
	//# \also	$@DataDescription@$
	//
	//# \wiki	Open_Data_Description_Language		Open Data Description Language


	//# \function	Structure::GetStructureType		Returns the structure type.
	//
	//# \proto	StructureType GetStructureType(void) const;
	//
	//# \desc
	//# The $GetStructureType$ function returns the structure type. This may be a custom data type or one of
	//# the following built-in primitive data types.
	//
	//# \table	DataType
	//
	//# \also	$@Structure::GetBaseStructureType@$
	//# \also	$@PrimitiveStructure@$
	//# \also	$@DataStructure@$


	//# \function	Structure::GetBaseStructureType		Returns the base structure type.
	//
	//# \proto	StructureType GetBaseStructureType(void) const;
	//
	//# \desc
	//# The $GetBaseStructureType$ function returns the base structure type representing a more general classification
	//# than the type returned by the $@Structure::GetStructureType@$ function. By default, the base structure type
	//# is simply the value zero, but a subclass of the $Structure$ class may set the base structure type to any value
	//# it wants by calling the $@Structure::SetBaseStructureType@$ function.
	//#
	//# The base structure type for all built-in primitive data structures derived from the $@PrimitiveStructure@$
	//# class is $kStructurePrimitive$.
	//
	//# \also	$@Structure::SetBaseStructureType@$
	//# \also	$@Structure::GetStructureType@$
	//# \also	$@PrimitiveStructure@$


	//# \function	Structure::SetBaseStructureType		Returns the base structure type.
	//
	//# \proto	void SetBaseStructureType(StructureType type);
	//
	//# \param	type	The base structure type.
	//
	//# \desc
	//# The $GetBaseStructureType$ function sets the base structure type to that specified by the $type$ parameter.
	//# The base structure type represents a more general classification than the type returned by the
	//# $@Structure::GetStructureType@$ function. A subclass of the $Structure$ class may set the base structure
	//# type to a custom value in order to indicate that the data structure belongs to a particular category.
	//#
	//# Subclasses for custom data structures should specify a unique 32-bit integer for the $type$ parameter, normally
	//# represented by a four-character code. All four-character codes consisting only of uppercase letters and decimal
	//# digits are reserved for use by the engine.
	//#
	//# By default, the base structure type for custom subclasses of the $Structure$ class is the value zero.
	//
	//# \also	$@Structure::GetBaseStructureType@$
	//# \also	$@Structure::GetStructureType@$


	//# \function	Structure::GetStructureName		Returns the structure name.
	//
	//# \proto	const char *GetStructureName(void) const;
	//
	//# \desc
	//# The $GetStructureName$ function returns the name of a structure. The dollar sign or percent sign at the
	//# beginning is omitted. If a structure has no name, then the return value points to an empty string&mdash;it is
	//# never $nullptr$.
	//#
	//# Whether the structure's name is global or local can be determined by calling the $@Structure::GetGlobalNameFlag@$ function.
	//
	//# \also	$@Structure::GetGlobalNameFlag@$
	//# \also	$@Structure::GetStructureType@$
	//# \also	$@Structure::GetBaseStructureType@$


	//# \function	Structure::GetGlobalNameFlag	Returns a boolean value indicating whether a structure's name is global.
	//
	//# \proto	bool GetGlobalNameFlag(void) const;
	//
	//# \desc
	//# The $GetGlobalNameFlag$ function returns $true$ if the structure's name is global, and it returns
	//# $false$ if the structure's name is local. A structure's name is global if it begins with a dollar sign in the
	//# OpenDDL file, and the name is local if it begins with a percent sign.
	//
	//# \also	$@Structure::GetStructureName@$


	//# \function	Structure::FindStructure		Finds a named structure using a local reference.
	//
	//# \proto	Structure *FindStructure(const StructureRef& reference, int32 index = 0) const;
	//
	//# \param	reference	The reference to the structure to find.
	//# \param	index		The index of the name to search for within the reference's name array. This is used internally and should be set to its default value of 0.
	//
	//# \desc
	//# The $FindStructure$ function finds the structure referenced by the sequence of names stored in the
	//# $reference$ parameter and returns a pointer to it. If no such structure exists, then the return value is $nullptr$.
	//# Only structures belonging to the subtree of the structure for which this function is called can be
	//# returned by this function. The sequence of names in the reference identify a branch along the subtree
	//# leading to the referenced structure.
	//#
	//# The reference must be a local reference, meaning that the first name stored in the reference is a
	//# local name as indicated by a value of $false$ being returned by the $@StructureRef::GetGlobalRefFlag@$ function.
	//# If the reference is not a local reference, then this function always returns $nullptr$. The $@DataDescription::FindStructure@$
	//# function should be used to find a structure through a global reference.
	//#
	//# If the specified reference has an empty name array, then the return value is always $nullptr$. The empty name
	//# array is assigned to a reference data value when $null$ appears in the OpenDDL file.
	//
	//# \also	$@StructureRef@$
	//# \also	$@Structure::GetStructureName@$
	//# \also	$@Structure::GetGlobalNameFlag@$
	//# \also	$@DataDescription::FindStructure@$
	//# \also	$@DataDescription::GetRootStructure@$


	//# \function	Structure::ValidateProperty		Determines the validity of a property and returns its type and location.
	//
	//# \proto	virtual bool ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value);
	//
	//# \param	dataDescription		The data description object to which the structure belongs.
	//# \param	identifier			The property identifier, as read from an OpenDDL file.
	//# \param	type				A pointer to the location that receives the data type for the property.
	//# \param	value				A pointer to the location that receives a pointer to the property's value.
	//
	//# \desc
	//# The $ValidateProperty$ function is called for each property specified in an OpenDDL file for a particular
	//# data structure to determine whether the property is valid, and if so, what type it expects and where
	//# to store its value. This function should be overridden by any subclass of the $Structure$ class that
	//# defines properties, and it should return $true$ when the $identifier$ parameter identifies one of the
	//# supported properties. If the string specified by the $identifier$ parameter is not recognized, then
	//# the function should return $false$. The default implementation of the $ValidateProperty$ function
	//# always returns $false$.
	//#
	//# When the property identifier is valid, an implementation of the $ValidateProperty$ function must write
	//# the type of data expected by the property to the location specified by the $type$ parameter, and it must
	//# write a pointer to the location holding the property value to the location specified by the $value$
	//# parameter. The data type must be one of the following values.
	//
	//# \table	DataType
	//
	//# For the string and reference data types, the property value must be represented by a $@Utilities/String@$
	//# object with the default template parameter of 0.
	//#
	//# An implementation of the $ValidateProperty$ function must always return the same results for any given
	//# property identifier. If the same property appears multiple times in the property list for a structure,
	//# then values appearing later must overwrite earlier values, and the earlier values must be ignored.
	//
	//# \also	$@Structure::ValidateSubstructure@$
	//# \also	$@DataDescription@$


	//# \function	Structure::ValidateSubstructure		Determines the validity of a substructure.
	//
	//# \proto	virtual bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const;
	//
	//# \param	dataDescription		The data description object to which the structure belongs.
	//# \param	structure			The substructure to validate.
	//
	//# \desc
	//# The $ValidateSubstructure$ function is called for the $Structure$ object representing the enclosing data
	//# structure each time a new substructure is created to determine whether the new substructure can legally
	//# be contained in the data of the $Structure$ object. An overriding implementation should examine the
	//# structure specified by the $structure$ parameter and return $true$ if it can legally appear as a direct
	//# subnode of the $Structure$ object for which the $ValidateSubstructure$ function is called. Otherwise, the
	//# function should return $false$.
	//#
	//# An implementation would typically call the $@Structure::GetStructureType@$ function to make its decision,
	//# but other information such as the base structure type or the primitive subarray size may also be taken into
	//# account. At the time that the $ValidateSubstructure$ function is called, no data belonging to the structure
	//# is available, so the data itself cannot be used to validate any substructures.
	//#
	//# The default implementation of the $ValidateSubstructure$ function always returns $true$.
	//
	//# \also	$@Structure::ValidateProperty@$
	//# \also	$@Structure::GetStructureType@$
	//# \also	$@Structure::GetBaseStructureType@$
	//# \also	$@PrimitiveStructure::GetArraySize@$
	//# \also	$@DataDescription::ValidateTopLevelStructure@$


	//# \function	Structure::GetStateValue		Translates a state identifier into a state value.
	//
	//# \proto	virtual bool GetStateValue(const String<>& identifier, uint32 *state) const;
	//
	//# \param	identifier		The state identifier specified in the data.
	//# \param	state			A pointer to the location to which the corresponding state value is returned.
	//
	//# \desc
	//# The $GetStateValue$ function is called for the $Structure$ object representing the enclosing data
	//# structure each time a state identifier is encountered in data containing subarrays. An overriding
	//# implementation should examine the state specified by the $identifier$ parameter and store the
	//# corresponding state value in the location specified by the $state$ parameter. If the state identifier
	//# is recognized, then the function should return $true$. If the state identifier is not recognized,
	//# then the function should return $false$.


	//# \function	Structure::ProcessData		Performs custom processing of the structure data.
	//
	//# \proto	virtual DataResult ProcessData(DataDescription *dataDescription);
	//
	//# \param	dataDescription		The $@DataDescription@$ object to which the structure belongs.
	//
	//# \desc
	//# The $ProcessData$ function can be overridden by a subclass to perform any custom processing of the data
	//# contained in a structure. This function is called for all direct subnodes of the root structure of
	//# a data file when the $@DataDescription::ProcessText@$ function is called. (These correspond to the
	//# top-level data structures in the file itself.) The implementation may examine the structure's subtree
	//# and take whatever action is appropriate to process the data.
	//#
	//# The $dataDescription$ parameter points to the $@DataDescription@$ object to which the structure belongs.
	//# An implementation of the $ProcessData$ function may call the $@DataDescription::FindStructure@$ function
	//# to find referenced structures.
	//#
	//# If no error occurs, then the $ProcessData$ function should return $kDataOkay$. Otherwise, an error
	//# code should be returned. An implementation may return a custom error code or one of the following
	//# standard error codes.
	//
	//# \table	DataProcessResult
	//
	//# The default implementation calls the $ProcessData$ function for each of the direct subnodes of a data
	//# structure. If an error is returned by any of these calls, then this function stops iterating through
	//# its subnodes and returns the error code immediately.
	//#
	//# An overriding implementation of the $ProcessData$ function is not required to call the base class
	//# implementation, but if it does, it must pass the value of the $dataDescription$ parameter through
	//# to the base class.
	//
	//# \also	$@DataDescription::ProcessText@$


	class Structure : public Tree<Structure>, public MapElement<Structure>
	{
		friend class DataDescription;

		public:

			typedef ConstCharKey KeyType;

		private:

			StructureType		structureType;
			StructureType		baseStructureType;

			String<>			structureName;
			bool				globalNameFlag;

			Map<Structure>		structureMap;

			const char			*textLocation;

		protected:

			TERATHON_API Structure(StructureType type);

			void SetBaseStructureType(StructureType type)
			{
				baseStructureType = type;
			}

		public:

			TERATHON_API virtual ~Structure();

			KeyType GetKey(void) const
			{
				return (structureName);
			}

			StructureType GetStructureType(void) const
			{
				return (structureType);
			}

			StructureType GetBaseStructureType(void) const
			{
				return (baseStructureType);
			}

			const char *GetStructureName(void) const
			{
				return (structureName);
			}

			bool GetGlobalNameFlag(void) const
			{
				return (globalNameFlag);
			}

			TERATHON_API Structure *GetFirstSubstructure(StructureType type) const;
			TERATHON_API Structure *GetLastSubstructure(StructureType type) const;

			TERATHON_API Structure *FindStructure(const StructureRef& reference, int32 index = 0) const;

			TERATHON_API virtual bool ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value);
			TERATHON_API virtual bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const;

			TERATHON_API virtual bool GetStateValue(const String<>& identifier, uint32 *state) const;
			TERATHON_API virtual DataResult ProcessData(DataDescription *dataDescription);
	};


	//# \class	PrimitiveStructure		Base class for built-in primitive data structures in an OpenDDL file.
	//
	//# The $PrimitiveStructure$ class is the base class for built-in primitive data structures in an OpenDDL file.
	//
	//# \def	class PrimitiveStructure : public Structure
	//
	//# \ctor	PrimitiveStructure(StructureType type);
	//
	//# \param	type	The type of the structure.
	//
	//# \desc
	//# The $PrimitiveStructure$ class is the base class for all objects that represent built-in primitive
	//# data structures in an Open Data Description Language (OpenDDL) file. Specific types of primitive data
	//# structures are represented by specializations of the $@DataStructure@$ class template.
	//#
	//# The base structure type returned by the $@Structure::GetBaseStructureType@$ function for all built-in
	//# primitive data structures is $kStructurePrimitive$.
	//
	//# \base	Structure		A primitive structure is a special type of $Structure$ object.
	//
	//# \also	$@DataStructure@$


	//# \function	PrimitiveStructure::GetArraySize		Returns the array size for a primitive data type.
	//
	//# \proto	uint32 GetArraySize(void) const;
	//
	//# \desc
	//# The $GetArraySize$ function returns the number of elements in each subarray contained in the data
	//# belonging to a primitive data structure. This corresponds to the number specified in brackets after
	//# the type identifier. If no array size is specified for a structure, then the $GetArraySize$ function
	//# returns zero.
	//
	//# \also	$@DataStructure::GetArrayDataElement@$


	//# \function	PrimitiveStructure::GetStateFlag		Returns a boolean value indicating whether state data is present.
	//
	//# \proto	bool GetStateFlag(void) const;
	//
	//# \desc
	//# The $GetStateFlag$ function returns $true$ if the primitive data structure contains state data and
	//# $false$ if it does not. State data can exist only for primitive data that has subarrays, so the
	//# $GetStateFlag$ function always returns $false$ if the $@PrimitiveStructure::GetArraySize@$ function
	//# returns zero.
	//
	//# \also	$@DataStructure::GetArrayStateElement@$


	class PrimitiveStructure : public Structure
	{
		friend class DataDescription;

		private:

			uint32		arraySize;
			bool		stateFlag;

		protected:

			PrimitiveStructure(StructureType type);
			PrimitiveStructure(StructureType type, uint32 size, bool state);

		public:

			~PrimitiveStructure();

			uint32 GetArraySize(void) const
			{
				return (arraySize);
			}

			bool GetStateFlag(void) const
			{
				return (stateFlag);
			}

			virtual DataResult ParseData(const char *& text) = 0;
	};


	//# \class	DataStructure		Represents a specific built-in primitive data structure in an OpenDDL file.
	//
	//# The $DataStructure$ class template represents each of the specific built-in primitive data structure in an OpenDDL file.
	//
	//# \def	template <class type> class DataStructure final : public PrimitiveStructure
	//
	//# \tparam		type	An object type representing the specific type of data contained in the structure.
	//
	//# \ctor	DataStructure();
	//
	//# \desc
	//# A specialization of the $DataStructure$ class template represents each of the built-in primitive data structures in
	//# an Open Data Description Language (OpenDDL) file. The $type$ template parameter can only be one of the following types.
	//
	//# \value	BoolDataType		A boolean type that can have the value $true$ or $false$.
	//# \value	Int8DataType		An 8-bit signed integer that can have values in the range [&minus;2<sup>7</sup>,&#x202F;2<sup>7</sup>&#x202F;&mins;&#x202F;1].
	//# \value	Int16DataType		A 16-bit signed integer that can have values in the range [&minus;2<sup>15</sup>,&#x202F;2<sup>15</sup>&#x202F;&mins;&#x202F;1].
	//# \value	Int32DataType		A 32-bit signed integer that can have values in the range [&minus;2<sup>31</sup>,&#x202F;2<sup>31</sup>&#x202F;&mins;&#x202F;1].
	//# \value	Int64DataType		A 64-bit signed integer that can have values in the range [&minus;2<sup>63</sup>,&#x202F;2<sup>63</sup>&#x202F;&mins;&#x202F;1].
	//# \value	UInt8DataType		An 8-bit unsigned integer that can have values in the range [0,&#x202F;2<sup>8&#x202F;-&#x202F;1].
	//# \value	UInt16DataType		A 16-bit unsigned integer that can have values in the range [0,&#x202F;2<sup>16&#x202F;-&#x202F;1].
	//# \value	UInt32DataType		A 32-bit unsigned integer that can have values in the range [0,&#x202F;2<sup>32&#x202F;-&#x202F;1].
	//# \value	UInt64DataType		A 64-bit unsigned integer that can have values in the range [0,&#x202F;2<sup>64&#x202F;-&#x202F;1].
	//# \value	HalfDataType		A 16-bit floating-point type conforming to the standard S1E5M10 format.
	//# \value	FloatDataType		A 32-bit floating-point type conforming to the standard S1E8M23 format.
	//# \value	DoubleDataType		A 64-bit floating-point type conforming to the standard S1E11M52 format.
	//# \value	StringDataType		A double-quoted character string with contents encoded in UTF-8.
	//# \value	RefDataType			A sequence of structure names, or the keyword $null$.
	//# \value	TypeDataType		A type whose values are identifiers naming types in the first column of this table.
	//# \value	Base64Type			A raw binary data type with contents encoded as base64.
	//
	//# \desc
	//# The raw data belonging to a data structure is stored as a linear array in memory, regardless of whether subarrays
	//# are specified. The total number of data elements present can be retrieved with the $@DataStructure::GetDataElementCount@$
	//# function, and the data for each element can be retrieved with the $@DataStructure::GetDataElement@$ function.
	//# If subarrays are in use, then the elements belonging to each subarray are stored contiguously, and each subarray
	//# is then stored contiguously with the one preceding it. The $@DataStructure::GetArrayDataElement@$ function can be
	//# used to retrieve a pointer to the beginning of a specific subarray.
	//
	//# \base	PrimitiveStructure		Each data structure specialization is a specific type of $PrimitiveStructure$ object.


	//# \function	DataStructure::GetDataElementCount		Returns the total number of data elements stored in a data structure.
	//
	//# \proto	int32 GetDataElementCount(void) const;
	//
	//# \desc
	//# The $GetDataElementCount$ function returns the total number of individual primitive data elements stored in a
	//# data structure. If a data structure contains subarrays, then this function returns the number of elements in
	//# each subarray multiplied by the total number of subarrays.
	//
	//# \also	$@DataStructure::GetDataElement@$
	//# \also	$@DataStructure::GetArrayDataElement@$
	//# \also	$@PrimitiveStructure::GetArraySize@$


	//# \function	DataStructure::GetDataElement		Returns a single data element stored in a data structure.
	//
	//# \proto	const PrimType& GetDataElement(int32 index) const;
	//
	//# \param	index	The zero-based index of the data element to retrieve.
	//
	//# \desc
	//# The $GetDataElement$ function returns a single primitive data element stored in a data structure. The legal values
	//# of the $index$ parameter range from zero to <i>n</i>&nbsp;&minus;1, inclusive, where <i>n</i> is the total count of
	//# data elements returned by the $@DataStructure::GetDataElementCount@$ function.
	//#
	//# The $PrimType$ type is defined by the class corresponding to the $type$ template parameter associated
	//# with the particular specialization of the $DataStructure$ class template.
	//
	//# \also	$@DataStructure::GetArrayDataElement@$
	//# \also	$@DataStructure::GetDataElementCount@$
	//# \also	$@PrimitiveStructure::GetArraySize@$


	//# \function	DataStructure::GetArrayDataElement		Returns a pointer to a subarray stored in a data structure.
	//
	//# \proto	const PrimType *GetArrayDataElement(int32 index) const;
	//
	//# \param	index	The zero-based index of the subarray to retrieve.
	//
	//# \desc
	//# The $GetArrayDataElement$ function returns a pointer to the first element in a subarray stored in a data structure.
	//# The legal values of the $index$ parameter range from zero to <i>n</i>&nbsp;/&nbsp;<i>s</i>&nbsp;&minus;1, inclusive,
	//# where <i>n</i> is the total count of data elements returned by the $@DataStructure::GetDataElementCount@$ function,
	//# and <i>s</i> is the size of each subarray returned by the $@PrimitiveStructure::GetArraySize@$ function. The elements
	//# of each subarray are stored contiguously in memory.
	//#
	//# The $PrimType$ type is defined by the class corresponding to the $type$ template parameter associated
	//# with the particular specialization of the $DataStructure$ class template.
	//
	//# \also	$@DataStructure::GetDataElement@$
	//# \also	$@DataStructure::GetDataElementCount@$
	//# \also	$@DataStructure::GetArrayStateElement@$
	//# \also	$@PrimitiveStructure::GetArraySize@$


	//# \function	DataStructure::GetArrayStateElement		Returns the state associated with a single subarray in a data structure.
	//
	//# \proto	const uint32& GetArrayStateElement(int32 index) const;
	//
	//# \param	index	The zero-based index of the subarray for which to retrieve the state.
	//
	//# \desc
	//# The $GetArrayStateElement$ function returns a reference to the state associated with a single subarray in a data structure.
	//# The legal values of the $index$ parameter range from zero to <i>n</i>&nbsp;/&nbsp;<i>s</i>&nbsp;&minus;1, inclusive,
	//# where <i>n</i> is the total count of data elements returned by the $@DataStructure::GetDataElementCount@$ function,
	//# and <i>s</i> is the size of each subarray returned by the $@PrimitiveStructure::GetArraySize@$ function. States are stored
	//# contiguously in memory.
	//#
	//# State data exists only if the $@PrimitiveStructure::GetStateFlag@$ function returns $true$, and there is one state value
	//# per subarray. The $GetArrayStateElement$ function should not be called if the $PrimitiveStructure::GetStateFlag$ function
	//# returns $false$.
	//
	//# \also	$@PrimitiveStructure::GetStateFlag@$
	//# \also	$@DataStructure::GetArrayDataElement@$



	template <class type>
	class DataStructure final : public PrimitiveStructure
	{
		private:

			typedef typename type::PrimType PrimType;

			Array<PrimType, 4>		dataArray;
			Array<uint32, 4>		stateArray;

		public:

			DataStructure();
			TERATHON_API DataStructure(uint32 size, bool state = false);
			TERATHON_API ~DataStructure();

			const ImmutableArray<PrimType>& GetDataArray(void) const
			{
				return (dataArray);
			}

			int32 GetDataElementCount(void) const
			{
				return (dataArray.GetArrayElementCount());
			}

			void SetDataElementCount(int32 count)
			{
				dataArray.SetArrayElementCount(count);
			}

			int32 GetArrayStateElementCount(void) const
			{
				return (stateArray.GetArrayElementCount());
			}

			void SetArrayStateElementCount(int32 count)
			{
				stateArray.SetArrayElementCount(count);
			}

			const PrimType& GetDataElement(int32 index) const
			{
				return (dataArray[index]);
			}

			void SetDataElement(int32 index, const PrimType& data)
			{
				dataArray[index] = data;
			}

			int32 GetArrayDataElementCount(void) const
			{
				return (dataArray.GetArrayElementCount() / GetArraySize());
			}

			const PrimType *GetArrayDataElement(int32 index) const
			{
				return (&dataArray[GetArraySize() * index]);
			}

			const uint32& GetArrayStateElement(int32 index) const
			{
				return (stateArray[index]);
			}

			void SetArrayStateElement(int32 index, uint32 state)
			{
				stateArray[index] = state;
			}

			void AppendDataElement(const PrimType& data)
			{
				dataArray.AppendArrayElement(data);
			}

			void AppendDataElement(PrimType&& data)
			{
				dataArray.AppendArrayElement(static_cast<PrimType&&>(data));
			}

			void AppendArrayStateElement(uint32 state)
			{
				stateArray.AppendArrayElement(state);
			}

			DataResult ParseData(const char *& text) override;
	};


	class RootStructure : public Structure
	{
		public:

			RootStructure();
			~RootStructure();

			bool ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const override;
	};


	//# \class	DataDescription		Represents a derivative file format based on the OpenDDL language.
	//
	//# The $DataDescription$ class represents a derivative file format based on the OpenDDL language.
	//
	//# \def	class DataDescription
	//
	//# \ctor	DataDescription();
	//
	//# \desc
	//# The $DataDescription$ class is the base class for objects that represent derivative file format based on
	//# the Open Data Description Language (OpenDDL). It serves as a container for the tree hierarchy of data structures
	//# in an OpenDDL file.
	//#
	//# A subclass of the $DataDescription$ class represents a specific OpenDDL-based file format and provides the means for
	//# constructing custom $@Structure@$ subclasses by overriding the $@DataDescription::CreateStructure@$ function.
	//
	//# \also	$@Structure@$
	//# \also	$@PrimitiveStructure@$
	//# \also	$@DataStructure@$
	//
	//# \wiki	Open_Data_Description_Language		Open Data Description Language


	//# \function	DataDescription::GetRootStructure		Returns root structure for an OpenDDL file.
	//
	//# \proto	const Structure *GetRootStructure(void) const;
	//
	//# \desc
	//# The $GetRootStructure$ function returns the root structure for an OpenDDL file. The direct subnodes
	//# of the root structure correspond to the top-level data structures in the file.
	//
	//# \also	$@DataDescription::FindStructure@$
	//# \also	$@Utilities/Tree@$


	//# \function	DataDescription::FindStructure		Finds a named structure.
	//
	//# \proto	Structure *FindStructure(const StructureRef& reference) const;
	//
	//# \param	reference	The reference to the structure to find.
	//
	//# \desc
	//# The $FindStructure$ function finds the structure referenced by the sequence of names stored in the
	//# $reference$ parameter and returns a pointer to it. If no such structure exists, then the return value is $nullptr$.
	//#
	//# The reference must be a global reference, meaning that the first name stored in the reference is a
	//# global name as indicated by a value of $true$ being returned by the $@StructureRef::GetGlobalRefFlag@$ function.
	//# If the reference is not a global reference, then this function always returns $nullptr$. The $@Structure::FindStructure@$
	//# function should be used to find a structure through a local reference.
	//#
	//# If the specified reference has an empty name array, then the return value is always $nullptr$. The empty name
	//# array is assigned to a reference data value when $null$ appears in the OpenDDL file.
	//
	//# \also	$@StructureRef@$
	//# \also	$@Structure::FindStructure@$
	//# \also	$@Structure::GetStructureName@$
	//# \also	$@Structure::GetGlobalNameFlag@$
	//# \also	$@DataDescription::GetRootStructure@$


	//# \function	DataDescription::CreateStructure		Creates a custom data structure.
	//
	//# \proto	virtual Structure *CreateStructure(const String<>& identifier) const;
	//
	//# \param	identifier		The identifier of a data structure in an OpenDDL file.
	//
	//# \desc
	//# The $CreateStructure$ function should be overridden by any subclass of the $DataDescription$ class
	//# representing a file format that defines custom data structures. The implementation should use the $new$
	//# operator to create a new object based on the $Structure$ subclass corresponding to the $identifier$
	//# parameter. If the identifier is not recognized, then this function should return $nullptr$. The default
	//# implementation always returns $nullptr$.
	//
	//# \also	$@Structure@$


	//# \function	DataDescription::ValidateTopLevelStructure		Determines the validity of a top-level structure.
	//
	//# \proto	virtual bool ValidateTopLevelStructure(const Structure *structure) const;
	//
	//# \param	structure		The top-level structure to validate.
	//
	//# \desc
	//# The $ValidateTopLevelStructure$ function is called each time a new structure is created at the top level
	//# of an OpenDDL file to determine whether the new structure can legally appear outside all other structures.
	//# An overriding implementation should examine the structure specified by the $structure$ parameter and return
	//# $true$ if it can legally appear at the top level of a file, and it should return $false$ otherwise.
	//#
	//# An implementation would typically call the $@Structure::GetStructureType@$ function to make its decision,
	//# but other information such as the base structure type or the primitive subarray size may also be taken into
	//# account. At the time that the $ValidateTopLevelStructure$ function is called, no data belonging to the structure
	//# is available, so the data itself cannot be used to validate any top-level structures.
	//#
	//# The default implementation of the $ValidateTopLevelStructure$ function always returns $true$.
	//
	//# \also	$@Structure::ValidateSubstructure@$


	//# \function	DataDescription::ProcessText		Parses an OpenDDL file and processes the top-level data structures.
	//
	//# \proto	DataResult ProcessText(const char *text);
	//
	//# \param	text	The full contents of an OpenDDL file with a terminating zero byte.
	//
	//# \desc
	//# The $ProcessText$ function parses the entire OpenDDL file specified by the $text$ parameter. If the file is
	//# successfully parsed, then the data is processed as described below. If an error occurs during the parsing stage,
	//# then the $ProcessText$ function returns one of the following values, and the $DataDescription$ object contains no data.
	//
	//# \table	DataResult
	//
	//# During the parsing stage, the $@DataDescription::CreateStructure@$ function is called for each custom data structure
	//# that is encountered in order to construct an object whose type is the proper subclass of the $@Structure@$ class.
	//
	//# After a successful parse, the $ProcessText$ function iterates through all of the top-level data structures in
	//# the file (which are the direct subnodes of the root structure returned by the $@DataDescription::GetRootStructure@$
	//# function) and calls the $@Structure::ProcessData@$ function for each one. If an error is returned by any of the
	//# calls to the $@Structure::ProcessData@$ function, then the processing stops, and the same error is returned by the
	//# $DataDescription::ProcessText$ function. If all of the top-level data structures are processed without error, then
	//# the $DataDescription::ProcessText$ function returns $kDataOkay$. The error returned during the processing stage can
	//# be one of the following values or a value defined by a derivative data format.
	//
	//# \table	DataProcessResult
	//
	//# If an error is returned for either the parsing stage or the processing stage, then the line number where the error
	//# occurred can be retrieved by calling the $@DataDescription::GetErrorLine@$ function.
	//#
	//# The default implementation of the $@Structure::ProcessData@$ function iterates over the direct subnodes of a
	//# data structure and calls the $ProcessData$ function for each one. If all overrides call the base class implementation,
	//# then the entire tree of data structures will be visited during the processing stage.
	//#
	//# Any implementation of the $@Structure::ProcessData@$ function may make the following assumptions about the data:
	//#
	//# 1. The input text is syntactically valid.<br/>
	//# 2. Each structure described in the input text was recognized and successfully created.<br/>
	//# 3. Each structure is valid as indicated by the $@Structure::ValidateSubstructure@$ function called for its enclosing structure.<br/>
	//# 4. Each property identifier is valid as indicated by the $@Structure::ValidateProperty@$ function called for the associated structure, and it has a value of the proper type assigned to it.<br/>
	//# 5. Any existing subarrays of primitive data have the correct number of elements, matching the number specified in brackets after the primitive type identifier.<br/>
	//# 6. Any existing state identifiers associated with primitive data subarrays are valid as indicated by the $@Structure::GetStateValue@$ function.
	//
	//# \also	$@Structure::ProcessData@$
	//# \also	$@DataDescription::GetErrorLine@$


	//# \function	DataDescription::GetErrorLine		Returns the line on which an error occurred.
	//
	//# \proto	int32 GetErrorLine(void) const;
	//
	//# \desc
	//# The $GetErrorLine$ function returns the line number on which an error occurred when the $@DataDescription::ProcessText@$
	//# function was called. Line numbering begins at one. If the $@DataDescription::ProcessText@$ function returned $kDataOkay$,
	//# then the $GetErrorLine$ function will return zero.
	//
	//# \also	$@DataDescription::ProcessText@$


	class DataDescription
	{
		friend Structure;

		private:

			Map<Structure>		structureMap;
			RootStructure		rootStructure;

			const Structure		*errorStructure;
			int32				errorLine;

			static Structure *CreatePrimitive(const String<>& identifier);

			DataResult ParseProperties(const char *& text, Structure *structure);
			DataResult ParseStructures(const char *& text, Structure *root);

		protected:

			TERATHON_API DataDescription();

		public:

			TERATHON_API virtual ~DataDescription();

			Structure *GetRootStructure(void)
			{
				return (&rootStructure);
			}

			const Structure *GetRootStructure(void) const
			{
				return (&rootStructure);
			}

			int32 GetErrorLine(void) const
			{
				return (errorLine);
			}

			TERATHON_API Structure *FindStructure(const StructureRef& reference) const;

			TERATHON_API virtual Structure *CreateStructure(const String<>& identifier) const;
			TERATHON_API virtual bool ValidateTopLevelStructure(const Structure *structure) const;

			TERATHON_API virtual DataResult ProcessData(void);
			TERATHON_API DataResult ProcessText(const char *text);
	};
}


#endif
