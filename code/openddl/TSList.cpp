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


#include "TSList.h"


using namespace Terathon;


ListElementBase::~ListElementBase()
{
	if (owningList)
	{
		owningList->RemoveListElement(this);
	}
}

int32 ListElementBase::GetListIndex(void) const
{
	machine index = 0;

	const ListElementBase *element = this;
	for (;;)
	{
		element = element->GetPreviousListElement();
		if (!element)
		{
			break;
		}

		index++;
	}

	return (int32(index));
}

void ListElementBase::Detach(void)
{
	if (owningList)
	{
		owningList->RemoveListElement(this);
	}
}


ListBase::~ListBase()
{
	PurgeList();
}

ListElementBase *ListBase::operator [](machine index) const
{
	machine i = 0;
	ListElementBase *element = firstListElement;
	while (element)
	{
		if (i == index)
		{
			return (element);
		}

		i++;
		element = element->nextListElement;
	}

	return (nullptr);
}

int32 ListBase::GetListElementCount(void) const
{
	machine count = 0;
	const ListElementBase *element = firstListElement;
	while (element)
	{
		count++;
		element = element->nextListElement;
	}

	return (int32(count));
}

void ListBase::PrependListElement(ListElementBase *element)
{
	ListBase *list = element->owningList;
	if (list)
	{
		ListElementBase *prev = element->prevListElement;
		ListElementBase *next = element->nextListElement;

		if (prev)
		{
			prev->nextListElement = next;
			element->prevListElement = nullptr;
		}

		if (next)
		{
			next->prevListElement = prev;
			element->nextListElement = nullptr;
		}

		if (list->firstListElement == element)
		{
			list->firstListElement = next;
		}

		if (list->lastListElement == element)
		{
			list->lastListElement = prev;
		}
	}

	if (firstListElement)
	{
		firstListElement->prevListElement = element;
		element->nextListElement = firstListElement;
		firstListElement = element;
	}
	else
	{
		firstListElement = element;
		lastListElement = element;
	}

	element->owningList = this;
}

void ListBase::AppendListElement(ListElementBase *element)
{
	ListBase *list = element->owningList;
	if (list)
	{
		ListElementBase *prev = element->prevListElement;
		ListElementBase *next = element->nextListElement;

		if (prev)
		{
			prev->nextListElement = next;
			element->prevListElement = nullptr;
		}

		if (next)
		{
			next->prevListElement = prev;
			element->nextListElement = nullptr;
		}

		if (list->firstListElement == element)
		{
			list->firstListElement = next;
		}

		if (list->lastListElement == element)
		{
			list->lastListElement = prev;
		}
	}

	element->owningList = this;

	if (lastListElement)
	{
		lastListElement->nextListElement = element;
		element->prevListElement = lastListElement;
		lastListElement = element;
	}
	else
	{
		firstListElement = element;
		lastListElement = element;
	}
}

void ListBase::InsertListElementBefore(ListElementBase *element, ListElementBase *before)
{
	ListBase *list = element->owningList;
	if (list)
	{
		ListElementBase *prev = element->prevListElement;
		ListElementBase *next = element->nextListElement;

		if (prev)
		{
			prev->nextListElement = next;
		}

		if (next)
		{
			next->prevListElement = prev;
		}

		if (list->firstListElement == element)
		{
			list->firstListElement = next;
		}

		if (list->lastListElement == element)
		{
			list->lastListElement = prev;
		}
	}

	element->owningList = this;
	element->nextListElement = before;

	if (before)
	{
		ListElementBase *after = before->prevListElement;
		element->prevListElement = after;
		before->prevListElement = element;

		if (after)
		{
			after->nextListElement = element;
		}
		else
		{
			firstListElement = element;
		}
	}
	else
	{
		ListElementBase *after = lastListElement;
		element->prevListElement = after;

		if (after)
		{
			after->nextListElement = element;
			lastListElement = element;
		}
		else
		{
			firstListElement = element;
			lastListElement = element;
		}
	}
}

void ListBase::InsertListElementAfter(ListElementBase *element, ListElementBase *after)
{
	ListBase *list = element->owningList;
	if (list)
	{
		ListElementBase *prev = element->prevListElement;
		ListElementBase *next = element->nextListElement;

		if (prev)
		{
			prev->nextListElement = next;
		}

		if (next)
		{
			next->prevListElement = prev;
		}

		if (list->firstListElement == element)
		{
			list->firstListElement = next;
		}

		if (list->lastListElement == element)
		{
			list->lastListElement = prev;
		}
	}

	element->owningList = this;
	element->prevListElement = after;

	if (after)
	{
		ListElementBase *before = after->nextListElement;
		element->nextListElement = before;
		after->nextListElement = element;

		if (before)
		{
			before->prevListElement = element;
		}
		else
		{
			lastListElement = element;
		}
	}
	else
	{
		ListElementBase *before = firstListElement;
		element->nextListElement = before;

		if (before)
		{
			before->prevListElement = element;
			firstListElement = element;
		}
		else
		{
			firstListElement = element;
			lastListElement = element;
		}
	}
}

void ListBase::RemoveListElement(ListElementBase *element)
{
	ListElementBase *prev = element->prevListElement;
	ListElementBase *next = element->nextListElement;

	if (prev)
	{
		prev->nextListElement = next;
	}

	if (next)
	{
		next->prevListElement = prev;
	}

	if (firstListElement == element)
	{
		firstListElement = next;
	}

	if (lastListElement == element)
	{
		lastListElement = prev;
	}

	element->prevListElement = nullptr;
	element->nextListElement = nullptr;
	element->owningList = nullptr;
}

void ListBase::RemoveAllListElements(void)
{
	ListElementBase *element = firstListElement;
	while (element)
	{
		ListElementBase *next = element->nextListElement;
		element->prevListElement = nullptr;
		element->nextListElement = nullptr;
		element->owningList = nullptr;
		element = next;
	}

	firstListElement = nullptr;
	lastListElement = nullptr;
}

void ListBase::PurgeList(void)
{
	while (firstListElement)
	{
		delete firstListElement;
	}
}
