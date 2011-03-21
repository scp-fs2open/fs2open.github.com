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

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;


public class ChoicePage extends InstallerPage
{
	private final ButtonGroup group;
	private final JRadioButton basic;
	private final JRadioButton complete;
	private final JRadioButton custom;
	private final JRadioButton update;
	
	public ChoicePage()
	{
		super("choice");
		
		// create widgets
		group = new ButtonGroup();
		basic = createButton("Basic", "basic.png", "Install the latest FreeSpace 2 Open and MediaVPs, but no mods.");
		complete = createButton("Complete", "complete.png", "Install everything: FreeSpace 2 Open, the MediaVPs, all mods, and all optional downloads.");
		custom = createButton("Custom", "custom.png", "");
		update = null;
	}
	
	private static JRadioButton createButton(String name, String image, String description)
	{
		return new JRadioButton(name);
	}
	
	@Override
	public JPanel createCenterPanel()
	{
		JPanel panel = new JPanel();
		panel.setBorder(BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN));
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
		panel.add(new JLabel("Please select a mode of installation."));
		
		panel.add(new JRadioButton("Basic - Install the latest FreeSpace 2 Open and MediaVPs, but no mods"));
		panel.add(new JRadioButton("Complete - Everything, including all mods"));
		panel.add(new JRadioButton("Custom - Choose the mods to install"));
		panel.add(new JRadioButton("Update Only - Just update the mods that are already installed"));
		
		return panel;
	}
	
	@Override
	public void prepareForDisplay()
	{
	}
	
	@Override
	public void prepareToLeavePage(Runnable runWhenReady)
	{
		runWhenReady.run();
	}
}
