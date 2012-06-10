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

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;

import com.fsoinstaller.common.BaseURL;
import com.fsoinstaller.common.InstallerNode;
import com.fsoinstaller.common.InstallerNode.InstallUnit;
import com.fsoinstaller.common.InstallerNode.RenamePair;
import com.fsoinstaller.internet.Downloader;
import com.fsoinstaller.main.Configuration;
import com.fsoinstaller.utils.Logger;
import com.fsoinstaller.utils.ProgressBarDialog;

import static com.fsoinstaller.wizard.GUIConstants.*;


public class InstallItem extends JPanel implements Callable<Boolean>
{
	private static final Logger logger = Logger.getLogger(InstallItem.class);
	
	private final InstallerNode node;
	private final JProgressBar bar;
	
	public InstallItem(InstallerNode node)
	{
		super();
		this.node = node;
		
		setBorder(BorderFactory.createEmptyBorder(SMALL_MARGIN, SMALL_MARGIN, SMALL_MARGIN, SMALL_MARGIN));
		setLayout(new BorderLayout(0, SMALL_MARGIN));
		
		bar = new JProgressBar(0, 100);
		bar.setIndeterminate(true);
		bar.setString(ProgressBarDialog.INDETERMINATE_STRING);
		bar.setStringPainted(true);
		bar.setPreferredSize(new Dimension((int) bar.getPreferredSize().getWidth(), (int) bar.getMinimumSize().getHeight()));
		bar.setMaximumSize(new Dimension((int) bar.getMaximumSize().getWidth(), (int) bar.getMinimumSize().getHeight()));
		
		add(new JLabel(node.getName()), BorderLayout.NORTH);
		add(bar, BorderLayout.CENTER);
		
		setMaximumSize(new Dimension((int) getMaximumSize().getWidth(), (int) getPreferredSize().getHeight()));
	}
	
	/**
	 * Perform the installation tasks for this node.
	 * 
	 * @throws SecurityException if e.g. we are not allowed to create a folder
	 *         or download files to it
	 */
	public Boolean call() throws SecurityException
	{
		File installDir = Configuration.getInstance().getApplicationDir();
		String nodeName = node.getName();
		
		logger.info(nodeName + ": Starting processing");
		
		// create the folder for this mod, if it has one
		File folder;
		String folderName = node.getFolder();
		if (folderName == null || folderName.length() == 0 || folderName.equals("/") || folderName.equals("\\"))
		{
			logger.debug(nodeName + ": This node has no folder; using application folder instead");
			folder = installDir;
		}
		else
		{
			logger.info(nodeName + ": Creating folder '" + folderName + "'");
			folder = new File(installDir, folderName);
			if (!folder.exists() && !folder.mkdir())
			{
				logger.error(nodeName + ": Unable to create the '" + folderName + "' folder!");
				return false;
			}
		}
		
		logger.info(nodeName + ": Processing DELETE items");
		
		// delete what we need to
		for (String delete: node.getDeleteList())
		{
			logger.debug(nodeName + ": Deleting '" + delete + "'");
			File file = new File(installDir, delete);
			if (file.exists())
			{
				if (file.isDirectory())
					logger.debug(nodeName + ": Cannot delete '" + delete + "'; deleting directories is not supported at this time");
				else if (!file.delete())
				{
					logger.error(nodeName + ": Unable to delete '" + delete + "'!");
					return false;
				}
			}
		}
		
		logger.info(nodeName + ": Processing RENAME items");
		
		// rename what we need to
		for (RenamePair rename: node.getRenameList())
		{
			logger.debug(nodeName + ": Renaming '" + rename.getFrom() + "' to '" + rename.getTo() + "'");
			File from = new File(installDir, rename.getFrom());
			File to = new File(installDir, rename.getTo());
			if (!from.exists())
				logger.debug(nodeName + ": Cannot rename '" + rename.getFrom() + "'; it does not exist");
			else if (to.exists())
				logger.debug(nodeName + ": Cannot rename '" + rename.getFrom() + "' to '" + rename.getTo() + "'; the latter already exists");
			else if (!from.renameTo(to))
			{
				logger.error(nodeName + ": Unable to rename '" + rename.getFrom() + "' to '" + rename.getTo() + "'!");
				return false;
			}
		}
		
		logger.info(nodeName + ": Processing INSTALL items");
		
		for (InstallUnit install: node.getInstallList())
		{
			List<BaseURL> urls = install.getBaseURLList();
			for (String file: install.getFileList())
			{
				// attempt to install this file
				boolean succeeded = installOne(urls, file);
				if (!succeeded)
					return false;
			}
		}
		
		// TODO: hash lists
		
		return true;
	}
	
	private boolean installOne(List<BaseURL> baseURLList, String file)
	{
		logger.info(node.getName() + ": installing '" + file + "'");
		return true;
	}
	
	public void cancel()
	{
	}
}
