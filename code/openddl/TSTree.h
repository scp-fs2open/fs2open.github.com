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


#ifndef TSTree_h
#define TSTree_h


//# \component	Utility Library
//# \prefix		Utilities/


#include "TSPlatform.h"


#define TERATHON_TREE 1


namespace Terathon
{
	class TreeBase
	{
		private:

			TreeBase		*prevNode;
			TreeBase		*nextNode;
			TreeBase		*superNode;
			TreeBase		*firstSubnode;
			TreeBase		*lastSubnode;

			TreeBase(const TreeBase&) = delete;
			TreeBase& operator =(const TreeBase&) = delete;

		protected:

			TreeBase()
			{
				prevNode = nullptr;
				nextNode = nullptr;
				superNode = nullptr;
				firstSubnode = nullptr;
				lastSubnode = nullptr;
			}

			TERATHON_API virtual ~TreeBase();

			TreeBase *GetPreviousSubnode(void) const
			{
				return (prevNode);
			}

			TreeBase *GetNextSubnode(void) const
			{
				return (nextNode);
			}

			TreeBase *GetSuperNode(void) const
			{
				return (superNode);
			}

			TreeBase *GetFirstSubnode(void) const
			{
				return (firstSubnode);
			}

			TreeBase *GetLastSubnode(void) const
			{
				return (lastSubnode);
			}

			TERATHON_API TreeBase *GetRootNode(void);
			TERATHON_API const TreeBase *GetRootNode(void) const;

			TERATHON_API bool Successor(const TreeBase *node) const;

			TERATHON_API TreeBase *GetLeftmostNode(void);
			TERATHON_API const TreeBase *GetLeftmostNode(void) const;
			TERATHON_API TreeBase *GetRightmostNode(void);
			TERATHON_API const TreeBase *GetRightmostNode(void) const;

			TERATHON_API TreeBase *GetNextTreeNode(const TreeBase *node) const;
			TERATHON_API TreeBase *GetPreviousTreeNode(const TreeBase *node);
			TERATHON_API const TreeBase *GetPreviousTreeNode(const TreeBase *node) const;
			TERATHON_API TreeBase *GetNextLevelNode(const TreeBase *node) const;
			TERATHON_API TreeBase *GetPreviousLevelNode(const TreeBase *node) const;

			TERATHON_API void MoveSubtree(TreeBase *super);

			TERATHON_API void AppendSubnode(TreeBase *node);
			TERATHON_API void PrependSubnode(TreeBase *node);
			TERATHON_API void InsertSubnodeBefore(TreeBase *node, TreeBase *before);
			TERATHON_API void InsertSubnodeAfter(TreeBase *node, TreeBase *after);
			TERATHON_API void RemoveSubnode(TreeBase *node);

		public:

			TERATHON_API int32 GetSubnodeCount(void) const;
			TERATHON_API int32 GetSubtreeNodeCount(void) const;

			TERATHON_API int32 GetNodeIndex(void) const;
			TERATHON_API int32 GetNodeDepth(void) const;

			TERATHON_API void RemoveSubtree(void);
			TERATHON_API void PurgeSubtree(void);

			TERATHON_API virtual void Detach(void);
	};


	//# \class	Tree	The base class for objects that can be stored in a hierarchical tree.
	//
	//# Objects inherit from the $Tree$ class so that they can be stored in a hierarchical tree.
	//
	//# \def	template <class type> class Tree : public TreeBase
	//
	//# \tparam		type	The type of the class that can be stored in a tree. This parameter should be the
	//#						type of the class that inherits directly from the $Tree$ class.
	//
	//# \ctor	Tree();
	//
	//# \desc
	//# The $Tree$ class should be declared as a base class for objects that need to be stored in a hierarchical tree.
	//# The $type$ template parameter should match the class type of such objects.
	//#
	//# When a $Tree$ object is initially created, its super node is $nullptr$ and it has no subnodes. An object having no super node
	//# is the root of a tree. (A newly created $Tree$ object can be thought of as the root of a tree containing a single object.)
	//# In a normal tree structure, one object is created to serve as the root node. Objects are added to the tree by calling the
	//# $@Tree::AppendSubnode@$ function to designate them as subnodes of other objects in the tree.
	//#
	//# A tree is traversed in depth-first order using the $@Tree::GetNextTreeNode@$ function. The root node of a tree is considered
	//# to be the first node visited in a traversal. Iterative calls to the $@Tree::GetNextTreeNode@$ function visit every non-root
	//# node in a tree.
	//#
	//# When a $Tree$ object is destroyed, all of its subnodes (and all of their subnodes, etc.) are also destroyed.
	//
	//# \privbase	TreeBase	Used internally to encapsulate common functionality that is independent
	//#							of the template parameter.


