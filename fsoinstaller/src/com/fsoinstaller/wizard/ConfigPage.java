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
import java.awt.event.ActionEvent;
import java.io.File;
import java.io.IOException;
import java.net.Proxy;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.fsoinstaller.internet.Connector;
import com.fsoinstaller.internet.InvalidProxyException;
import com.fsoinstaller.utils.Logger;
import com.fsoinstaller.utils.MiscUtils;
import com.l2fprod.common.swing.JDirectoryChooser;


public class ConfigPage extends InstallerPage
{
	private static final Logger logger = Logger.getLogger(ConfigPage.class);
	
	private final JTextField directoryField;
	private final JTextField hostField;
	private final JTextField portField;
	private boolean usingProxy;
	
	public ConfigPage()
	{
		super("config");
		
		// load initial directory
		String dirText = "";
		File dir = configuration.getApplicationDir();
		if (dir != null)
		{
			try
			{
				dirText = dir.getCanonicalPath();
			}
			catch (IOException ioe)
			{
				logger.warn("Could not get canonical path of destination directory", ioe);
				dirText = dir.getAbsolutePath();
			}
		}
		
		// load initial proxy settings
		String host = configuration.getProxyHost();
		int port = configuration.getProxyPort();
		usingProxy = (host != null && port >= 0);
		
		// create widgets
		directoryField = new JTextField(dirText);
		hostField = new JTextField(usingProxy ? host : "none");
		portField = new JTextField(usingProxy ? Integer.toString(port) : "none");
		hostField.setEnabled(usingProxy);
		portField.setEnabled(usingProxy);
	}
	
	@Override
	public JPanel createCenterPanel()
	{
		JLabel dummy = new JLabel();
		
		// bleh, for multiline we need a JTextArea, but we want it to look like a JLabel
		JTextArea text = new JTextArea("Choose the directory where you would like to install FreeSpace 2 Open and associated mods.  If your network requires the use of a proxy, you can also specify that here.");
		text.setEditable(false);
		text.setRows(4);
		text.setOpaque(false);
		text.setHighlighter(null);
		text.setFont(dummy.getFont());
		text.setLineWrap(true);
		text.setWrapStyleWord(true);
		
		JPanel dirPanel = new JPanel();
		dirPanel.setBorder(BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN));
		dirPanel.setLayout(new BoxLayout(dirPanel, BoxLayout.X_AXIS));
		dirPanel.add(directoryField);
		dirPanel.add(Box.createHorizontalStrut(GUIConstants.DEFAULT_MARGIN));
		dirPanel.add(new JButton(new BrowseAction()));
		
		JPanel outerDirPanel = new JPanel(new BorderLayout());
		outerDirPanel.setBorder(BorderFactory.createTitledBorder("Installation Directory"));
		outerDirPanel.add(dirPanel, BorderLayout.CENTER);
		
		JCheckBox check = new JCheckBox(new ProxyCheckAction());
		check.setSelected(usingProxy);
		JLabel hostLabel = new JLabel("Proxy host:");
		JLabel portLabel = new JLabel("Proxy port:");
		int m_width = (int) Math.max(hostLabel.getMinimumSize().getWidth(), portLabel.getMinimumSize().getWidth());
		int p_width = (int) Math.max(hostLabel.getPreferredSize().getWidth(), portLabel.getPreferredSize().getWidth());
		hostLabel.setMinimumSize(new Dimension(m_width, (int) hostLabel.getMinimumSize().getHeight()));
		portLabel.setMinimumSize(new Dimension(m_width, (int) portLabel.getMinimumSize().getHeight()));
		hostLabel.setPreferredSize(new Dimension(p_width, (int) hostLabel.getPreferredSize().getHeight()));
		portLabel.setPreferredSize(new Dimension(p_width, (int) portLabel.getPreferredSize().getHeight()));
		
		JPanel checkPanel = new JPanel();
		checkPanel.setLayout(new BoxLayout(checkPanel, BoxLayout.X_AXIS));
		checkPanel.add(check);
		checkPanel.add(Box.createHorizontalGlue());
		
		JPanel hostPanel = new JPanel();
		hostPanel.setLayout(new BoxLayout(hostPanel, BoxLayout.X_AXIS));
		hostPanel.add(Box.createHorizontalStrut(GUIConstants.DEFAULT_MARGIN * 3));
		hostPanel.add(hostLabel);
		hostPanel.add(Box.createHorizontalStrut(GUIConstants.DEFAULT_MARGIN));
		hostPanel.add(hostField);
		
		JPanel portPanel = new JPanel();
		portPanel.setLayout(new BoxLayout(portPanel, BoxLayout.X_AXIS));
		portPanel.add(Box.createHorizontalStrut(GUIConstants.DEFAULT_MARGIN * 3));
		portPanel.add(portLabel);
		portPanel.add(Box.createHorizontalStrut(GUIConstants.DEFAULT_MARGIN));
		portPanel.add(portField);
		
