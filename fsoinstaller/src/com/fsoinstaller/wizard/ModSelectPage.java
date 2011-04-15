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
import java.awt.event.ActionEvent;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.swing.AbstractAction;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;

import com.fsoinstaller.common.InstallerNode;
import com.fsoinstaller.main.Configuration;
import com.fsoinstaller.utils.Logger;
import com.fsoinstaller.utils.MiscUtils;


public class ModSelectPage extends InstallerPage
{
	private static final Logger logger = Logger.getLogger(ModSelectPage.class);
	
	private final JRadioButton basicButton;
	private final JRadioButton completeButton;
	private final JRadioButton customButton;
	private final JPanel modPanel;
	
	private final List<InstallerNode> treeWalk;
	private boolean inited;
	
	public ModSelectPage()
	{
		super("mod-select");
		
		// create widgets
		basicButton = new JRadioButton("Basic - FreeSpace 2 Open and MediaVPs, but no mods");
		completeButton = new JRadioButton("Complete - Everything, including all mods");
		customButton = new JRadioButton("Custom - Choose the mods to install");
		modPanel = new JPanel();
		modPanel.setLayout(new BoxLayout(modPanel, BoxLayout.Y_AXIS));
		
		// group them
		ButtonGroup group = new ButtonGroup();
		group.add(basicButton);
		group.add(completeButton);
		group.add(customButton);
		
		treeWalk = new ArrayList<InstallerNode>();
		inited = false;
	}
	
	@Override
	public JPanel createCenterPanel()
	{
		JPanel labelPanel = new JPanel();
		labelPanel.setLayout(new BoxLayout(labelPanel, BoxLayout.X_AXIS));
		labelPanel.add(new JLabel("You can modify your installation here, or continue with your current selection."));
		labelPanel.add(Box.createHorizontalGlue());
		
		JScrollPane modScrollPane = new JScrollPane(modPanel, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
		
		JPanel panel = new JPanel(new BorderLayout(0, GUIConstants.DEFAULT_MARGIN));
		panel.setBorder(BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN));
		panel.add(labelPanel, BorderLayout.NORTH);
		panel.add(modScrollPane, BorderLayout.CENTER);
		
		basicButton.doClick();
		
		return panel;
	}
	
	@Override
	public void prepareForDisplay()
	{
		setNextButton("Install", "Proceed with installation");
		
		Map<String, Object> settings = Configuration.getInstance().getSettings();
		@SuppressWarnings("unchecked")
		List<InstallerNode> modNodes = (List<InstallerNode>) settings.get(Configuration.MOD_NODES_KEY);
		
		// populating the mod panel only needs to be done once
		if (!inited)
		{
			if (modNodes.isEmpty())
			{
				logger.error("There are no mods available!  (And this should have been checked already!)");
				return;
			}
			
			// populate the mod panel
			modPanel.removeAll();
			treeWalk.clear();
			for (InstallerNode node: modNodes)
				addTreeNode(node, 0);
			modPanel.add(Box.createVerticalGlue());
			
			inited = true;
		}
		
		// select applicable nodes
		InstallChoice choice = (InstallChoice) settings.get(Configuration.INSTALL_CHOICE_KEY);
		if (choice == InstallChoice.BASIC)
		{
			@SuppressWarnings("unchecked")
			List<String> basicMods = (List<String>) settings.get(Configuration.BASIC_CONFIG_MODS_KEY);
			
			for (InstallerNode node: treeWalk)
				((SingleModPanel) node.getUserObject()).setSelected(basicMods.contains(node.getName()) && MiscUtils.validForOS(node.getName()));
		}
		else if (choice == InstallChoice.COMPLETE)
		{
			for (InstallerNode node: treeWalk)
				((SingleModPanel) node.getUserObject()).setSelected(MiscUtils.validForOS(node.getName()));
		}
		
		// TODO: select and disable nodes that have been installed already
	}
	
	private void addTreeNode(InstallerNode node, int depth)
	{
		SingleModPanel panel = new SingleModPanel(node, depth);
		node.setUserObject(panel);
		treeWalk.add(node);
		
		modPanel.add(panel);
		for (InstallerNode child: node.getChildren())
			addTreeNode(child, depth + 1);
	}
	
	@Override
	public void prepareToLeavePage(Runnable runWhenReady)
	{
		resetNextButton();
		runWhenReady.run();
	}
	
	private static class SingleModPanel extends JPanel
	{
		private final InstallerNode node;
		private final JCheckBox checkBox;
		private final JButton button;
		
		public SingleModPanel(InstallerNode node, int depth)
		{
			this.node = node;
			this.checkBox = createCheckBox(node);
			this.button = createMoreInfoButton(node);
			
			// set up layout
			setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
			for (int i = 0; i < depth; i++)
				add(Box.createHorizontalStrut(15));
			add(checkBox);
			add(Box.createHorizontalGlue());
			add(button);
		}
		
		public InstallerNode getNode()
		{
			return node;
		}
		
		public boolean isSelected()
		{
			return checkBox.isSelected();
		}
		
		public void setSelected(boolean selected)
		{
			checkBox.setSelected(selected);
		}
		
		private static JCheckBox createCheckBox(final InstallerNode node)
		{
			JCheckBox checkBox = new JCheckBox(new AbstractAction()
			{
				{
					putValue(AbstractAction.NAME, node.getName());
				}
				
				@Override
				public void actionPerformed(ActionEvent e)
				{
					// we want to automatically select or deselect any children
					setSubTreeState(node, ((JCheckBox) e.getSource()).isSelected());
					
					// changing the selection means the installation is now custom
					Configuration.getInstance().getSettings().put(Configuration.INSTALL_CHOICE_KEY, InstallChoice.CUSTOM);
				}
				
				private void setSubTreeState(InstallerNode root, boolean selected)
				{
					// this check exists because we don't want to doubly-set the node we're on
					if (root != node)
						((SingleModPanel) root.getUserObject()).setSelected(selected);
					
					// iterate through the tree
					for (InstallerNode child: root.getChildren())
						setSubTreeState(child, selected);
				}
			});
			return checkBox;
		}
		
		private static JButton createMoreInfoButton(final InstallerNode node)
		{
			JButton button = new JButton(new AbstractAction()
			{
				{
					putValue(AbstractAction.NAME, "More Info");
					putValue(AbstractAction.SHORT_DESCRIPTION, "Click to display additional information about this mod");
				}
				
				@Override
				public void actionPerformed(ActionEvent e)
				{
					JOptionPane.showMessageDialog(MiscUtils.getActiveFrame(), node.getDescription(), node.getName(), JOptionPane.INFORMATION_MESSAGE);
				}
			});
			
			if (node.getDescription() == null || node.getDescription().isEmpty())
				button.setEnabled(false);
			
			return button;
		}
	}
}
