
package com.fsoinstaller.common;

import java.util.ArrayList;
import java.util.List;

import javax.swing.event.TreeModelListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;


public class InstallerNodeTreeModel implements TreeModel
{
	protected final InstallerNodeRoot root;
	protected final List<TreeModelListener> treeModelListeners;

	public InstallerNodeTreeModel(InstallerNodeRoot root)
	{
		this.root = root;
		this.treeModelListeners = new ArrayList<TreeModelListener>();
	}

	protected List<InstallerNode> getInstallerNodeChildren(Object parent)
	{
		if (parent instanceof InstallerNodeRoot)
			return ((InstallerNodeRoot) parent).getChildren();
		else if (parent instanceof InstallerNode)
			return ((InstallerNode) parent).getChildren();
		else
			throw new IllegalArgumentException("Illegal node in tree!  Parent is " + parent.toString() + "; class is " + parent.getClass());
	}

	@Override
	public Object getChild(Object parent, int index)
	{
		return getInstallerNodeChildren(parent).get(index);
	}

	@Override
	public int getChildCount(Object parent)
	{
		return getInstallerNodeChildren(parent).size();
	}

	@Override
	public int getIndexOfChild(Object parent, Object child)
	{
		return getInstallerNodeChildren(parent).indexOf(child);
	}

	@Override
	public Object getRoot()
	{
		return root;
	}

	@Override
	public boolean isLeaf(Object node)
	{
		// all nodes have potential children
		return false;
	}

	@Override
	public void valueForPathChanged(TreePath path, Object newValue)
	{
		// we probably don't need to do anything here
	}

	@Override
	public void addTreeModelListener(TreeModelListener listener)
	{
		treeModelListeners.add(listener);
	}

	@Override
	public void removeTreeModelListener(TreeModelListener listener)
	{
		treeModelListeners.remove(listener);
	}
}
