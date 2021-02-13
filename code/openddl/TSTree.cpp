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


#include "TSTree.h"


using namespace Terathon;


TreeBase::~TreeBase()
{
	PurgeSubtree();

	if (superNode)
	{
		superNode->RemoveSubnode(this);
	}
}

TreeBase *TreeBase::GetRootNode(void)
{
	TreeBase *root = this;
	for (;;)
	{
		TreeBase *node = root->superNode;
		if (!node)
		{
			break;
		}

		root = node;
	}

	return (root);
}

const TreeBase *TreeBase::GetRootNode(void) const
{
	const TreeBase *root = this;
	for (;;)
	{
		const TreeBase *node = root->superNode;
		if (!node)
		{
			break;
		}

		root = node;
	}

	return (root);
}

bool TreeBase::Successor(const TreeBase *node) const
{
	TreeBase *super = node->superNode;
	while (super)
	{
		if (super == this)
		{
			return (true);
		}

		super = super->superNode;
	}

	return (false);
}

TreeBase *TreeBase::GetLeftmostNode(void)
{
	TreeBase *node = this;
	for (;;)
	{
		TreeBase *subnode = node->firstSubnode;
		if (!subnode)
		{
			break;
		}

		node = subnode;
	}

	return (node);
}

const TreeBase *TreeBase::GetLeftmostNode(void) const
{
	const TreeBase *node = this;
	for (;;)
	{
		const TreeBase *subnode = node->firstSubnode;
		if (!subnode)
		{
			break;
		}

		node = subnode;
	}

	return (node);
}

TreeBase *TreeBase::GetRightmostNode(void)
{
	TreeBase *node = this;
	for (;;)
	{
		TreeBase *subnode = node->lastSubnode;
		if (!subnode)
		{
			break;
		}

		node = subnode;
	}

	return (node);
}

const TreeBase *TreeBase::GetRightmostNode(void) const
{
	const TreeBase *node = this;
	for (;;)
	{
		const TreeBase *subnode = node->lastSubnode;
		if (!subnode)
		{
			break;
		}

		node = subnode;
	}

	return (node);
}

TreeBase *TreeBase::GetNextTreeNode(const TreeBase *node) const
{
	TreeBase *next = node->firstSubnode;
	if (!next)
	{
		for (;;)
		{
			if (node == this)
			{
				break;
			}

			next = node->nextNode;
			if (next)
			{
				break;
			}

			node = node->superNode;
		}
	}

	return (next);
}

TreeBase *TreeBase::GetPreviousTreeNode(const TreeBase *node)
{
	if (node == this)
	{
		return (nullptr);
	}

	TreeBase *prev = node->prevNode;
	if (!prev)
	{
		return (node->superNode);
	}

	return (prev->GetRightmostNode());
}

const TreeBase *TreeBase::GetPreviousTreeNode(const TreeBase *node) const
{
	if (node == this)
	{
		return (nullptr);
	}

	const TreeBase *prev = node->prevNode;
	if (!prev)
	{
		return (node->superNode);
	}

	return (prev->GetRightmostNode());
}

TreeBase *TreeBase::GetNextLevelNode(const TreeBase *node) const
{
	TreeBase *next = nullptr;
	for (;;)
	{
		if (node == this)
		{
			break;
		}

		next = node->nextNode;
		if (next)
		{
			break;
		}

		node = node->superNode;
	}

	return (next);
}

TreeBase *TreeBase::GetPreviousLevelNode(const TreeBase *node) const
{
	TreeBase *prev = nullptr;
	for (;;)
	{
		if (node == this)
		{
			break;
		}

		prev = node->prevNode;
		if (prev)
		{
			break;
		}

		node = node->superNode;
	}

	return (prev);
}

int32 TreeBase::GetSubnodeCount(void) const
{
	machine count = 0;
	const TreeBase *subnode = firstSubnode;
	while (subnode)
	{
		count++;
		subnode = subnode->nextNode;
	}

	return (int32(count));
}

int32 TreeBase::GetSubtreeNodeCount(void) const
{
	machine count = 0;
	const TreeBase *subnode = firstSubnode;
	while (subnode)
	{
		count++;
		subnode = GetNextTreeNode(subnode);
	}

	return (int32(count));
}

int32 TreeBase::GetNodeIndex(void) const
{
	machine index = 0;

	const TreeBase *element = this;
	for (;;)
	{
		element = element->prevNode;
		if (!element)
		{
			break;
		}

		index++;
	}

	return (int32(index));
}

int32 TreeBase::GetNodeDepth(void) const
{
	machine depth = 0;

	const TreeBase *element = this;
	for (;;)
	{
		element = element->superNode;
		if (!element)
		{
			break;
		}

		depth++;
	}

	return (int32(depth));
}

void TreeBase::MoveSubtree(TreeBase *super)
{
	for (;;)
	{
		TreeBase *node = firstSubnode;
		if (!node)
		{
			break;
		}

		super->AppendSubnode(node);
	}
}

