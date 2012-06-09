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

import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.UIManager;

import com.fsoinstaller.common.InstallerNode;
import com.fsoinstaller.main.Configuration;
import com.fsoinstaller.main.FreeSpaceOpenInstaller;
import com.fsoinstaller.utils.Logger;
import com.fsoinstaller.utils.MiscUtils;


public class ChoicePage extends WizardPage
{
	private static final Logger logger = Logger.getLogger(ChoicePage.class);
	
	private final ButtonGroup group;
	private final Map<InstallChoice, JRadioButton> buttons;
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
		
		// keep track of buttons
		buttons = new LinkedHashMap<InstallChoice, JRadioButton>();
		buttons.put(InstallChoice.BASIC, basic);
		buttons.put(InstallChoice.COMPLETE, complete);
		buttons.put(InstallChoice.CUSTOM, custom);
		
		inited = false;
	}
	
	// careful with this; it's called from the constructor
	private final JRadioButton createButton(final InstallChoice choice)
	{
		JRadioButton button = new JRadioButton(choice.getName());
		group.add(button);
		button.addActionListener(new ActionListener()
		{
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
		panel.setBorder(BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, 3 * GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, 3 * GUIConstants.DEFAULT_MARGIN));
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
		
		JLabel selectLabel = new JLabel("Please select a mode of installation.");
		selectLabel.setAlignmentX(LEFT_ALIGNMENT);
		panel.add(selectLabel);
		
		for (Map.Entry<InstallChoice, JRadioButton> entry: buttons.entrySet())
		{
			JPanel optionPanel = createOptionPanel(entry.getKey(), entry.getValue());
			optionPanel.setAlignmentX(LEFT_ALIGNMENT);
			
			panel.add(Box.createVerticalStrut(GUIConstants.DEFAULT_MARGIN));
			panel.add(optionPanel);
		}
		
		return panel;
	}
	
	private JPanel createOptionPanel(InstallChoice choice, JRadioButton button)
	{
		// bold for distinguishing
		button.setFont(button.getFont().deriveFont(Font.BOLD));
		
		// figure out how wide radio button icons are
		Icon icon = UIManager.getIcon("RadioButton.icon");
		int width = (icon != null) ? icon.getIconWidth() : 13;
		// add spacing around icon
		width += 2 * button.getIconTextGap();
		
		// for some annoying reason, creating a standard horizontal strut leads to vertical spacing!
		Box.Filler strictStrut = new Box.Filler(new Dimension(width, 0), new Dimension(width, 0), new Dimension(width, 0));
		
		JLabel detailLabel = new JLabel(choice.getDescription(), new ImageIcon(choice.getImage()), JLabel.LEFT);
		detailLabel.setIconTextGap(GUIConstants.DEFAULT_MARGIN);
		
		JPanel detailPanel = new JPanel();
		detailPanel.setLayout(new BoxLayout(detailPanel, BoxLayout.X_AXIS));
		detailPanel.add(strictStrut);
		detailPanel.add(detailLabel);
		
		JPanel panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
		button.setAlignmentX(LEFT_ALIGNMENT);
		panel.add(button);
		detailPanel.setAlignmentX(LEFT_ALIGNMENT);
		panel.add(detailPanel);
		
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
			logger.error("There are no mods available!  (And this should have been checked already!)");
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
