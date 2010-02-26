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


/**
 * This class represents a single unit of installation, such as a mod, with a
 * group of associated files.
 * 
 * @author Goober5000
 */
public class InstallerNode
{
	protected String name;
	protected String description;
	protected String folder;
	protected String version;
	protected String note;

	protected final List<RenamePair> renameList;
	protected final List<String> deleteList;
	protected final List<InstallUnit> installList;

	protected final List<InstallerNode> children;

	public InstallerNode(String name)
	{
		if (name == null)
			throw new NullPointerException("The 'name' field cannot be null!");

		this.name = name;
		this.description = null;
		this.folder = null;
		this.version = null;
		this.note = null;

		this.renameList = new ArrayList<RenamePair>();
		this.deleteList = new ArrayList<String>();
		this.installList = new ArrayList<InstallUnit>();

		this.children = new ArrayList<InstallerNode>();
	}

	public String getName()
	{
		return name;
	}

	public void setName(String name)
	{
		if (name == null)
			throw new NullPointerException("The 'name' field cannot be null!");

		this.name = name;
	}

	public String getDescription()
	{
		return description;
	}

	public void setDescription(String description)
	{
		this.description = description;
	}

	public String getFolder()
	{
		return folder;
	}

	public void setFolder(String folder)
	{
		this.folder = folder;
	}

	public String getVersion()
	{
		return version;
	}

	public void setVersion(String version)
	{
		this.version = version;
	}

	public String getNote()
	{
		return note;
	}

	public void setNote(String note)
	{
		this.note = note;
	}

	public List<RenamePair> getRenameList()
	{
		return Collections.unmodifiableList(renameList);
	}

	public List<String> getDeleteList()
	{
		return Collections.unmodifiableList(deleteList);
	}

	public List<InstallUnit> getInstallList()
	{
		return Collections.unmodifiableList(installList);
	}

	public List<InstallerNode> getChildren()
	{
		return Collections.unmodifiableList(children);
	}

	public void addRenamePair(RenamePair renamePair)
	{
		if (renamePair == null)
			throw new NullPointerException("Cannot add a null rename pair!");

		renameList.add(renamePair);
	}

	public void removeRenamePair(RenamePair renamePair)
	{
		renameList.remove(renamePair);
	}

	public void addDelete(String deleteItem)
	{
		if (deleteItem == null)
			throw new NullPointerException("Cannot add a null delete item!");

		deleteList.add(deleteItem);
	}

	public void removeDelete(String deleteItem)
	{
		deleteList.remove(deleteItem);
	}

	public void addInstall(InstallUnit installUnit)
	{
		if (installUnit == null)
			throw new NullPointerException("Cannot add a null install unit!");

		installList.add(installUnit);
	}

	public void removeInstall(InstallUnit installUnit)
	{
		installList.remove(installUnit);
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
	public boolean equals(Object object)
	{
		if (this == object)
			return true;
		if (object == null)
			return false;
		if (getClass() != object.getClass())
			return false;
		InstallerNode other = (InstallerNode) object;
		return name.equals(other.name);
	}

	@Override
	public int hashCode()
	{
		return name.hashCode();
	}

	@Override
	public String toString()
	{
		return name;
	}

	public static class RenamePair
	{
		private String from;
		private String to;

		public RenamePair(String from, String to)
		{
			if (from == null || to == null)
				throw new NullPointerException("Arguments cannot be null!");

			this.from = from;
			this.to = to;
		}

		public String getFrom()
		{
			return from;
		}

		public void setFrom(String from)
		{
			if (from == null)
				throw new NullPointerException("The 'from' field cannot be null!");

			this.from = from;
		}

		public String getTo()
		{
			return to;
		}

		public void setTo(String to)
		{
			if (from == null)
				throw new NullPointerException("The 'to' field cannot be null!");

			this.to = to;
		}
	}

	public static class InstallUnit
	{
		private List<BaseURL> baseURLList;
		private List<String> fileList;

		public InstallUnit()
		{
			this.baseURLList = new ArrayList<BaseURL>();
			this.fileList = new ArrayList<String>();
		}

		public List<BaseURL> getBaseURLList()
		{
			return Collections.unmodifiableList(baseURLList);
		}

		public List<String> getFileList()
		{
			return Collections.unmodifiableList(fileList);
		}

		public void addBaseURL(BaseURL baseURL)
		{
			if (baseURL == null)
				throw new NullPointerException("Cannot add a null base URL!");

			baseURLList.add(baseURL);
		}

		public void removeBaseURL(BaseURL baseURL)
		{
			baseURLList.remove(baseURL);
		}

		public void addFile(String file)
		{
			if (file == null)
				throw new NullPointerException("Cannot add a null file!");

			fileList.add(file);
		}

		public void removeFile(String file)
		{
			fileList.remove(file);
		}
	}
}