	//# \function	Tree::GetSuperNode		Returns the super node of an object.
	//
	//# \proto	type *GetSuperNode(void) const;
	//
	//# \desc
	//# The $GetSuperNode$ function returns the direct super node above an object in a tree. If the object is the root of a
	//# tree, then the return value is $nullptr$.
	//
	//# \also	$@Tree::GetRootNode@$
	//# \also	$@Tree::Successor@$
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetNextTreeNode@$


	//# \function	Tree::GetRootNode		Returns the root node of the tree containing an object.
	//
	//# \proto	type *GetRootNode(void);
	//# \proto	const type *GetRootNode(void) const;
	//
	//# \desc
	//# The $GetRootNode$ function returns the root node of the tree containing an object. If the object is the root node
	//# of a tree, then the return value is a pointer to itself.
	//
	//# \also	$@Tree::GetSuperNode@$
	//# \also	$@Tree::Successor@$
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetNextTreeNode@$


	//# \function	Tree::GetFirstSubnode		Returns the first subnode of an object.
	//
	//# \proto	type *GetFirstSubnode(void) const;
	//
	//# \desc
	//# The $GetFirstSubnode$ function returns the first direct subnode of an object is a tree. If the object has no subnodes,
	//# then the return value is $nullptr$.
	//#
	//# All of the direct subnodes of an object are stored in a single linked list. To go from one subnode to the next,
	//# starting with the first, the $@Tree::GetNextSubnode@$ function should be called.
	//
	//# \also	$@Tree::GetNextTreeNode@$
	//# \also	$@Tree::GetLastSubnode@$
	//# \also	$@Tree::GetSuperNode@$
	//# \also	$@Tree::GetRootNode@$


	//# \function	Tree::GetLastSubnode		Returns the last subnode of an object.
	//
	//# \proto	type *GetLastSubnode(void) const;
	//
	//# \desc
	//# The $GetLastSubnode$ function returns the last direct subnode of an object is a tree. If the object has no subnodes,
	//# then the return value is $nullptr$.
	//#
	//# All of the direct subnodes of an object are stored in a single linked list. To go from one subnode to the previous,
	//# starting with the last, the $@Tree::GetPreviousSubnode@$ function should be called.
	//
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetSuperNode@$
	//# \also	$@Tree::GetRootNode@$


	//# \function	Tree::GetPreviousSubnode		Returns the previous subnode in the same list of subnodes.
	//
	//# \proto	type *GetPreviousSubnode(void) const;
	//
	//# \desc
	//# Let <i>N</i> represent the tree node for which the $GetPreviousSubnode$ function is called. The $GetPreviousSubnode$ function
	//# returns the previous subnode in the list of subnodes to which the node <i>N</i> belongs. If <i>N</i> is the first subnode in
	//# the list, then the return value is $nullptr$.
	//
	//# \also	$@Tree::GetNextSubnode@$
	//# \also	$@Tree::GetNextTreeNode@$
	//# \also	$@Tree::GetNextLevelNode@$
	//# \also	$@Tree::GetPreviousTreeNode@$
	//# \also	$@Tree::GetPreviousLevelNode@$
	//# \also	$@Tree::GetSuperNode@$
	//# \also	$@Tree::GetRootNode@$


