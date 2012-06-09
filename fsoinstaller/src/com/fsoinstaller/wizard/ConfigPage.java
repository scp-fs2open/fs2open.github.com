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
import java.awt.EventQueue;
import java.awt.event.ActionEvent;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.Proxy;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.Callable;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.fsoinstaller.common.InstallerNode;
import com.fsoinstaller.common.InstallerNodeFactory;
import com.fsoinstaller.common.InstallerNodeParseException;
import com.fsoinstaller.internet.Connector;
import com.fsoinstaller.internet.Downloader;
import com.fsoinstaller.internet.InvalidProxyException;
import com.fsoinstaller.main.Configuration;
import com.fsoinstaller.main.FreeSpaceOpenInstaller;
import com.fsoinstaller.utils.Logger;
import com.fsoinstaller.utils.MiscUtils;
import com.fsoinstaller.utils.ProgressBarDialog;
import com.fsoinstaller.utils.ThreadSafeJOptionPane;
import com.l2fprod.common.swing.JDirectoryChooser;


public class ConfigPage extends WizardPage
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
		
		// they shouldn't change vertical size
		directoryField.setMaximumSize(new Dimension((int) directoryField.getMaximumSize().getWidth(), (int) directoryField.getPreferredSize().getHeight()));
		hostField.setMaximumSize(new Dimension((int) hostField.getMaximumSize().getWidth(), (int) hostField.getPreferredSize().getHeight()));
		portField.setMaximumSize(new Dimension((int) portField.getMaximumSize().getWidth(), (int) portField.getPreferredSize().getHeight()));
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
		
		JPanel outerDirPanel = new JPanel();
		outerDirPanel.setBorder(BorderFactory.createTitledBorder("Installation Directory"));
		outerDirPanel.setLayout(new BoxLayout(outerDirPanel, BoxLayout.Y_AXIS));
		outerDirPanel.add(dirPanel);
		
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
		
		JPanel outerProxyPanel = new JPanel();
		outerProxyPanel.setBorder(BorderFactory.createTitledBorder("Proxy Settings"));
		outerProxyPanel.setLayout(new BoxLayout(outerProxyPanel, BoxLayout.Y_AXIS));
		outerProxyPanel.add(proxyPanel);
		
		JPanel panel = new JPanel();
		panel.setBorder(BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN));
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
		panel.add(text);
		panel.add(Box.createVerticalStrut(GUIConstants.DEFAULT_MARGIN * 2));
		panel.add(outerDirPanel);
		panel.add(Box.createVerticalStrut(GUIConstants.DEFAULT_MARGIN * 2));
		panel.add(outerProxyPanel);
		panel.add(Box.createVerticalStrut(GUIConstants.DEFAULT_MARGIN * 2));
		panel.add(Box.createVerticalGlue());
		
		return panel;
	}
	
	@Override
	public void prepareForDisplay()
	{
		backButton.setVisible(false);
	}
	
	@Override
	public void prepareToLeavePage(Runnable runWhenReady)
	{
		Callable<Void> task = new SuperValidationTask((JFrame) MiscUtils.getActiveFrame(), directoryField.getText(), usingProxy, hostField.getText(), portField.getText(), runWhenReady, new Runnable()
		{
			@Override
			public void run()
			{
				MiscUtils.getActiveFrame().dispose();
			}
		});
		
		ProgressBarDialog dialog = new ProgressBarDialog("Accessing installer information...");
		dialog.runTask(task, null);
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
	
	private static final class SuperValidationTask implements Callable<Void>
	{
		private final JFrame activeFrame;
		private final String directoryText;
		private final boolean usingProxy;
		private final String hostText;
		private final String portText;
		private final Runnable runWhenReady;
		private final Runnable exitRunnable;
		
		private final Configuration configuration;
		private final Map<String, Object> settings;
		
		public SuperValidationTask(JFrame activeFrame, String directoryText, boolean usingProxy, String hostText, String portText, Runnable runWhenReady, Runnable exitRunnable)
		{
			this.activeFrame = activeFrame;
			this.directoryText = directoryText;
			this.usingProxy = usingProxy;
			this.hostText = hostText;
			this.portText = portText;
			this.runWhenReady = runWhenReady;
			this.exitRunnable = exitRunnable;
			
			// Configuration and its two maps are thread-safe
			this.configuration = Configuration.getInstance();
			this.settings = configuration.getSettings();
		}
		
		public Void call()
		{
			logger.info("Validating user input...");
			
			// check directory
			File destinationDir = MiscUtils.validateApplicationDir(directoryText);
			if (destinationDir == null)
			{
				ThreadSafeJOptionPane.showMessageDialog(activeFrame, "The destination directory is not valid.  Please select another directory.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.WARNING_MESSAGE);
				return null;
			}
			
			// ditto
			if (!destinationDir.exists())
			{
				// prompt to create it
				int result = ThreadSafeJOptionPane.showConfirmDialog(activeFrame, "The destination directory does not exist.  Do you want to create it?", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.YES_NO_OPTION);
				if (result != JOptionPane.YES_OPTION)
					return null;
				
				logger.info("Attempting to create directory/ies...");
				
				// attempt to create it
				if (!destinationDir.mkdirs())
				{
					ThreadSafeJOptionPane.showMessageDialog(activeFrame, "Could not create the destination directory.  Please select another directory.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.ERROR_MESSAGE);
					return null;
				}
				
				logger.info("Directory creation successful.");
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
					host = hostText;
					port = Integer.valueOf(portText);
					proxy = Connector.createProxy(host, port);
				}
				catch (NumberFormatException nfe)
				{
					ThreadSafeJOptionPane.showMessageDialog(activeFrame, "The proxy port could not be parsed as an integer.  Please enter a correct proxy port.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.WARNING_MESSAGE);
					return null;
				}
				catch (InvalidProxyException ipe)
				{
					logger.error("Proxy could not be created!", ipe);
					ThreadSafeJOptionPane.showMessageDialog(activeFrame, "This proxy appears to be invalid!  Check that you have entered the host and port correctly.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.ERROR_MESSAGE);
					return null;
				}
				
				// good to go
				settings.put(Configuration.PROXY_KEY, proxy);
			}
			
			logger.info("Validation succeeded!");
			
			// save our settings
			configuration.setApplicationDir(destinationDir);
			configuration.setProxyInfo(host, port);
			configuration.saveUserProperties();
			
			Connector connector = new Connector(proxy);
			settings.put(Configuration.CONNECTOR_KEY, connector);
			
			// only check for the installer version if we haven't checked already
			if (!settings.containsKey(Configuration.REMOTE_VERSION_KEY))
			{
				logger.info("Checking installer version...");
				
				File tempVersion;
				File tempFilenames;
				File tempBasicConfig;
				try
				{
					tempVersion = File.createTempFile("fsoinstaller_version", null);
					tempFilenames = File.createTempFile("fsoinstaller_filenames", null);
					tempBasicConfig = File.createTempFile("fsoinstaller_basicconfig", null);
				}
				catch (IOException ioe)
				{
					logger.error("Error creating temporary file!", ioe);
					ThreadSafeJOptionPane.showMessageDialog(activeFrame, "There was an error creating a temporary file!  This application may need elevated privileges to run.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.ERROR_MESSAGE);
					return null;
				}
				tempVersion.deleteOnExit();
				tempFilenames.deleteOnExit();
				tempBasicConfig.deleteOnExit();
				
				double maxVersion = -1.0;
				String maxVersionURL = null;
				
				// check all URLs for version and filename info
				for (String url: FreeSpaceOpenInstaller.INSTALLER_HOME_URLs)
				{
					logger.debug("Accessing version info from " + url + "...");
					
					// assemble URLs
					URL versionURL;
					URL filenameURL;
					URL basicURL;
					try
					{
						versionURL = new URL(url + "version.txt");
						filenameURL = new URL(url + "filenames.txt");
						basicURL = new URL(url + "basic_config.txt");
					}
					catch (MalformedURLException murle)
					{
						logger.error("Something went wrong with the URL!", murle);
						continue;
					}
					
					// download version information
					Downloader tempVersionDownloader = new Downloader(connector, versionURL, tempVersion);
					if (tempVersionDownloader.download())
					{
						List<String> versionLines = MiscUtils.readTextFile(tempVersion);
						if (!versionLines.isEmpty())
						{
							double thisVersion;
							try
							{
								thisVersion = Double.valueOf(versionLines.get(0));
							}
							catch (NumberFormatException nfe)
							{
								thisVersion = 0.0;
							}
							
							logger.info("Version at this URL is " + thisVersion);
							
							// get the information from the highest version available
							if (thisVersion > maxVersion)
							{
								// get file names
								Downloader tempFilenamesDownloader = new Downloader(connector, filenameURL, tempFilenames);
								if (tempFilenamesDownloader.download())
								{
									List<String> filenameLines = MiscUtils.readTextFile(tempFilenames);
									if (!filenameLines.isEmpty())
									{
										settings.put(Configuration.REMOTE_VERSION_KEY, thisVersion);
										settings.put(Configuration.MOD_URLS_KEY, filenameLines);
										
										maxVersion = thisVersion;
										maxVersionURL = versionLines.get(1);
										
										// try to get basic configuration too, but this can be optional (sort of)
										Downloader tempBasicConfigDownloader = new Downloader(connector, basicURL, tempBasicConfig);
										if (tempBasicConfigDownloader.download())
										{
											List<String> basicLines = MiscUtils.readTextFile(tempBasicConfig);
											
											// strip empty/blank lines
											Iterator<String> ii = basicLines.iterator();
											while (ii.hasNext())
												if (ii.next().trim().isEmpty())
													ii.remove();
											
											if (!basicLines.isEmpty())
												settings.put(Configuration.BASIC_CONFIG_MODS_KEY, basicLines);
										}
									}
								}
							}
						}
					}
				}
				
				// make sure we could access version information
				if (!settings.containsKey(Configuration.REMOTE_VERSION_KEY))
				{
					ThreadSafeJOptionPane.showMessageDialog(activeFrame, "There was a problem accessing the remote sites.  Check your network connection and try again.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.WARNING_MESSAGE);
					return null;
				}
				
				// we have a version; check if it is more recent than what we're running
				// (this prompt should only ever come up once, because once the version is known, future visits to this page will take the early exit above)
				if (maxVersion > FreeSpaceOpenInstaller.INSTALLER_VERSION)
				{
					int result = ThreadSafeJOptionPane.showConfirmDialog(activeFrame, "This version of the installer is out-of-date.  Would you like to bring up the download page for the most recent version?\n\n(If you click Yes, the program will exit.)", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.YES_NO_OPTION);
					if (result == JOptionPane.YES_OPTION)
					{
						try
						{
							if (connector.browseToURL(new URL(maxVersionURL)))
							{
								// this should close the program
								EventQueue.invokeLater(exitRunnable);
								return null;
							}
						}
						catch (MalformedURLException murle)
						{
							logger.error("Something went wrong with the URL!", murle);
						}
						ThreadSafeJOptionPane.showMessageDialog(activeFrame, "There was a problem bringing up the download link.  Try re-downloading the installer using your favorite Internet browser.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.ERROR_MESSAGE);
						return null;
					}
				}
			}
			
			// only check for mod information if we haven't checked already
			if (!settings.containsKey(Configuration.MOD_NODES_KEY))
			{
				logger.info("Downloading mod information...");
				
				@SuppressWarnings("unchecked")
				List<String> urls = (List<String>) Configuration.getInstance().getSettings().get(Configuration.MOD_URLS_KEY);
				if (urls == null || urls.isEmpty())
				{
					ThreadSafeJOptionPane.showMessageDialog(activeFrame, "For some reason, there are no mods available for download.  This is not an error with the network, but rather with the remote mod repositories.  It shouldn't ever happen, and we're rather perplexed that you're seeing this right now.  We can only suggest that you try again later.\n\nClick OK to exit.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.ERROR_MESSAGE);
					EventQueue.invokeLater(exitRunnable);
					return null;
				}
				
				// parse mod urls into nodes
				List<InstallerNode> modNodes = new ArrayList<InstallerNode>();
				for (String url: urls)
				{
					// create a URL
					URL modURL;
					try
					{
						modURL = new URL(url);
					}
					catch (MalformedURLException murle)
					{
						logger.error("Something went wrong with the URL!", murle);
						continue;
					}
					
					// create a temporary file
					File tempModFile;
					try
					{
						tempModFile = File.createTempFile("fsoinstaller_mod", null);
					}
					catch (IOException ioe)
					{
						logger.error("Error creating temporary file!", ioe);
						ThreadSafeJOptionPane.showMessageDialog(activeFrame, "There was an error creating a temporary file!  This application may need elevated privileges to run.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.ERROR_MESSAGE);
						return null;
					}
					tempModFile.deleteOnExit();
					
					// download it to the temp file
					Downloader tempModFileDownloader = new Downloader(connector, modURL, tempModFile);
					if (!tempModFileDownloader.download())
					{
						logger.warn("Could not download mod information from '" + url + "'");
						continue;
					}
					
					// parse it into one or more nodes
					InstallerNode node;
					try
					{
						FileReader reader = new FileReader(tempModFile);
						while (true)
						{
							node = InstallerNodeFactory.readNode(reader);
							if (node == null)
								break;
							
							modNodes.add(node);
							logger.info("Successfully added " + node.getName());
						}
					}
					catch (FileNotFoundException fnfe)
					{
						logger.error("This is very odd; we can't find the temp file we just created!", fnfe);
					}
					catch (IOException ioe)
					{
						logger.error("This is very odd; there was an error reading the temp file we just created!", ioe);
					}
					catch (InstallerNodeParseException inpe)
					{
						logger.warn("There was an error parsing the mod file at '" + url + "'", inpe);
					}
				}
				
				// check that we have mods
				if (modNodes.isEmpty())
				{
					ThreadSafeJOptionPane.showMessageDialog(activeFrame, "For some reason, there are no mods available for download.  This is not an error with the network, but rather with the remote mod repositories.  It shouldn't ever happen, and we're rather perplexed that you're seeing this right now.  We can only suggest that you try again later.\n\nClick OK to exit.", FreeSpaceOpenInstaller.INSTALLER_TITLE, JOptionPane.ERROR_MESSAGE);
					EventQueue.invokeLater(exitRunnable);
					return null;
				}
				
				// add to settings
				settings.put(Configuration.MOD_NODES_KEY, modNodes);
			}
			
			// now that we have the mod list, check to see if there is any old version information left over from Turey's installer
			logger.info("Checking for legacy version information...");
			
			// we check this every time because the user could have changed the destination directory
			// (however, the installed versions file should be deleted after the first try, or at least the properties file should contain the keys we need)
			try
			{
				File oldInstallerInfoDir = new File(destinationDir, "temp");
				if (oldInstallerInfoDir.exists() && oldInstallerInfoDir.isDirectory())
				{
					File installedversions = new File(oldInstallerInfoDir, "installedversions.txt");
					if (installedversions.exists())
					{
						// read lines from the installedversions file
						List<String> lines = MiscUtils.readTextFile(installedversions);
						
						// load the version for each node
						@SuppressWarnings("unchecked")
						List<InstallerNode> modNodes = (List<InstallerNode>) settings.get(Configuration.MOD_NODES_KEY);
						for (InstallerNode node: modNodes)
							loadLegacyModVersions(node, lines, configuration.getUserProperties());
						
						// save our properties
						configuration.saveUserProperties();
						
						// delete the file, since we don't need it any more
						installedversions.delete();
					}
					
					// delete other old files and the folder
					File latest = new File(oldInstallerInfoDir, "latest.txt");
					if (latest.exists())
						latest.delete();
					File version = new File(oldInstallerInfoDir, "version.txt");
					if (version.exists())
						version.delete();
					File[] filesLeft = oldInstallerInfoDir.listFiles();
					if (filesLeft.length == 0)
						oldInstallerInfoDir.delete();
				}
			}
			catch (SecurityException se)
			{
				logger.warn("A SecurityException was thrown when checking for legacy version information!", se);
			}
			
			// validation completed!
			logger.info("Done with SuperValidationTask!");
			EventQueue.invokeLater(runWhenReady);
			return null;
		}
		
		private void loadLegacyModVersions(InstallerNode node, List<String> installedversions_lines, Properties properties)
		{
			String propertyName = node.buildTreeName();
			
			// if we have a version already, we don't need to query the legacy version
			logger.debug(propertyName);
			if (properties.containsKey(propertyName))
				return;
			
			// find the version corresponding to this node in the installedversions file
			String version = null;
			Iterator<String> ii = installedversions_lines.iterator();
			while (ii.hasNext())
			{
				// get name matching this node
				if (!ii.next().equalsIgnoreCase("NAME"))
					continue;
				if (!ii.hasNext())
					break;
				if (!ii.next().equals(node.getName()))
					continue;
				// ensure name hasn't provided a version
				if (version != null)
				{
					logger.warn("The installedversions file contains more than one version for the name '" + node.getName() + "'!");
					return;
				}
				
				// get version
				if (!ii.hasNext())
					break;
				if (!ii.next().equalsIgnoreCase("VERSION"))
					continue;
				if (!ii.hasNext())
					break;
				version = ii.next();
			}
			
			// now that we have a version, save it
			if (version != null)
				properties.setProperty(propertyName, version);
			
			// we need to check all the child nodes as well
			for (InstallerNode child: node.getChildren())
				loadLegacyModVersions(child, installedversions_lines, properties);
		}
	}
}
