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

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.List;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import com.fsoinstaller.common.InstallerNode;
import com.fsoinstaller.main.Configuration;
import com.fsoinstaller.main.FreeSpaceOpenInstaller;
import com.fsoinstaller.utils.Logger;
import com.fsoinstaller.utils.MiscUtils;


public class ChoicePage extends InstallerPage
{
	private static final Logger logger = Logger.getLogger(ChoicePage.class);
	
	private final ButtonGroup group;
	private final JRadioButton basic;
	private final JRadioButton complete;
	private final JRadioButton custom;
	
	private boolean inited;
	
	public ChoicePage()
	{
		super("choice");
		
		// create widgets
		group = new ButtonGroup();
		basic = createButton(InstallChoice.BASIC);
		complete = createButton(InstallChoice.COMPLETE);
		custom = createButton(InstallChoice.CUSTOM);
		
		inited = false;
	}
	
	private JRadioButton createButton(final InstallChoice choice)
	{
		JRadioButton button = new JRadioButton(choice.getName());
		group.add(button);
		button.addActionListener(new ActionListener()
		{
			@Override
			public void actionPerformed(ActionEvent e)
			{
				nextButton.setEnabled(true);
				Configuration.getInstance().getSettings().put(Configuration.INSTALL_CHOICE_KEY, choice);
			}
		});
		return button;
	}
	
	@Override
	public JPanel createCenterPanel()
	{
		JPanel panel = new JPanel();
		panel.setBorder(BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN));
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
		panel.add(new JLabel("Please select a mode of installation."));
		
		panel.add(basic);
		panel.add(complete);
		panel.add(custom);
		
		return panel;
	}
	
	@Override
	public void prepareForDisplay()
	{
		// take care of correct choice state, including whether anything has been chosen yet
		Map<String, Object> settings = Configuration.getInstance().getSettings();
		InstallChoice choice = (InstallChoice) settings.get(Configuration.INSTALL_CHOICE_KEY);
		if (choice == null)
			nextButton.setEnabled(false);
		else
		{
			switch (choice)
			{
				case BASIC:
					group.setSelected(basic.getModel(), true);
					break;
				case COMPLETE:
					group.setSelected(complete.getModel(), true);
					break;
				case CUSTOM:
					group.setSelected(custom.getModel(), true);
					break;
			}
		}
		
		if (inited)
			return;
		
		logger.info("Validating choice configuration...");
		
		@SuppressWarnings("unchecked")
		List<String> basicMods = (List<String>) settings.get(Configuration.BASIC_CONFIG_MODS_KEY);
		@SuppressWarnings("unchecked")
		List<InstallerNode> modNodes = (List<InstallerNode>) settings.get(Configuration.MOD_NODES_KEY);
		if (modNodes.isEmpty())
		{
			logger.error("There are no mods available!  (And this should have been checked already!");
			return;
		}
		
		// check BASIC config
		if (basicMods == null || basicMods.isEmpty())
		{
			basic.setEnabled(false);
			JOptionPane.showMessageDialog(MiscUtils.getActiveFrame(), "The Basic Installation configuration could not be retrieved from the installer website.  This option will not be available.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.WARNING_MESSAGE);
		}
		else
		{
			// be sure that all BASIC mods are present
			for (String mod: basicMods)
			{
				// be sure that the mod can be found in the node list
				boolean found = false;
				for (InstallerNode node: modNodes)
				{
					if (node.findInTree(mod) != null)
					{
						found = true;
						break;
					}
				}
				
				if (!found)
				{
					basic.setEnabled(false);
					JOptionPane.showMessageDialog(MiscUtils.getActiveFrame(), "The Basic Installation configuration could not be validated against the current list of mods.  This option will not be available.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.WARNING_MESSAGE);
					break;
				}
			}
		}
		
		logger.info("Validation complete");
		
		inited = true;
	}
	
	@Override
	public void prepareToLeavePage(Runnable runWhenReady)
	{
		nextButton.setEnabled(true);
		runWhenReady.run();
	}
}