	//# \function	Tree::GetNextSubnode			Returns the next subnode in the same list of subnodes.
	//
	//# \proto	type *GetNextSubnode(void) const;
	//
	//# \desc
	//# Let <i>N</i> represent the tree node for which the $GetNextSubnode$ function is called. The $GetNextSubnode$ function
	//# returns the next subnode in the list of subnodes to which the node <i>N</i> belongs. If <i>N</i> is the last subnode in
	//# the list, then the return value is $nullptr$.
	//
	//# \also	$@Tree::GetPreviousSubnode@$
	//# \also	$@Tree::GetNextTreeNode@$
	//# \also	$@Tree::GetNextLevelNode@$
	//# \also	$@Tree::GetPreviousTreeNode@$
	//# \also	$@Tree::GetPreviousLevelNode@$
	//# \also	$@Tree::GetSuperNode@$
	//# \also	$@Tree::GetRootNode@$


	//# \function	Tree::GetNextTreeNode		Returns the next node in a traversal of a tree.
	//
	//# \proto	type *GetNextTreeNode(const Tree<type> *node) const;
	//
	//# \param	node	A pointer to the current node in the traversal.
	//
	//# \desc
	//# To iterate forward through the subnode hierarchy of a tree, the $GetNextTreeNode$ function should be repeatedly called for
	//# the root node of the tree until it returns $nullptr$. The root node is considered the first node visited in the tree.
	//# The second node is obtained by calling $GetNextTreeNode$ with the $node$ parameter set to a pointer to the root node. The
	//# traversal occurs in depth-first order, meaning that a particular node's entire subtree is visited before the next node
	//# at the same level in the tree. The traversal is also a pre-order traversal, meaning that a particular node is visited
	//# before any of its subnodes are visited.
	//
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetNextLevelNode@$
	//# \also	$@Tree::GetPreviousTreeNode@$
	//# \also	$@Tree::GetPreviousLevelNode@$
	//# \also	$@Tree::GetPreviousSubnode@$
	//# \also	$@Tree::GetNextSubnode@$
	//# \also	$@Tree::GetSuperNode@$
	//# \also	$@Tree::GetRootNode@$


	//# \function	Tree::GetNextLevelNode		Returns the next node in a traversal of a tree that is not a subnode of the current node.
	//
	//# \proto	type *GetNextLevelNode(const Tree<type> *node) const;
	//
	//# \param	node	A pointer to the current node in the traversal.
	//
	//# \desc
	//# During iteration of the nodes in a tree, the $GetNextLevelNode$ function can be used to jump to the next subnode on the
	//# same level as the $node$ parameter, skipping its entire subtree. Node selection after skipping the subtree behaves
	//# exactly like that used by the $@Tree::GetNextTreeNode@$ function.
	//
	//# \also	$@Tree::GetNextTreeNode@$
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetPreviousTreeNode@$
	//# \also	$@Tree::GetPreviousLevelNode@$
	//# \also	$@Tree::GetPreviousSubnode@$
	//# \also	$@Tree::GetNextSubnode@$
	//# \also	$@Tree::GetSuperNode@$
	//# \also	$@Tree::GetRootNode@$


	//# \function	Tree::GetPreviousTreeNode		Returns the previous node in a traversal of a tree.
	//
	//# \proto	type *GetPreviousTreeNode(const Tree<type> *node) const;
	//
	//# \param	node	A pointer to the current node in the traversal.
	//
	//# \desc
	//# To iterate backward through the subnode hierarchy of a tree, the $GetPreviousTreeNode$ function should be repeatedly called
	//# for the root node of the tree until it returns $nullptr$. The root node is considered the first node visited in the tree.
	//# The second node is obtained by calling $GetPreviousTreeNode$ with the $node$ parameter set to a pointer to the root node. The
	//# traversal occurs in depth-first order, meaning that a particular node's entire subtree is visited before the previous node
	//# at the same level in the tree. The traversal is also a pre-order traversal, meaning that a particular node is visited
	//# before any of its subnodes are visited.
	//
	//# \also	$@Tree::GetLastSubnode@$
	//# \also	$@Tree::GetPreviousLevelNode@$
	//# \also	$@Tree::GetNextTreeNode@$
	//# \also	$@Tree::GetNextLevelNode@$
	//# \also	$@Tree::GetPreviousSubnode@$
	//# \also	$@Tree::GetNextSubnode@$
	//# \also	$@Tree::GetSuperNode@$
	//# \also	$@Tree::GetRootNode@$


