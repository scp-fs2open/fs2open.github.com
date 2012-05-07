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
import java.util.List;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ScrollPaneConstants;

import com.fsoinstaller.common.InstallerNode;
import com.fsoinstaller.main.Configuration;


public class InstallPage extends WizardPage
{
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
		
		Map<String, Object> settings = Configuration.getInstance().getSettings();
		@SuppressWarnings("unchecked")
		List<InstallerNode> installNodes = (List<InstallerNode>) settings.get(Configuration.NODES_TO_INSTALL_KEY);
		
		for (InstallerNode node: installNodes)
		{
			JPanel labelPanel = new JPanel();
			labelPanel.setLayout(new BoxLayout(labelPanel, BoxLayout.X_AXIS));
			labelPanel.add(new JLabel(node.getName()));
			labelPanel.add(Box.createHorizontalGlue());
			
			installPanel.add(labelPanel);
		}
	}
	
	@Override
	public void prepareToLeavePage(Runnable runWhenReady)
	{
		runWhenReady.run();
	}
}
