
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
