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
import java.awt.EventQueue;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ScrollPaneConstants;

import com.fsoinstaller.common.InstallerNode;
import com.fsoinstaller.main.Configuration;
import com.fsoinstaller.utils.Logger;


public class InstallPage extends WizardPage
{
	private static final Logger logger = Logger.getLogger(ModSelectPage.class);
	
	private final JPanel installPanel;
	
	public InstallPage()
	{
		super("install");
		
		// create widgets
		installPanel = new JPanel();
		installPanel.setLayout(new BoxLayout(installPanel, BoxLayout.Y_AXIS));
	}
	
	@Override
	public JPanel createCenterPanel()
	{
		JPanel labelPanel = new JPanel();
		labelPanel.setLayout(new BoxLayout(labelPanel, BoxLayout.X_AXIS));
		labelPanel.add(new JLabel("Installing..."));
		labelPanel.add(Box.createHorizontalGlue());
		
		JScrollPane installScrollPane = new JScrollPane(installPanel, ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS, ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
		
		JPanel panel = new JPanel(new BorderLayout(0, GUIConstants.DEFAULT_MARGIN));
		panel.setBorder(BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN));
		panel.add(labelPanel, BorderLayout.NORTH);
		panel.add(installScrollPane, BorderLayout.CENTER);
		
		return panel;
	}
	
	@Override
	public void prepareForDisplay()
	{
		backButton.setVisible(false);
		nextButton.setVisible(false);
		
		Map<String, Object> settings = configuration.getSettings();
		@SuppressWarnings("unchecked")
		List<InstallerNode> modNodes = (List<InstallerNode>) settings.get(Configuration.MOD_NODES_KEY);
		@SuppressWarnings("unchecked")
		Set<String> selectedMods = (Set<String>) settings.get(Configuration.MODS_TO_INSTALL_KEY);
		
		// log the mods
		logger.info("Selected mods:");
		for (String mod: selectedMods)
			logger.info(mod);
		
		// prepare to bring out the big guns: create an ExecutorService
		// IMPLEMENTATION DETAIL: since tasks are queued from the event thread, we need to use an implementation that never blocks on adding a task
		ExecutorService exec = Executors.newCachedThreadPool();
		
		
		// OK HERE'S HOW IT WORKS
		
		// submit an InstallItem for an individual node
		// each InstallItem is basically its own task or thread
		// if that node's future returns true (meaning success) then submit InstallItems for its child nodes		
		
		for (InstallerNode node: modNodes)
		{
			if (!selectedMods.contains(node.getName()))
				continue;
			
			InstallItem item = new InstallItem(node);
			installPanel.add(item);
			Future<Boolean> f = exec.submit(item);
			// TODO: do something with F
		}
		
		// TODO: note that the ExecutorService isn't going to terminate automatically, so we need to shut it down properly in success or failure
		
	}
	
	@Override
	public void prepareToLeavePage(Runnable runWhenReady)
	{
		// TODO
		//runWhenReady.run();
	}
}
