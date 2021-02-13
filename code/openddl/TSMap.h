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


#ifndef TSMap_h
#define TSMap_h


//# \component	Utility Library
//# \prefix		Utilities/


#include "TSPlatform.h"


#define TERATHON_MAP 1


namespace Terathon
{
	class MapBase;
	class MapElementBase;

	template <class>
	class Map;


	struct MapReservation
	{
		MapElementBase		*superElement;
		int32				direction;
	};


	class MapElementBase
	{
		friend class MapBase;

		private:

			MapElementBase		*superNode;
			MapElementBase		*leftSubnode;
			MapElementBase		*rightSubnode;

			MapBase				*owningMap;
			int32				balance;

			MapElementBase(const MapElementBase&) = delete;
			MapElementBase& operator =(const MapElementBase&) = delete;

			TERATHON_API MapElementBase *GetFirstMapElement(void);
			TERATHON_API MapElementBase *GetLastMapElement(void);

			void RemoveSubtree(void);
			void DeleteSubtree(void);

		protected:

			MapElementBase()
			{
				superNode = nullptr;
				leftSubnode = nullptr;
				rightSubnode = nullptr;
				owningMap = nullptr;
			}

			TERATHON_API virtual ~MapElementBase();

			MapElementBase *GetLeftSubnode(void) const
			{
				return (leftSubnode);
			}

			MapElementBase *GetRightSubnode(void) const
			{
				return (rightSubnode);
			}

			MapBase *GetOwningMap(void) const
			{
				return (owningMap);
			}

			TERATHON_API MapElementBase *GetPreviousMapElement(void) const;
			TERATHON_API MapElementBase *GetNextMapElement(void) const;

		public:

			TERATHON_API virtual void Detach(void);
	};


	class MapBase
	{
		friend class MapElementBase;

		private:

			MapElementBase		*rootElement;

			MapBase(const MapBase&) = delete;
			MapBase& operator =(const MapBase&) = delete;

			MapElementBase *RotateLeft(MapElementBase *node);
			MapElementBase *RotateRight(MapElementBase *node);
			MapElementBase *ZigZagLeft(MapElementBase *node);
			MapElementBase *ZigZagRight(MapElementBase *node);

			void RemoveBranchNode(MapElementBase *node, MapElementBase *subnode);

		protected:

			MapBase()
			{
				rootElement = nullptr;
			}

			TERATHON_API ~MapBase();

			TERATHON_API MapElementBase *operator [](machine index) const;

			MapElementBase *GetRootMapElement(void) const
			{
				return (rootElement);
			}

			MapElementBase *GetFirstMapElement(void) const
			{
				return ((rootElement) ? rootElement->GetFirstMapElement() : nullptr);
			}

			MapElementBase *GetLastMapElement(void) const
			{
				return ((rootElement) ? rootElement->GetLastMapElement() : nullptr);
			}

			bool Member(const MapElementBase *element) const
			{
				return (element->owningMap == this);
			}

			TERATHON_API void SetRootElement(MapElementBase *node);

			TERATHON_API void InsertLeftSubnode(MapElementBase *node, MapElementBase *subnode);
			TERATHON_API void InsertRightSubnode(MapElementBase *node, MapElementBase *subnode);

			TERATHON_API void ReplaceMapElement(MapElementBase *element, MapElementBase *replacement);
			TERATHON_API void RemoveMapElement(MapElementBase *element);

		public:

			bool Empty(void) const
			{
				return (!rootElement);
			}

			TERATHON_API int32 GetMapElementCount(void) const;

			TERATHON_API void RemoveAllMapElements(void);
			TERATHON_API void PurgeMap(void);
	};


	//# \class	MapElement		The base class for objects that can be stored in a map.
	//
	//# Objects inherit from the $MapElement$ class so that they can be stored in a map.
	//
	//# \def	template <class type> class MapElement : public MapElementBase
	//
	//# \tparam		type	The type of the class that can be stored in a map. This parameter should be the
	//#						type of the class that inherits directly from the $MapElement$ class.
	//
	//# \ctor	MapElement();
	//
	//# \desc
	//# The $MapElement$ class should be declared as a base class for objects that need to be stored in a map.
	//# The $type$ template parameter should match the class type of such objects, and these objects can be
	//# stored in a $@Map@$ container declared with the same $type$ template parameter.
	//
	//# \privbase	MapElementBase		Used internally to encapsulate common functionality that is independent
	//#									of the template parameters.
	//
	//# \also	$@Map@$


