
package com.fsoinstaller.common;

import java.net.URI;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;


public class InstallerNode
{
	protected String name;
	protected String description;
	protected String folder;
	protected String version;
	protected String note;

	protected final List<RenamePair> renameList;
	protected final List<String> deleteList;
	protected final List<String> installList;
	protected final List<URI> rootURLList;

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
		this.installList = new ArrayList<String>();
		this.rootURLList = new ArrayList<URI>();

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

	public List<String> getInstallList()
	{
		return Collections.unmodifiableList(installList);
	}

	public List<URI> getRootURLList()
	{
		return Collections.unmodifiableList(rootURLList);
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

	public void addInstall(String installItem)
	{
		if (installItem == null)
			throw new NullPointerException("Cannot add a null install item!");

		installList.add(installItem);
	}

	public void removeInstall(String installItem)
	{
		installList.remove(installItem);
	}

	public void addRootURL(URI rootURL)
	{
		if (rootURL == null)
			throw new NullPointerException("Cannot add a null root URL!");

		rootURLList.add(rootURL);
	}

	public void removeRootURL(URI rootURL)
	{
		rootURLList.remove(rootURL);
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

		@Override
		public boolean equals(Object object)
		{
			if (this == object)
				return true;
			if (object == null)
				return false;
			if (getClass() != object.getClass())
				return false;
			RenamePair other = (RenamePair) object;
			return to.equals(other.to) && from.equals(other.from);
		}

		@Override
		public int hashCode()
		{
			final int prime = 31;
			int result = 1;
			result = prime * result + from.hashCode();
			result = prime * result + to.hashCode();
			return result;
		}

		@Override
		public String toString()
		{
			StringBuilder builder = new StringBuilder();
			builder.append(from);
			builder.append("->");
			builder.append(to);
			return builder.toString();
		}
	}
}
