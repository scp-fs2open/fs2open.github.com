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


#include "TSMap.h"
#include "TSBasic.h"


using namespace Terathon;


MapElementBase::~MapElementBase()
{
	if (owningMap)
	{
		owningMap->RemoveMapElement(this);
	}
}

void MapElementBase::Detach(void)
{
	if (owningMap)
	{
		owningMap->RemoveMapElement(this);
	}
}

MapElementBase *MapElementBase::GetFirstMapElement(void)
{
	MapElementBase *element = this;
	for (;;)
	{
		MapElementBase *left = element->leftSubnode;
		if (!left)
		{
			break;
		}

		element = left;
	}

	return (element);
}

MapElementBase *MapElementBase::GetLastMapElement(void)
{
	MapElementBase *element = this;
	for (;;)
	{
		MapElementBase *right = element->rightSubnode;
		if (!right)
		{
			break;
		}

		element = right;
	}

	return (element);
}

MapElementBase *MapElementBase::GetPreviousMapElement(void) const
{
	if (leftSubnode)
	{
		return (leftSubnode->GetLastMapElement());
	}

	const MapElementBase *element = this;
	for (;;)
	{
		MapElementBase *super = element->superNode;
		if (!super)
		{
			break;
		}

		if (super->rightSubnode == element)
		{
			return (super);
		}

		element = super;
	}

	return (nullptr);
}

MapElementBase *MapElementBase::GetNextMapElement(void) const
{
	if (rightSubnode)
	{
		return (rightSubnode->GetFirstMapElement());
	}

	const MapElementBase *element = this;
	for (;;)
	{
		MapElementBase *super = element->superNode;
		if (!super)
		{
			break;
		}

		if (super->leftSubnode == element)
		{
			return (super);
		}

		element = super;
	}

	return (nullptr);
}

void MapElementBase::RemoveSubtree(void)
{
	if (leftSubnode)
	{
		leftSubnode->RemoveSubtree();
	}

	if (rightSubnode)
	{
		rightSubnode->RemoveSubtree();
	}

	superNode = nullptr;
	leftSubnode = nullptr;
	rightSubnode = nullptr;
	owningMap = nullptr;
}

void MapElementBase::DeleteSubtree(void)
{
	if (leftSubnode)
	{
		leftSubnode->DeleteSubtree();
	}

	if (rightSubnode)
	{
		rightSubnode->DeleteSubtree();
	}

	owningMap = nullptr;
	delete this;
}


MapBase::~MapBase()
{
	PurgeMap();
}

MapElementBase *MapBase::operator [](machine index) const
{
	machine i = 0;
	MapElementBase *element = GetFirstMapElement();
	while (element)
	{
		if (i == index)
		{
			return (element);
		}

		i++;
		element = element->GetNextMapElement();
	}

	return (nullptr);
}

int32 MapBase::GetMapElementCount(void) const
{
	machine count = 0;
	const MapElementBase *element = GetFirstMapElement();
	while (element)
	{
		count++;
		element = element->GetNextMapElement();
	}

	return (int32(count));
}

MapElementBase *MapBase::RotateLeft(MapElementBase *node)
{
	MapElementBase *right = node->rightSubnode;

	if (node != rootElement)
	{
		MapElementBase *super = node->superNode;

		if (super->leftSubnode == node)
		{
			super->leftSubnode = right;
		}
		else
		{
			super->rightSubnode = right;
		}

		right->superNode = super;
	}
	else
	{
		rootElement = right;
		right->superNode = nullptr;
	}

	MapElementBase *subnode = right->leftSubnode;
	if (subnode)
	{
		subnode->superNode = node;
	}

	node->rightSubnode = subnode;

	right->leftSubnode = node;
	node->superNode = right;
	node->balance = -(--right->balance);

	return (right);
}

MapElementBase *MapBase::RotateRight(MapElementBase *node)
{
	MapElementBase *left = node->leftSubnode;

	if (node != rootElement)
	{
		MapElementBase *super = node->superNode;

		if (super->leftSubnode == node)
		{
			super->leftSubnode = left;
		}
		else
		{
			super->rightSubnode = left;
		}

		left->superNode = super;
	}
	else
	{
		rootElement = left;
		left->superNode = nullptr;
	}

	MapElementBase *subnode = left->rightSubnode;
	if (subnode)
	{
		subnode->superNode = node;
	}

	node->leftSubnode = subnode;

	left->rightSubnode = node;
	node->superNode = left;
	node->balance = -(++left->balance);

	return (left);
}