	//# \function	MapElement::GetPreviousMapElement		Returns the previous element in a map.
	//
	//# \proto	type *GetPreviousMapElement(void) const;
	//
	//# \desc
	//# The $GetPreviousMapElement$ function returns a pointer to the element immediately preceding an object
	//# in its owning map. If the object is the first element in a map, or the object does not belong to a
	//# map, then the return value is $nullptr$.
	//
	//# \also	$@MapElement::GetNextMapElement@$


	//# \function	MapElement::GetNextMapElement		Returns the next element in a map.
	//
	//# \proto	type *GetNextMapElement(void) const;
	//
	//# \desc
	//# The $GetNextMapElement$ function returns a pointer to the element immediately succeeding an object
	//# in its owning map. If the object is the last element in a map, or the object does not belong to a
	//# map, then the return value is $nullptr$.
	//
	//# \also	$@MapElement::GetPreviousMapElement@$


	//# \function	MapElement::GetOwningMap		Returns the map to which an object belongs.
	//
	//# \proto	Map<type> *GetOwningMap(void) const;
	//
	//# \desc
	//# The $GetOwningMap$ function returns a pointer to the $@Map@$ container to which an object belongs.
	//# If the object is not a member of a map, then the return value is $nullptr$.
	//
	//# \also	$@Map::Member@$


	//# \function	MapElement::Detach	Removes an object from any map to which it belongs.
	//
	//# \proto	virtual void Detach(void);
	//
	//# \desc
	//# The $Detach$ function removes an object from its owning map. If the object is not a member of
	//# a map, then the $Detach$ function has no effect.
	//
	//# \also	$@Map::RemoveMapElement@$


	template <class type>
	class MapElement : public MapElementBase
	{
		public:

			inline MapElement() = default;

			type *GetLeftSubnode(void) const
			{
				return (static_cast<type *>(static_cast<MapElement<type> *>(MapElementBase::GetLeftSubnode())));
			}

			type *GetRightSubnode(void) const
			{
				return (static_cast<type *>(static_cast<MapElement<type> *>(MapElementBase::GetRightSubnode())));
			}

			type *GetPreviousMapElement(void) const
			{
				return (static_cast<type *>(static_cast<MapElement<type> *>(MapElementBase::GetPreviousMapElement())));
			}

			type *GetNextMapElement(void) const
			{
				return (static_cast<type *>(static_cast<MapElement<type> *>(MapElementBase::GetNextMapElement())));
			}

			Map<type> *GetOwningMap(void) const
			{
				return (static_cast<Map<type> *>(MapElementBase::GetOwningMap()));
			}
	};


	template <class type>
	class MapIterator
	{
		private:

			type		*iteratorElement;

		public:

			MapIterator(type *element) : iteratorElement(element) {}

			type *operator *(void) const
			{
				return (iteratorElement);
			}

			MapIterator& operator ++(void)
			{
				iteratorElement = iteratorElement->MapElement<type>::GetNextMapElement();
				return (*this);
			}

			bool operator ==(const MapIterator& iterator) const
			{
				return (iteratorElement == iterator.iteratorElement);
			}

			bool operator !=(const MapIterator& iterator) const
			{
				return (iteratorElement != iterator.iteratorElement);
			}
	};


	//# \class	Map		An associative container class that holds a set of objects.
	//
	//# The $Map$ class encapsulates an associative key-value map.
	//
	//# \def	template <class type> class Map : public MapBase
	//
	//# \tparam		type	The type of the class that can be stored in the map. The class specified
	//#						by this parameter should inherit directly from the $@MapElement@$ class
	//#						using the same template parameter.
	//
	//# \ctor	Map();
	//
	//# \desc
	//# The $Map$ class template is a container used to store an associative key-value map of objects.
	//# The class type of objects that are to be stored in the map must be a subclass of the $@MapElement@$
	//# class template using the same template parameters as the $Map$ container.
	//#
	//# Upon construction, a $Map$ object is empty. When a $Map$ object is destroyed, all of the members
	//# of the map are also destroyed. To avoid deleting the members of a map when a $Map$ object is
	//# destroyed, first call the $@Map::RemoveAllMapElements@$ function to remove all of the map's members.
	//#
	//# The class specified by the $type$ template parameter must define a type named $KeyType$ and a
	//# function named $GetKey$ that has one of the following two prototypes.
	//
	//# \source
	//# KeyType GetKey(void) const;
	//# const KeyType& GetKey(void) const;
	//
	//# \desc
	//# This function should return the key associated with the object for which it is called. The $KeyType$
	//# type must be capable of being compared to other key values using the $<$ and $>$ operators.
	//#
	//# It is possible to iterate over the elements of a map using a range-based for loop.
	//# This is illustrated by the following code, where $map$ is a variable of type $Map<type>$.
	//
	//# \source
	//# for (type *element : map)\n
	//# {\n
	//#	\t...\n
	//# }
	//
	//# \operator	type *operator [](machine index) const;
	//#				Returns the element of a map whose zero-based index is $index$. If $index$ is
	//#				greater than or equal to the number of elements in the map, then the return
	//#				value is $nullptr$.
	//
	//# \privbase	MapBase		Used internally to encapsulate common functionality that is independent
	//#							of the template parameters.
	//
	//# \also	$@MapElement@$


