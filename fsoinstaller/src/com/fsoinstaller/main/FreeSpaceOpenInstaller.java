
package com.fsoinstaller.main;

import java.io.File;
import java.util.LinkedList;
import java.util.List;

import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;

import org.apache.log4j.Logger;

import com.fsoinstaller.gui.ConfigPage;
import com.fsoinstaller.gui.DirPage;
import com.fsoinstaller.gui.ExitPage;
import com.fsoinstaller.gui.InstallPage;
import com.fsoinstaller.gui.InstallerGUI;
import com.fsoinstaller.gui.InstallerPage;
import com.fsoinstaller.gui.IntroPage;
import com.fsoinstaller.gui.SelectPage;
import com.fsoinstaller.util.InternetUtil;
import com.fsoinstaller.util.THFileIO;
import com.fsoinstaller.widgets.LoggingArea;


public class FreeSpaceOpenInstaller
{
	private static final Logger logger = Logger.getLogger(FreeSpaceOpenInstaller.class);

	/**
	 * Name of this Installer.
	 */
	public static final String INSTALLER_NAME = "FreeSpace Open Installer";

	/**
	 * Default directory to install to.
	 */
	public static final String INSTALLER_DEFAULT_DIR = "/Games/FreeSpace2/";

	/**
	 * URL of the latest version of this Installer.
	 */
	public static final String INSTALLER_LATEST_VERSION_URL = "http://www.fsoinstaller.com/files/installer/java/";

	/**
	 * URL of the directory where version.txt, filenames.txt, and endnote.txt reside.
	 */
	public static final String INSTALLER_HOME_URL = "http://www.fsoinstaller.com/files/installer/java/";

	/**
	 * Version of the Installer.
	 */
	public static final String INSTALLER_VERSION = "3.6.9 Config";

	/**
	 * Whether this is the JAR version (as opposed to the EXE version).
	 */
	public static final boolean INSTALLER_IS_JAR = true;

	/**
	 * Widget to keep track of installer status messages (downloads and so forth).
	 */
	public static final LoggingArea INSTALL_STATUS = new LoggingArea("InstallStatus", 20, 50);

	/**
	 * Widget to keep track of installer notes for post-installation "debriefing".
	 */
	public static final LoggingArea INSTALL_NOTES = new LoggingArea("InstallNotes", 20, 50);

	private static final String proxyfile = "proxy.txt";
	private static InstallerGUI gui = null;

	public static void main(String[] args)
	{
		logger.debug("Checking for proxy information...");
		if ((new File(proxyfile)).exists())
			getProxyInfo(proxyfile);

		logger.debug("Setting look-and-feel...");
		try
		{
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		}
		catch (ClassNotFoundException cnfe)
		{
			logger.error(cnfe);
		}
		catch (InstantiationException ie)
		{
			logger.error(ie);
		}
		catch (IllegalAccessException iae)
		{
			logger.error(iae);
		}
		catch (UnsupportedLookAndFeelException iae)
		{
			logger.error(iae);
		}

		logger.debug("Creating installer pages...");
		List<InstallerPage> pages = new LinkedList<InstallerPage>();
		pages.add(new IntroPage());
		pages.add(new DirPage());
		pages.add(new ConfigPage());
		pages.add(new SelectPage());
		pages.add(new InstallPage());
		pages.add(new ExitPage());

		logger.debug("Launching installer GUI");
		gui = new InstallerGUI(pages);
		gui.launch();
	}

	public static InstallerGUI GUI()
	{
		return gui;
	}

	private static void getProxyInfo(String filename)
	{
		THFileIO proxyfile = new THFileIO(filename);
		System.getProperties().put("proxySet", "true");
		String host = proxyfile.readFromFile();
		System.getProperties().put("proxyHost", host.substring(0, host.indexOf(":")));
		System.getProperties().put("proxyPort", host.substring(host.indexOf(":") + 1));
		if (proxyfile.hasNext())
			InternetUtil.setProxyPassword(proxyfile.readFromFile());

		logger.info("Using proxy: " + System.getProperties().getProperty("proxyHost") + ":" + System.getProperty("proxyPort"));
	}
}