	//# \function	Tree::GetPreviousLevelNode		Returns the previous node in a traversal of a tree that is not a subnode of the current node.
	//
	//# \proto	type *GetPreviousLevelNode(const Tree<type> *node) const;
	//
	//# \param	node	A pointer to the current node in the traversal.
	//
	//# \desc
	//# During iteration of the nodes in a tree, the $GetPreviousLevelNode$ function can be used to jump to the previous subnode
	//# on the same level as the $node$ parameter, skipping its entire subtree. Node selection after skipping the subtree behaves
	//# exactly like that used by the $@Tree::GetPreviousTreeNode@$ function.
	//
	//# \also	$@Tree::GetPreviousTreeNode@$
	//# \also	$@Tree::GetLastSubnode@$
	//# \also	$@Tree::GetNextTreeNode@$
	//# \also	$@Tree::GetNextLevelNode@$
	//# \also	$@Tree::GetPreviousSubnode@$
	//# \also	$@Tree::GetNextSubnode@$
	//# \also	$@Tree::GetSuperNode@$
	//# \also	$@Tree::GetRootNode@$


	//# \function	Tree::GetSubnodeCount		Returns the number of immediate subnodes of an object.
	//
	//# \proto	int32 GetSubnodeCount(void) const;
	//
	//# \desc
	//# The $GetSubnodeCount$ function returns the number of the immediate subnodes of an object.
	//
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetLastSubnode@$


	//# \function	Tree::AppendSubnode		Adds a subnode to an object at the end of the subnode list.
	//
	//# \proto	virtual void AppendSubnode(Tree<type> *node);
	//
	//# \param	node	A pointer to the subnode to add.
	//
	//# \desc
	//# The $AppendSubnode$ function adds the node specified by the $node$ parameter to an object at the end of the list of
	//# subnodes for that object. If the node is already a subnode of the object, then it is moved to the end of the list of subnodes.
	//#
	//# If the node being added is already a member of a different tree of the same type, then it is first removed from that tree.
	//# If the node has subnodes itself, then those subnodes are carried with the node into the new tree.
	//
	//# \also	$@Tree::PrependSubnode@$
	//# \also	$@Tree::InsertSubnodeBefore@$
	//# \also	$@Tree::InsertSubnodeAfter@$
	//# \also	$@Tree::RemoveSubnode@$
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetNextTreeNode@$


	//# \function	Tree::PrependSubnode	Adds a subnode to an object at the beginning of the subnode list.
	//
	//# \proto	virtual void PrependSubnode(Tree<type> *node);
	//
	//# \param	node	A pointer to the subnode to add.
	//
	//# \desc
	//# The $AppendSubnode$ function adds the node specified by the $node$ parameter to an object at the beginning of the list of
	//# subnodes for that object. If the node is already a subnode of the object, then it is moved to the beginning of the list of subnodes.
	//#
	//# If the node being added is already a member of a different tree of the same type, then it is first removed from that tree.
	//# If the node has subnodes itself, then those subnodes are carried with the node into the new tree.
	//
	//# \also	$@Tree::AppendSubnode@$
	//# \also	$@Tree::InsertSubnodeBefore@$
	//# \also	$@Tree::InsertSubnodeAfter@$
	//# \also	$@Tree::RemoveSubnode@$
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetNextTreeNode@$