	//# \function	Map::GetFirstMapElement		Returns the first element in a map.
	//
	//# \proto	type *GetFirstMapElement(void) const;
	//
	//# \desc
	//# The $GetFirstMapElement$ function returns a pointer to the first element in a map. If the map is empty,
	//# then this function returns $nullptr$.
	//#
	//# The first element in a map is always the one having the least key value based on the
	//# ordering defined for the $KeyType$ type. The $@MapElement::GetNextMapElement@$ function can
	//# be repeatedly called to iterate through the members of a map in order.
	//#
	//# \note
	//# The running time for the $First$ function is <i>O</i>(log&#x202F;<i>n</i>), where <i>n</i> is
	//# the number of objects stored in the map.
	//
	//# \also	$@Map::GetLastMapElement@$


	//# \function	Map::GetLastMapElement		Returns the last element in a map.
	//
	//# \proto	type *GetLastMapElement(void) const;
	//
	//# \desc
	//# The $GetLastMapElement$ function returns a pointer to the last element in a map. If the map is empty,
	//# then this function returns $nullptr$.
	//#
	//# The last element in a map is always the one having the greatest key value based on the
	//# ordering defined for the $KeyType$ type. The $@MapElement::GetPreviousMapElement@$ function can
	//# be repeatedly called to iterate through the members of a map in reverse order.
	//#
	//# \note
	//# The running time for the $Last$ function is <i>O</i>(log&#x202F;<i>n</i>), where <i>n</i> is
	//# the number of objects stored in the map.
	//
	//# \also	$@Map::GetFirstMapElement@$


	//# \function	Map::Member		Returns a boolean value indicating whether a particular object is
	//#								a member of a map.
	//
	//# \proto	bool Member(const MapElement<type> *element) const;
	//
	//# \param	element		A pointer to the object to test for membership.
	//
	//# \desc
	//# The $Member$ function returns $true$ if the object specified by the $element$ parameter is
	//# a member of the map, and $false$ otherwise.
	//
	//# \also	$@MapElement::GetOwningMap@$


	//# \function	Map::Empty		Returns a boolean value indicating whether a map is empty.
	//
	//# \proto	bool Empty(void) const;
	//
	//# \desc
	//# The $Empty$ function returns $true$ if the map contains no elements, and $false$ otherwise.
	//
	//# \also	$@Map::GetFirstMapElement@$
	//# \also	$@Map::GetLastMapElement@$


	//# \function	Map::GetMapElementCount		Returns the number of elements in a map.
	//
	//# \proto	int32 GetMapElementCount(void) const;
	//
	//# \desc
	//# The $GetMapElementCount$ function iterates through the members of a map and returns the count.
	//
	//# \also	$@Map::Empty@$


	//# \function	Map::InsertMapElement		Adds an object to a map.
	//
	//# \proto	bool InsertMapElement(type *element);
	//
	//# \param	element		A pointer to the object to add to the map.
	//
	//# \desc
	//# The $InsertMapElement$ function adds the object specified by the $element$ parameter to a map.
	//# If the object is a member of a different map of the same type, then it is first removed from
	//# that map before being added to the new map.
	//#
	//# Only one object having a particular key value may be stored in a map at one time. If the key
	//# value associated with the $element$ parameter is not found in the map, then the object is
	//# inserted, and $true$ is returned. If a different object with the same key value is already
	//# in the map, then the new object is not inserted in the map and $false$ is returned.
	//
	//# \also	$@Map::InsertReplaceMapElement@$
	//# \also	$@Map::FindMapElement@$