MapElementBase *MapBase::ZigZagLeft(MapElementBase *node)
{
	MapElementBase *right = node->rightSubnode;
	MapElementBase *top = right->leftSubnode;

	if (node != rootElement)
	{
		MapElementBase *super = node->superNode;

		if (super->leftSubnode == node)
		{
			super->leftSubnode = top;
		}
		else
		{
			super->rightSubnode = top;
		}

		top->superNode = super;
	}
	else
	{
		rootElement = top;
		top->superNode = nullptr;
	}

	MapElementBase *subLeft = top->leftSubnode;
	if (subLeft)
	{
		subLeft->superNode = node;
	}

	node->rightSubnode = subLeft;

	MapElementBase *subRight = top->rightSubnode;
	if (subRight)
	{
		subRight->superNode = right;
	}

	right->leftSubnode = subRight;

	top->leftSubnode = node;
	top->rightSubnode = right;
	node->superNode = top;
	right->superNode = top;

	int32 b = top->balance;
	node->balance = -MaxZero(b);
	right->balance = -MinZero(b);
	top->balance = 0;

	return (top);
}

MapElementBase *MapBase::ZigZagRight(MapElementBase *node)
{
	MapElementBase *left = node->leftSubnode;
	MapElementBase *top = left->rightSubnode;

	if (node != rootElement)
	{
		MapElementBase *super = node->superNode;

		if (super->leftSubnode == node)
		{
			super->leftSubnode = top;
		}
		else
		{
			super->rightSubnode = top;
		}

		top->superNode = super;
	}
	else
	{
		rootElement = top;
		top->superNode = nullptr;
	}

	MapElementBase *subLeft = top->leftSubnode;
	if (subLeft)
	{
		subLeft->superNode = left;
	}

	left->rightSubnode = subLeft;

	MapElementBase *subRight = top->rightSubnode;
	if (subRight)
	{
		subRight->superNode = node;
	}

	node->leftSubnode = subRight;

	top->leftSubnode = left;
	top->rightSubnode = node;
	node->superNode = top;
	left->superNode = top;

	int32 b = top->balance;
	node->balance = -MinZero(b);
	left->balance = -MaxZero(b);
	top->balance = 0;

	return (top);
}

void MapBase::SetRootElement(MapElementBase *node)
{
	MapBase *map = node->owningMap;
	if (map)
	{
		map->RemoveMapElement(node);
	}

	node->owningMap = this;
	node->balance = 0;

	rootElement = node;
}

void MapBase::InsertLeftSubnode(MapElementBase *node, MapElementBase *subnode)
{
	MapBase *map = subnode->owningMap;
	if (map)
	{
		map->RemoveMapElement(subnode);
	}

	node->leftSubnode = subnode;
	subnode->superNode = node;
	subnode->owningMap = this;
	subnode->balance = 0;

	int32 b = node->balance - 1;
	node->balance = b;
	if (b != 0)
	{
		int32 dir1 = -1;
		for (;;)
		{
			int32	dir2;

			MapElementBase *super = node->superNode;
			if (!super)
			{
				break;
			}

			b = super->balance;
			if (super->leftSubnode == node)
			{
				super->balance = --b;
				dir2 = -1;
			}
			else
			{
				super->balance = ++b;
				dir2 = 1;
			}

			if (b == 0)
			{
				break;
			}

			if (Abs(b) == 2)
			{
				if (dir2 == -1)
				{
					if (dir1 == -1)
					{
						RotateRight(super);
					}
					else
					{
						ZigZagRight(super);
					}
				}
				else
				{
					if (dir1 == 1)
					{
						RotateLeft(super);
					}
					else
					{
						ZigZagLeft(super);
					}
				}

				break;
			}

			dir1 = dir2;
			node = super;
		}
	}
}

