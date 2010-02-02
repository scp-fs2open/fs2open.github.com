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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;


public class InstallerNodeRoot
{
	protected final List<InstallerNode> children;

	public InstallerNodeRoot()
	{
		this.children = new ArrayList<InstallerNode>();
	}

	public List<InstallerNode> getChildren()
	{
		return Collections.unmodifiableList(children);
	}

	public void addChild(InstallerNode installerNode)
	{
		if (installerNode == null)
			throw new NullPointerException("Cannot add a null child!");

		children.add(installerNode);
	}

	public void removeChild(InstallerNode installerNode)
	{
		children.remove(installerNode);
	}

	@Override
	public String toString()
	{
		return "Project Root";
	}
}