	//# \function	Map::InsertReplaceMapElement		Adds an object to a map and replaces an existing object having the same key.
	//
	//# \proto	type *InsertReplaceMapElement(type *element);
	//
	//# \param	element		A pointer to the object to add to the map.
	//
	//# \desc
	//# The $InsertReplaceMapElement$ function adds the object specified by the $element$ parameter to a map.
	//# If an object having the same key is already a member of the map, then it is replaced by the new
	//# object being inserted. The old object is removed from the map, and a pointer to it is returned.
	//# If no object is replaced, then the return value is $nullptr$.
	//#
	//# If the object being inserted is a member of a different map of the same type, then it is first removed
	//# from that map before being added to the new map. If the object is already a member of the map for which
	//# the $InsertReplaceMapElement$ function is called, then no action is taken, and the return value is $nullptr$.
	//
	//# \also	$@Map::InsertMapElement@$
	//# \also	$@Map::FindMapElement@$


	//# \function	Map::RemoveMapElement		Removes a particular element from a map.
	//
	//# \proto	void RemoveMapElement(MapElement<type> *element);
	//
	//# \param	element		A pointer to the object to remove from the map.
	//
	//# \desc
	//# The $RemoveMapElement$ function removes the object specified by the $element$ parameter from a map.
	//# The object must belong to map for which the $RemoveMapElement$ function is called, or the internal
	//# links will become corrupted.
	//
	//# \also	$@Map::RemoveAllMapElements@$
	//# \also	$@Map::PurgeMap@$
	//# \also	$@MapElement::Detach@$


	//# \function	Map::RemoveAllMapElements		Removes all elements from a map.
	//
	//# \proto	void RemoveAllMapElements(void);
	//
	//# \desc
	//# The $RemoveAllMapElements$ function removes all objects contained in a map, but does not delete them.
	//# The map is subsequently empty.
	//
	//# \also	$@Map::RemoveMapElement@$
	//# \also	$@Map::PurgeMap@$
	//# \also	$@MapElement::Detach@$


	//# \function	Map::PurgeMap		Deletes all elements in a map.
	//
	//# \proto	void PurgeMap(void);
	//
	//# \desc
	//# The $PurgeMap$ function deletes all objects contained in a map. The map is subsequently empty.
	//# To remove all elements of a map without destroying them, use the $@Map::RemoveAllMapElements@$ function.
	//
	//# \also	$@Map::RemoveMapElement@$
	//# \also	$@Map::RemoveAllMapElements@$
	//# \also	$@MapElement::Detach@$


	//# \function	Map::FindMapElement		Finds an object in a map.
	//
	//# \proto	type *FindMapElement(const KeyType& key) const;
	//
	//# \desc
	//# The $FindMapElement$ function searches a map for an object having the key value given by the $key$ parameter.
	//# If a matching object is found, then a pointer to it is returned. If no matching object is found, then
	//# the return value is $nullptr$. This function is guaranteed to run in <i>O</i>(log&#x202F;<i>n</i>) time,
	//# where <i>n</i> is the number of objects stored in the map.
	//
	//# \also	$@Map::InsertMapElement@$
	//# \also	$@Map::InsertReplaceMapElement@$
	//# \also	$@Map::RemoveMapElement@$


	template <class type>
	class Map : public MapBase
	{
		public:

			typedef typename type::KeyType		KeyType;

			inline Map() = default;

			type *operator [](machine index) const
			{
				return (static_cast<type *>(static_cast<MapElement<type> *>(MapBase::operator [](index))));
			}

			type *GetFirstMapElement(void) const
			{
				return (static_cast<type *>(static_cast<MapElement<type> *>(MapBase::GetFirstMapElement())));
			}

			type *GetLastMapElement(void) const
			{
				return (static_cast<type *>(static_cast<MapElement<type> *>(MapBase::GetLastMapElement())));
			}

			MapIterator<type> begin(void) const
			{
				return (MapIterator<type>(static_cast<type *>(static_cast<MapElement<type> *>(MapBase::GetFirstMapElement()))));
			}

			MapIterator<type> end(void) const
			{
				return (MapIterator<type>(nullptr));
			}

			type *GetRootMapElement(void) const
			{
				return (static_cast<type *>(static_cast<MapElement<type> *>(MapBase::GetRootMapElement())));
			}

