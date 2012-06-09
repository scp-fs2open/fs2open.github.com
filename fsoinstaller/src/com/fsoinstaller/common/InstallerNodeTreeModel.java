/*
 * This file is part of the FreeSpace Open Installer
 * Copyright (C) 2010 The FreeSpace 2 Source Code Project
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

package com.fsoinstaller.common;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

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
		this.treeModelListeners = new CopyOnWriteArrayList<TreeModelListener>();
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
	
	public Object getChild(Object parent, int index)
	{
		return getInstallerNodeChildren(parent).get(index);
	}
	
	public int getChildCount(Object parent)
	{
		return getInstallerNodeChildren(parent).size();
	}
	
	public int getIndexOfChild(Object parent, Object child)
	{
		return getInstallerNodeChildren(parent).indexOf(child);
	}
	
	public Object getRoot()
	{
		return root;
	}
	
	public boolean isLeaf(Object node)
	{
		// all nodes have potential children
		return false;
	}
	
	public void valueForPathChanged(TreePath path, Object newValue)
	{
		// we probably don't need to do anything here
	}
	
	public void addTreeModelListener(TreeModelListener listener)
	{
		treeModelListeners.add(listener);
	}
	
	public void removeTreeModelListener(TreeModelListener listener)
	{
		treeModelListeners.remove(listener);
	}
}