		JPanel proxyPanel = new JPanel();
		proxyPanel.setBorder(BorderFactory.createEmptyBorder(0, GUIConstants.SMALL_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN));
		proxyPanel.setLayout(new BoxLayout(proxyPanel, BoxLayout.Y_AXIS));
		proxyPanel.add(checkPanel);
		proxyPanel.add(Box.createVerticalStrut(GUIConstants.SMALL_MARGIN));
		proxyPanel.add(hostPanel);
		proxyPanel.add(Box.createVerticalStrut(GUIConstants.SMALL_MARGIN));
		proxyPanel.add(portPanel);
		
		JPanel outerProxyPanel = new JPanel(new BorderLayout());
		outerProxyPanel.setBorder(BorderFactory.createTitledBorder("Proxy Settings"));
		outerProxyPanel.add(proxyPanel, BorderLayout.CENTER);
		
		JPanel panel = new JPanel();
		panel.setBorder(BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN));
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
		panel.add(text);
		panel.add(Box.createVerticalStrut(GUIConstants.DEFAULT_MARGIN));
		panel.add(outerDirPanel);
		panel.add(Box.createVerticalStrut(GUIConstants.DEFAULT_MARGIN));
		panel.add(outerProxyPanel);
		panel.add(Box.createVerticalStrut(GUIConstants.DEFAULT_MARGIN));
		
		return panel;
	}
	
	@Override
	public void prepareForDisplay()
	{
		backButton.setVisible(false);
	}
	
	@Override
	public boolean prepareToLeavePage()
	{
		logger.info("Validating user input...");
		
		// check directory
		File destinationDir = MiscUtils.validateApplicationDir(directoryField.getText());
		if (destinationDir == null)
		{
			JOptionPane.showMessageDialog(MiscUtils.getActiveFrame(), "The destination directory is not valid.  Please select another directory.", "FreeSpace Open Installer", JOptionPane.WARNING_MESSAGE);
			return false;
		}
		
		// ditto
		if (!destinationDir.exists())
		{
			// prompt to create it
			int result = JOptionPane.showConfirmDialog(MiscUtils.getActiveFrame(), "The destination directory does not exist.  Do you want to create it?", "FreeSpace Open Installer", JOptionPane.YES_NO_OPTION);
			if (result != JOptionPane.YES_OPTION)
				return false;
			
			logger.info("Attempting to create directory/ies...");
			
			// attempt to create it
			if (!destinationDir.mkdirs())
			{
				JOptionPane.showMessageDialog(MiscUtils.getActiveFrame(), "Could not create the destination directory.  Please select another directory.", "FreeSpace Open Installer", JOptionPane.ERROR_MESSAGE);
				return false;
			}
		}
		
		// check proxy
		Proxy proxy = null;
		String host = null;
		int port = -1;
		if (usingProxy)
		{
			logger.info("Checking proxy...");
			
			try
			{
				host = hostField.getText();
				port = Integer.valueOf(portField.getText());
				proxy = Connector.createProxy(host, port);
			}
			catch (NumberFormatException nfe)
			{
				JOptionPane.showMessageDialog(MiscUtils.getActiveFrame(), "The proxy port could not be parsed as an integer.  Please enter a correct proxy port.", "FreeSpace Open Installer", JOptionPane.WARNING_MESSAGE);
				return false;
			}
			catch (InvalidProxyException ipe)
			{
				logger.error("Proxy could not be created!", ipe);
				JOptionPane.showMessageDialog(MiscUtils.getActiveFrame(), "This proxy appears to be invalid!  Check that you have entered the host and port correctly.", "FreeSpace Open Installer", JOptionPane.ERROR_MESSAGE);
				return false;
			}
			
			// good to go
			configuration.getSettings().put(Configuration.PROXY_KEY, proxy);
		}
		
		logger.info("Validation succeeded!");
		
		// save our settings
		configuration.setApplicationDir(destinationDir);
		configuration.setProxyInfo(host, port);
		configuration.saveProperties();
		
		return true;
	}
	
	private final class BrowseAction extends AbstractAction
	{
		public BrowseAction()
		{
			putValue(Action.NAME, "Browse...");
			putValue(Action.SHORT_DESCRIPTION, "Click to choose an installation directory");
		}
		
		@Override
		public void actionPerformed(ActionEvent e)
		{
			File dir = MiscUtils.validateApplicationDir(directoryField.getText());
			
			// create a file chooser
			JDirectoryChooser chooser = new JDirectoryChooser();
			chooser.setCurrentDirectory(dir);
			chooser.setDialogTitle("Choose a directory");
			chooser.setShowingCreateDirectory(false);
			
			// display it
			int result = chooser.showDialog(MiscUtils.getActiveFrame(), "OK");
			if (result == JDirectoryChooser.APPROVE_OPTION)
				directoryField.setText(chooser.getSelectedFile().getAbsolutePath());
		}
	}
	
	private final class ProxyCheckAction extends AbstractAction
	{
		public ProxyCheckAction()
		{
			putValue(Action.NAME, "Use proxy");
		}
		
		@Override
		public void actionPerformed(ActionEvent e)
		{
			usingProxy = !usingProxy;
			
			hostField.setEnabled(usingProxy);
			portField.setEnabled(usingProxy);
		}
	}
}
