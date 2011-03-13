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
import java.awt.image.BufferedImage;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;

import com.fsoinstaller.utils.GraphicsUtils;
import com.fsoinstaller.utils.MiscUtils;


public abstract class InstallerPage extends JPanel
{
	private static final BufferedImage banner = GraphicsUtils.getResourceImage("top.png");
	
	protected InstallerGUI gui;
	protected final Configuration configuration;
	
	protected final JButton backButton;
	protected final JButton nextButton;
	protected final JButton cancelButton;
	
	public InstallerPage(String name)
	{
		this.configuration = Configuration.getInstance();
		
		backButton = new JButton(new BackAction());
		nextButton = new JButton(new NextAction());
		cancelButton = new JButton(new CancelAction());
		
		setName(name);
	}
	
	final void setGUI(InstallerGUI gui)
	{
		this.gui = gui;
	}
	
	/**
	 * This should be called immediately after the object is constructed, so
	 * that the appropriate widgets are added in the appropriate way. This is
	 * done to avoid inheritance problems when a subclassed panel is working
	 * with widgets that should have been previously created in a superclass
	 * panel.
	 */
	public void buildUI()
	{
		setLayout(new BorderLayout());
		add(createHeaderPanel(), BorderLayout.NORTH);
		add(createCenterPanel(), BorderLayout.CENTER);
		add(createFooterPanel(), BorderLayout.SOUTH);
	}
	
	public JPanel createHeaderPanel()
	{
		return new ImagePanel(banner);
	}
	
	public abstract JPanel createCenterPanel();
	
	/**
	 * This method will be called immediately before the page is shown to the
	 * user or brought to the foreground.
	 */
	public abstract void prepareForDisplay();
	
	/**
	 * This method will be called immediately before the page is about to be
	 * changed. This method should return a boolean value to indicate whether
	 * the change may proceed.
	 */
	public abstract boolean prepareToLeavePage();
	
	public JPanel createFooterPanel()
	{
		JPanel separatorPanel = new JPanel(new BorderLayout());
		separatorPanel.setBorder(BorderFactory.createEmptyBorder(0, GUIConstants.DEFAULT_MARGIN, 0, GUIConstants.DEFAULT_MARGIN));
		separatorPanel.add(new JSeparator(), BorderLayout.CENTER);
		
		JPanel buttonPanel = new JPanel();
		buttonPanel.setBorder(BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN));
		buttonPanel.setLayout(new BoxLayout(buttonPanel, BoxLayout.X_AXIS));
		buttonPanel.add(new JLabel("\u00A9 2006-2011 The FreeSpace 2 Source Code Project"));
		buttonPanel.add(Box.createHorizontalGlue());
		buttonPanel.add(backButton);
		buttonPanel.add(nextButton);
		buttonPanel.add(Box.createHorizontalStrut(GUIConstants.DEFAULT_MARGIN));
		buttonPanel.add(cancelButton);
		
		JPanel footer = new JPanel(new BorderLayout());
		footer.add(separatorPanel, BorderLayout.NORTH);
		footer.add(buttonPanel, BorderLayout.CENTER);
		
		return footer;
	}
	
	private final class BackAction extends AbstractAction
	{
		public BackAction()
		{
			putValue(Action.NAME, "< Back");
			putValue(Action.SHORT_DESCRIPTION, "Go to the previous page");
		}
		
		@Override
		public void actionPerformed(ActionEvent e)
		{
			if (prepareToLeavePage())
				gui.moveBack();
		}
	}
	
	private final class NextAction extends AbstractAction
	{
		public NextAction()
		{
			putValue(Action.NAME, "Next >");
			putValue(Action.SHORT_DESCRIPTION, "Go to the next page");
		}
		
		@Override
		public void actionPerformed(ActionEvent e)
		{
			if (prepareToLeavePage())
				gui.moveNext();
		}
	}
	
	private final class CancelAction extends AbstractAction
	{
		public CancelAction()
		{
			putValue(Action.NAME, "Cancel");
			putValue(Action.SHORT_DESCRIPTION, "Cancel installation");
		}
		
		@Override
		public void actionPerformed(ActionEvent e)
		{
			JFrame frame = (JFrame) MiscUtils.getActiveFrame();
			frame.dispose();
		}
	}
}