	//# \function	Tree::InsertSubnodeBefore		Adds a subnode to an object before a specific subnode in the subnode list.
	//
	//# \proto	virtual void InsertSubnodeBefore(Tree<type> *node, Tree<type> *before);
	//
	//# \param	node	A pointer to the subnode to add.
	//# \param	before	A pointer to the subnode before which the new subnode is added.
	//
	//# \desc
	//# The $InsertSubnodeBefore$ function adds the node specified by the $node$ parameter to an object immediately before the node
	//# specified by the $before$ parameter in the list of subnodes for that object. If the node is already a subnode of the object,
	//# then it is moved to the new position in the subnode list. If the $before$ parameter is $nullptr$, then the node is added to
	//# the end of the subnode list. Otherwise, the $before$ parameter must specify a node that is already a subnode of the tree for
	//# which this function is called.
	//#
	//# If the node being added is already a member of a different tree of the same type, then it is first removed from that tree.
	//# If the node has subnodes itself, then those subnodes are carried with the node into the new tree.
	//
	//# \also	$@Tree::InsertSubnodeAfter@$
	//# \also	$@Tree::AppendSubnode@$
	//# \also	$@Tree::PrependSubnode@$
	//# \also	$@Tree::RemoveSubnode@$
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetNextTreeNode@$


	//# \function	Tree::InsertSubnodeAfter		Adds a subnode to an object after a specific subnode in the subnode list.
	//
	//# \proto	virtual void InsertSubnodeAfter(Tree<type> *node, Tree<type> *after);
	//
	//# \param	node	A pointer to the subnode to add.
	//# \param	after	A pointer to the subnode after which the new subnode is added.
	//
	//# \desc
	//# The $InsertSubnodeAfter$ function adds the node specified by the $node$ parameter to an object immediately after the node
	//# specified by the $after$ parameter in the list of subnodes for that object. If the node is already a subnode of the object,
	//# then it is moved to the new position in the subnode list. If the $after$ parameter is $nullptr$, then the node is added to
	//# the beginning of the subnode list. Otherwise, the $after$ parameter must specify a node that is already a subnode of the tree
	//# for which this function is called.
	//#
	//# If the node being added is already a member of a different tree of the same type, then it is first removed from that tree.
	//# If the node has subnodes itself, then those subnodes are carried with the node into the new tree.
	//
	//# \also	$@Tree::InsertSubnodeBefore@$
	//# \also	$@Tree::AppendSubnode@$
	//# \also	$@Tree::PrependSubnode@$
	//# \also	$@Tree::RemoveSubnode@$
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetNextTreeNode@$


	//# \function	Tree::RemoveSubnode		Removes a direct subnode of an object.
	//
	//# \proto	virtual void RemoveSubnode(Tree<type> *node);
	//
	//# \param	node	A pointer to the subnode to remove.
	//
	//# \desc
	//# The $RemoveSubnode$ function removes the node pointed to by the $node$ parameter from an object. The node must be
	//# an existing subnode of the object for which $RemoveSubnode$ is called. If the node has any subnodes, then those
	//# subnodes remain subnodes of the removed node.
	//
	//# \also	$@Tree::AppendSubnode@$
	//# \also	$@Tree::GetFirstSubnode@$
	//# \also	$@Tree::GetNextTreeNode@$


	//# \function	Tree::MoveSubtree		Moves all subnodes of an object to another object.
	//
	//# \proto	virtual void MoveSubtree(Tree<type> *super);
	//
	//# \param	super	The object to which the subnodes are moved. This cannot be the same object for which
	//#					the $MoveSubtree$ function is called.
	//
	//# \desc
	//# The $MoveSubtree$ function removes all of the subnodes of an object and then transfers all of them as
	//# subnodes of the object specified by the $super$ parameter.
	//
	//# \also	$@Tree::AppendSubnode@$
	//# \also	$@Tree::RemoveSubnode@$
	//# \also	$@Tree::PurgeSubtree@$


	//# \function	Tree::RemoveSubtree		Removes all subnodes of an object.
	//
	//# \proto	void RemoveSubtree(void);
	//
	//# \desc
	//# The $RemoveSubtree$ function removes all of the immediate subnodes of an object.
	//
	//# \also	$@Tree::RemoveSubnode@$
	//# \also	$@Tree::PurgeSubtree@$


	//# \function	Tree::PurgeSubtree		Deletes all subnodes of an object.
	//
	//# \proto	void PurgeSubtree(void);
	//
	//# \desc
	//# The $PurgeSubtree$ function recursively deletes all of the subnodes of an object.
	//
	//# \also	$@Tree::RemoveSubtree@$
	//# \also	$@Tree::RemoveSubnode@$


