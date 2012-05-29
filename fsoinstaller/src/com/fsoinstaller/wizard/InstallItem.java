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

package com.fsoinstaller.wizard;

import java.util.Map;
import java.util.Set;
import java.util.concurrent.ExecutorService;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;

import com.fsoinstaller.common.InstallerNode;
import com.fsoinstaller.common.InstallerNode.InstallUnit;
import com.fsoinstaller.common.InstallerNode.RenamePair;
import com.fsoinstaller.internet.Downloader;
import com.fsoinstaller.main.Configuration;


public class InstallItem extends JPanel
{
	private final ExecutorService exec;
	private final InstallerNode node;
	private final Set<String> modsToInstall;
	
	public InstallItem(ExecutorService exec, InstallerNode node, Set<String> modsToInstall)
	{
		super();
		this.exec = exec;
		this.node = node;
		this.modsToInstall = modsToInstall;
		
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		add(new JLabel(node.getName()));
		add(Box.createHorizontalGlue());
	}
	
	public void start()
	{
		for (String delete: node.getDeleteList())
		{
			final String _delete = delete;
			exec.submit(new Runnable()
			{
				@Override
				public void run()
				{
					System.out.println("DELETE " + _delete);
				}
			});
		}
		
		// ensure all deletions happen before renames start
		// (use a barrier or some sort)
		
		for (RenamePair rename: node.getRenameList())
		{
			final RenamePair _rename = rename;
			exec.submit(new Runnable()
			{
				@Override
				public void run()
				{
					System.out.println("RENAME " + _rename.getFrom() + " TO " + _rename.getTo());
				}
			});
		}
		
		// another barrier
		
		for (InstallUnit install: node.getInstallList())
		{
			final InstallUnit _install = install;
			exec.submit(new Runnable()
			{
				@Override
				public void run()
				{
					System.out.println("INSTALL " + _install.getFileList());
				}
			});
		}
		
		// another barrier: all installs should be successful before child nodes are added
		
		// TODO: create new InstallItems for child nodes and add them to InstallItem's parent panel
	}
	
	public void cancel()
	{
	}
}
