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

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;

import com.fsoinstaller.utils.Logger;


public class ModSelectPage extends InstallerPage
{
	private static final Logger logger = Logger.getLogger(ModSelectPage.class);
	
	private final JRadioButton basicButton;
	private final JRadioButton completeButton;
	private final JRadioButton customButton;
	private final JPanel modPanel;
	
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
	}
	
	@Override
	public JPanel createCenterPanel()
	{
		JPanel labelPanel = new JPanel();
		labelPanel.setLayout(new BoxLayout(labelPanel, BoxLayout.X_AXIS));
		labelPanel.add(new JLabel("Select the items you would like to install."));
		labelPanel.add(Box.createHorizontalGlue());
		
		JPanel quickSelectPanel = new JPanel();
		quickSelectPanel.setLayout(new BoxLayout(quickSelectPanel, BoxLayout.Y_AXIS));
		quickSelectPanel.add(basicButton);
		quickSelectPanel.add(completeButton);
		quickSelectPanel.add(customButton);
		
		JPanel outerQuickSelectPanel = new JPanel(new BorderLayout());
		outerQuickSelectPanel.setBorder(BorderFactory.createTitledBorder("Quick Select"));
		outerQuickSelectPanel.add(quickSelectPanel, BorderLayout.CENTER);
		
		JPanel topPanel = new JPanel();
		topPanel.setLayout(new BoxLayout(topPanel, BoxLayout.Y_AXIS));
		topPanel.add(labelPanel);
		topPanel.add(Box.createVerticalStrut(GUIConstants.DEFAULT_MARGIN));
		topPanel.add(outerQuickSelectPanel);
		topPanel.add(Box.createVerticalStrut(GUIConstants.DEFAULT_MARGIN));
		
		JScrollPane modScrollPane = new JScrollPane(modPanel, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
		
		JPanel panel = new JPanel(new BorderLayout());
		panel.setBorder(BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN));
		panel.add(topPanel, BorderLayout.NORTH);
		panel.add(modScrollPane, BorderLayout.CENTER);
		
		basicButton.doClick();
		
		return panel;
	}
	
	@Override
	public void prepareForDisplay()
	{
		@SuppressWarnings("unchecked")
		List<String> urls = (List<String>) Configuration.getInstance().getSettings().get(Configuration.MOD_URLs);
		if (urls == null)
			logger.warn("D'oh!  No URLs!");
		else
		{
			logger.info("mod URLs:");
			for (String url: urls)
				logger.info(url);
		}
		
		modPanel.removeAll();
		modPanel.add(new SingleModPanel("FSPort"));
		modPanel.add(new SingleModPanel("MediaVPs"));
		modPanel.add(new SingleModPanel("woot"));
		modPanel.add(Box.createVerticalGlue());
	}
	
	@Override
	public void prepareToLeavePage(Runnable runWhenReady)
	{
		runWhenReady.run();
	}
	
	private static class SingleModPanel extends JPanel
	{
		public SingleModPanel(String name)
		{
			setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
			add(new JCheckBox(name));
			add(Box.createHorizontalGlue());
			add(new JButton("More Info"));
		}
	}
}