	//# \function	Tree::Successor		Returns a boolean value indicating whether one node is a successor of another.
	//
	//# \proto	bool Successor(const Tree<type> *node) const;
	//
	//# \desc
	//# The $Successor$ function returns $true$ if the node specified by the $node$ parameter is a successor
	//# of the node for which the function is called. If the node is not a successor, this function returns $false$.
	//
	//# \also	$@Tree::GetSuperNode@$
	//# \also	$@Tree::GetRootNode@$


	template <class type>
	class Tree : public TreeBase
	{
		protected:

			inline Tree() = default;

		public:

			inline ~Tree() = default;

			type *GetPreviousSubnode(void) const
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetPreviousSubnode())));
			}

			type *GetNextSubnode(void) const
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetNextSubnode())));
			}

			type *GetSuperNode(void) const
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetSuperNode())));
			}

			type *GetFirstSubnode(void) const
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetFirstSubnode())));
			}

			type *GetLastSubnode(void) const
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetLastSubnode())));
			}

			type *GetRootNode(void)
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetRootNode())));
			}

			const type *GetRootNode(void) const
			{
				return (static_cast<const type *>(static_cast<const Tree<type> *>(TreeBase::GetRootNode())));
			}

			bool Successor(const Tree<type> *node) const
			{
				return (TreeBase::Successor(node));
			}

			type *GetLeftmostNode(void)
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetLeftmostNode())));
			}

			const type *GetLeftmostNode(void) const
			{
				return (static_cast<const type *>(static_cast<const Tree<type> *>(TreeBase::GetLeftmostNode())));
			}

			type *GetRightmostNode(void)
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetRightmostNode())));
			}

			const type *GetRightmostNode(void) const
			{
				return (static_cast<const type *>(static_cast<const Tree<type> *>(TreeBase::GetRightmostNode())));
			}

			type *GetNextTreeNode(const Tree<type> *node) const
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetNextTreeNode(node))));
			}

			type *GetPreviousTreeNode(const Tree<type> *node)
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetPreviousTreeNode(node))));
			}

			const type *GetPreviousTreeNode(const Tree<type> *node) const
			{
				return (static_cast<const type *>(static_cast<const Tree<type> *>(TreeBase::GetPreviousTreeNode(node))));
			}

			type *GetNextLevelNode(const Tree<type> *node) const
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetNextLevelNode(node))));
			}

			type *GetPreviousLevelNode(const Tree<type> *node) const
			{
				return (static_cast<type *>(static_cast<Tree<type> *>(TreeBase::GetPreviousLevelNode(node))));
			}

			void MoveSubtree(Tree<type> *super)
			{
				TreeBase::MoveSubtree(super);
			}

			virtual void AppendSubnode(type *node);
			virtual void PrependSubnode(type *node);
			virtual void InsertSubnodeBefore(type *node, type *before);
			virtual void InsertSubnodeAfter(type *node, type *after);
			virtual void RemoveSubnode(type *node);
	};


	template <class type>
	void Tree<type>::AppendSubnode(type *node)
	{
		TreeBase::AppendSubnode(static_cast<Tree<type> *>(node));
	}

	template <class type>
	void Tree<type>::PrependSubnode(type *node)
	{
		TreeBase::PrependSubnode(static_cast<Tree<type> *>(node));
	}

	template <class type>
	void Tree<type>::InsertSubnodeBefore(type *node, type *before)
	{
		TreeBase::InsertSubnodeBefore(static_cast<Tree<type> *>(node), static_cast<Tree<type> *>(before));
	}

	template <class type>
	void Tree<type>::InsertSubnodeAfter(type *node, type *after)
	{
		TreeBase::InsertSubnodeAfter(static_cast<Tree<type> *>(node), static_cast<Tree<type> *>(after));
	}

	template <class type>
	void Tree<type>::RemoveSubnode(type *node)
	{
		TreeBase::RemoveSubnode(static_cast<Tree<type> *>(node));
	}
}


#endif