void TreeBase::RemoveSubtree(void)
{
	TreeBase *subnode = firstSubnode;
	while (subnode)
	{
		TreeBase *next = subnode->nextNode;
		subnode->prevNode = nullptr;
		subnode->nextNode = nullptr;
		subnode->superNode = nullptr;
		subnode = next;
	}

	firstSubnode = nullptr;
	lastSubnode = nullptr;
}

void TreeBase::PurgeSubtree(void)
{
	while (firstSubnode)
	{
		delete firstSubnode;
	}
}

void TreeBase::AppendSubnode(TreeBase *node)
{
	TreeBase *tree = node->superNode;
	if (tree)
	{
		TreeBase *prev = node->prevNode;
		TreeBase *next = node->nextNode;

		if (prev)
		{
			prev->nextNode = next;
			node->prevNode = nullptr;
		}

		if (next)
		{
			next->prevNode = prev;
			node->nextNode = nullptr;
		}

		if (tree->firstSubnode == node)
		{
			tree->firstSubnode = next;
		}

		if (tree->lastSubnode == node)
		{
			tree->lastSubnode = prev;
		}
	}

	node->superNode = this;

	if (lastSubnode)
	{
		lastSubnode->nextNode = node;
		node->prevNode = lastSubnode;
		lastSubnode = node;
	}
	else
	{
		firstSubnode = node;
		lastSubnode = node;
	}
}

void TreeBase::PrependSubnode(TreeBase *node)
{
	TreeBase *tree = node->superNode;
	if (tree)
	{
		TreeBase *prev = node->prevNode;
		TreeBase *next = node->nextNode;

		if (prev)
		{
			prev->nextNode = next;
			node->prevNode = nullptr;
		}

		if (next)
		{
			next->prevNode = prev;
			node->nextNode = nullptr;
		}

		if (tree->firstSubnode == node)
		{
			tree->firstSubnode = next;
		}

		if (tree->lastSubnode == node)
		{
			tree->lastSubnode = prev;
		}
	}

	node->superNode = this;

	if (firstSubnode)
	{
		firstSubnode->prevNode = node;
		node->nextNode = firstSubnode;
		firstSubnode = node;
	}
	else
	{
		firstSubnode = node;
		lastSubnode = node;
	}
}

void TreeBase::InsertSubnodeBefore(TreeBase *node, TreeBase *before)
{
	TreeBase *tree = node->superNode;
	if (tree)
	{
		TreeBase *prev = node->prevNode;
		TreeBase *next = node->nextNode;

		if (prev)
		{
			prev->nextNode = next;
		}

		if (next)
		{
			next->prevNode = prev;
		}

		if (tree->firstSubnode == node)
		{
			tree->firstSubnode = next;
		}

		if (tree->lastSubnode == node)
		{
			tree->lastSubnode = prev;
		}
	}

	node->superNode = this;
	node->nextNode = before;

	if (before)
	{
		TreeBase *after = before->prevNode;
		node->prevNode = after;
		before->prevNode = node;

		if (after)
		{
			after->nextNode = node;
		}
		else
		{
			firstSubnode = node;
		}
	}
	else
	{
		TreeBase *after = lastSubnode;
		node->prevNode = after;

		if (after)
		{
			after->nextNode = node;
			lastSubnode = node;
		}
		else
		{
			firstSubnode = node;
			lastSubnode = node;
		}
	}
}

void TreeBase::InsertSubnodeAfter(TreeBase *node, TreeBase *after)
{
	TreeBase *tree = node->superNode;
	if (tree)
	{
		TreeBase *prev = node->prevNode;
		TreeBase *next = node->nextNode;

		if (prev)
		{
			prev->nextNode = next;
		}

		if (next)
		{
			next->prevNode = prev;
		}

		if (tree->firstSubnode == node)
		{
			tree->firstSubnode = next;
		}

		if (tree->lastSubnode == node)
		{
			tree->lastSubnode = prev;
		}
	}

	node->superNode = this;
	node->prevNode = after;

	if (after)
	{
		TreeBase *before = after->nextNode;
		node->nextNode = before;
		after->nextNode = node;

		if (before)
		{
			before->prevNode = node;
		}
		else
		{
			lastSubnode = node;
		}
	}
	else
	{
		TreeBase *before = firstSubnode;
		node->nextNode = before;

		if (before)
		{
			before->prevNode = node;
			firstSubnode = node;
		}
		else
		{
			firstSubnode = node;
			lastSubnode = node;
		}
	}
}

void TreeBase::RemoveSubnode(TreeBase *node)
{
	TreeBase *prev = node->prevNode;
	TreeBase *next = node->nextNode;

	if (prev)
	{
		prev->nextNode = next;
	}

	if (next)
	{
		next->prevNode = prev;
	}

	if (firstSubnode == node)
	{
		firstSubnode = next;
	}

	if (lastSubnode == node)
	{
		lastSubnode = prev;
	}

	node->prevNode = nullptr;
	node->nextNode = nullptr;
	node->superNode = nullptr;
}

void TreeBase::Detach(void)
{
	if (superNode)
	{
		superNode->RemoveSubnode(this);
	}
}