			bool Member(const MapElement<type> *element) const
			{
				return (MapBase::Member(element));
			}

			void RemoveMapElement(MapElement<type> *element)
			{
				MapBase::RemoveMapElement(element);
			}

			bool InsertMapElement(MapElement<type> *element);
			type *InsertReplaceMapElement(MapElement<type> *element);

			void InsertMapElement(MapElement<type> *element, const MapReservation *reservation);
			bool ReserveMapElement(const KeyType& key, MapReservation *reservation);

			type *FindMapElement(const KeyType& key) const;
	};


	template <class type>
	bool Map<type>::InsertMapElement(MapElement<type> *element)
	{
		MapElement<type> *node = GetRootMapElement();
		if (node)
		{
			const KeyType& key = static_cast<type *>(element)->GetKey();
			for (;;)
			{
				const KeyType& nodeKey = static_cast<type *>(node)->GetKey();
				if (key < nodeKey)
				{
					MapElement<type> *subnode = node->GetLeftSubnode();
					if (!subnode)
					{
						InsertLeftSubnode(node, element);
						break;
					}

					node = subnode;
				}
				else if (nodeKey < key)
				{
					MapElement<type> *subnode = node->GetRightSubnode();
					if (!subnode)
					{
						InsertRightSubnode(node, element);
						break;
					}

					node = subnode;
				}
				else
				{
					return (false);
				}
			}
		}
		else
		{
			SetRootElement(element);
		}

		return (true);
	}

	template <class type>
	type *Map<type>::InsertReplaceMapElement(MapElement<type> *element)
	{
		MapElement<type> *node = GetRootMapElement();
		if (node)
		{
			const KeyType& key = static_cast<type *>(element)->GetKey();
			for (;;)
			{
				const KeyType& nodeKey = static_cast<type *>(node)->GetKey();
				if (key < nodeKey)
				{
					MapElement<type> *subnode = node->GetLeftSubnode();
					if (!subnode)
					{
						InsertLeftSubnode(node, element);
						break;
					}

					node = subnode;
				}
				else if (nodeKey < key)
				{
					MapElement<type> *subnode = node->GetRightSubnode();
					if (!subnode)
					{
						InsertRightSubnode(node, element);
						break;
					}

					node = subnode;
				}
				else
				{
					if (element != node)
					{
						ReplaceMapElement(node, element);
						return (static_cast<type *>(node));
					}

					break;
				}
			}
		}
		else
		{
			SetRootElement(element);
		}

		return (nullptr);
	}

	template <class type>
	void Map<type>::InsertMapElement(MapElement<type> *element, const MapReservation *reservation)
	{
		MapElementBase *super = reservation->superElement;
		if (super)
		{
			if (reservation->direction < 0)
			{
				InsertLeftSubnode(super, element);
			}
			else
			{
				InsertRightSubnode(super, element);
			}
		}
		else
		{
			SetRootElement(element);
		}
	}

	template <class type>
	bool Map<type>::ReserveMapElement(const KeyType& key, MapReservation *reservation)
	{
		MapElement<type> *node = GetRootMapElement();
		if (node)
		{
			for (;;)
			{
				const KeyType& nodeKey = static_cast<type *>(node)->GetKey();
				if (key < nodeKey)
				{
					MapElement<type> *subnode = node->GetLeftSubnode();
					if (!subnode)
					{
						reservation->superElement = node;
						reservation->direction = -1;
						break;
					}

					node = subnode;
				}
				else if (nodeKey < key)
				{
					MapElement<type> *subnode = node->GetRightSubnode();
					if (!subnode)
					{
						reservation->superElement = node;
						reservation->direction = 1;
						break;
					}

					node = subnode;
				}
				else
				{
					return (false);
				}
			}
		}
		else
		{
			reservation->superElement = nullptr;
		}

		return (true);
	}

	template <class type>
	type *Map<type>::FindMapElement(const KeyType& key) const
	{
		MapElement<type> *node = GetRootMapElement();
		while (node)
		{
			const KeyType& nodeKey = static_cast<type *>(node)->GetKey();
			if (key < nodeKey)
			{
				node = node->GetLeftSubnode();
			}
			else if (nodeKey < key)
			{
				node = node->GetRightSubnode();
			}
			else
			{
				break;
			}
		}

		return (static_cast<type *>(node));
	}
}


#endif