void MapBase::InsertRightSubnode(MapElementBase *node, MapElementBase *subnode)
{
	MapBase *map = subnode->owningMap;
	if (map)
	{
		map->RemoveMapElement(subnode);
	}

	node->rightSubnode = subnode;
	subnode->superNode = node;
	subnode->owningMap = this;
	subnode->balance = 0;

	int32 b = node->balance + 1;
	node->balance = b;
	if (b != 0)
	{
		int32 dir1 = 1;
		for (;;)
		{
			int32	dir2;

			MapElementBase *super = node->superNode;
			if (!super)
			{
				break;
			}

			b = super->balance;
			if (super->leftSubnode == node)
			{
				super->balance = --b;
				dir2 = -1;
			}
			else
			{
				super->balance = ++b;
				dir2 = 1;
			}

			if (b == 0)
			{
				break;
			}

			if (Abs(b) == 2)
			{
				if (dir2 == -1)
				{
					if (dir1 == -1)
					{
						RotateRight(super);
					}
					else
					{
						ZigZagRight(super);
					}
				}
				else
				{
					if (dir1 == 1)
					{
						RotateLeft(super);
					}
					else
					{
						ZigZagLeft(super);
					}
				}

				break;
			}

			dir1 = dir2;
			node = super;
		}
	}
}

void MapBase::ReplaceMapElement(MapElementBase *element, MapElementBase *replacement)
{
	MapBase *map = replacement->owningMap;
	if (map)
	{
		map->RemoveMapElement(replacement);
	}

	MapElementBase *super = element->superNode;
	if (super)
	{
		if (super->leftSubnode == element)
		{
			super->leftSubnode = replacement;
		}
		else
		{
			super->rightSubnode = replacement;
		}
	}

	replacement->superNode = super;
	replacement->balance = element->balance;
	replacement->owningMap = this;

	MapElementBase *subnode = element->leftSubnode;
	replacement->leftSubnode = subnode;
	if (subnode)
	{
		subnode->superNode = replacement;
	}

	subnode = element->rightSubnode;
	replacement->rightSubnode = subnode;
	if (subnode)
	{
		subnode->superNode = replacement;
	}

	element->superNode = nullptr;
	element->leftSubnode = nullptr;
	element->rightSubnode = nullptr;
	element->owningMap = nullptr;
}

void MapBase::RemoveBranchNode(MapElementBase *node, MapElementBase *subnode)
{
	MapElementBase *super = node->superNode;
	if (subnode)
	{
		subnode->superNode = super;
	}

	if (super)
	{
		int32	db;

		if (super->leftSubnode == node)
		{
			super->leftSubnode = subnode;
			db = 1;
		}
		else
		{
			super->rightSubnode = subnode;
			db = -1;
		}

		for (;;)
		{
			int32 b = (super->balance += db);
			if (Abs(b) == 1)
			{
				break;
			}

			node = super;
			super = super->superNode;

			if (b != 0)
			{
				if (b > 0)
				{
					int32 rb = node->rightSubnode->balance;
					if (rb >= 0)
					{
						node = RotateLeft(node);
						if (rb == 0)
						{
							break;
						}
					}
					else
					{
						node = ZigZagLeft(node);
					}
				}
				else
				{
					int32 lb = node->leftSubnode->balance;
					if (lb <= 0)
					{
						node = RotateRight(node);
						if (lb == 0)
						{
							break;
						}
					}
					else
					{
						node = ZigZagRight(node);
					}
				}
			}

			if (!super)
			{
				break;
			}

			db = (super->leftSubnode == node) ? 1 : -1;
		}
	}
	else
	{
		rootElement = subnode;
	}
}

void MapBase::RemoveMapElement(MapElementBase *element)
{
	MapElementBase *left = element->leftSubnode;
	MapElementBase *right = element->rightSubnode;

	if ((left) && (right))
	{
		MapElementBase *top = right->GetFirstMapElement();
		RemoveBranchNode(top, top->rightSubnode);

		MapElementBase *super = element->superNode;
		top->superNode = super;
		if (super)
		{
			if (super->leftSubnode == element)
			{
				super->leftSubnode = top;
			}
			else
			{
				super->rightSubnode = top;
			}
		}
		else
		{
			rootElement = top;
		}

		left = element->leftSubnode;
		top->leftSubnode = left;
		if (left)
		{
			left->superNode = top;
		}

		right = element->rightSubnode;
		top->rightSubnode = right;
		if (right)
		{
			right->superNode = top;
		}

		top->balance = element->balance;
	}
	else
	{
		RemoveBranchNode(element, (left) ? left : right);
	}

	element->superNode = nullptr;
	element->leftSubnode = nullptr;
	element->rightSubnode = nullptr;
	element->owningMap = nullptr;
}

void MapBase::RemoveAllMapElements(void)
{
	if (rootElement)
	{
		rootElement->RemoveSubtree();
		rootElement = nullptr;
	}
}

void MapBase::PurgeMap(void)
{
	if (rootElement)
	{
		rootElement->DeleteSubtree();
		rootElement = nullptr;
	}
}
