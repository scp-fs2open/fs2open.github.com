package com.fsoinstaller.gui;

import java.awt.GridLayout;
import java.util.ArrayList;
import javax.swing.*;

import com.fsoinstaller.main.FreeSpaceOpenInstaller;
import com.fsoinstaller.main.InstallConfig;
import com.fsoinstaller.util.InternetUtil;
import com.fsoinstaller.util.THFileIO;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.awt.event.*;

/**
 * A lot of this class was ripped straight out of SelectPage and OptionPage, so some of the naming is off.
 * @author Thomas
 *
 */
public class ConfigPage extends InstallerPage implements ActionListener {
	private JPanel configpanel, panel;
	private ArrayList<InstallConfig> configs;

	protected JPanel createPage() {
		panel = new JPanel();
		panel.setLayout(new GridLayout(1, 1));
		configpanel = new JPanel();
		configpanel.setLayout(new GridLayout(0, 1));
		JScrollPane scrollpane = new JScrollPane(configpanel);
		scrollpane.setAutoscrolls(true);
		scrollpane.getVerticalScrollBar().setUnitIncrement(12);
		panel.add(scrollpane);
		return panel;
	}

	public void activatePage() {
		configpanel.removeAll();
		this.GUI().enablePrev(true);
		this.GUI().enableNext(false);
		this.GUI().enableExit(true);
		DirPage dirpage = (DirPage) this.GUI().getPage(DirPage.class);
		String instdir = dirpage.instdir();
		(new File(instdir + "Installer" + File.separator)).mkdir();
		(new File(instdir + "temp" + File.separator)).mkdir();
		try {
			System.setErr(new PrintStream(new FileOutputStream(instdir + "installer" + File.separator + "installer_error.log.txt", false), true));
		}
		catch (Exception E){
			try {
				(new File(instdir + "installer" + File.separator + "installer_error.log.txt")).createNewFile();
				System.setErr(new PrintStream(new FileOutputStream(instdir + "installer" + File.separator + "installer_error.log.txt", false), true));
			}
			catch (Exception F){E.printStackTrace(); F.printStackTrace();}
		}
		InternetUtil.download(FreeSpaceOpenInstaller.INSTALLER_HOME_URL + "version.txt",
				instdir + "temp" + File.separator + "version.txt");
		THFileIO versiontxt = new THFileIO(instdir + "temp" + File.separator + "version.txt");
		String currentver = versiontxt.readFromFile();
		if (! FreeSpaceOpenInstaller.INSTALLER_VERSION.equals(currentver)){
			int n;
			Object[] options3 = {"Yes", "No"};
			n = JOptionPane.showOptionDialog(panel,
				"This version of the " + FreeSpaceOpenInstaller.INSTALLER_NAME + " is outdated, and you " +
				"can not continue with an outdated version.\nWould you like to bring up the " +
				"download page for the most recent version?\n(Note: If you do not have a working " +
				"internet connection, it will cause this to show. If this is the case, please " +
				"connect to the internet and start the " + FreeSpaceOpenInstaller.INSTALLER_NAME + " again.)",
				FreeSpaceOpenInstaller.INSTALLER_NAME,
				JOptionPane.YES_NO_OPTION,
				JOptionPane.QUESTION_MESSAGE,
				null,
				options3,
				options3[0]);
		
			if (n == JOptionPane.YES_OPTION) {
				InternetUtil.download(FreeSpaceOpenInstaller.INSTALLER_LATEST_VERSION_URL + "latest.txt",
				instdir + "temp" + File.separator + "latest.txt");
				THFileIO latesttxt = new THFileIO(instdir + "temp" + File.separator + "latest.txt");
				if (FreeSpaceOpenInstaller.INSTALLER_IS_JAR) {
					latesttxt.readFromFile();
				}
				InternetUtil.browseToURL(latesttxt.readFromFile());
				System.exit(0);
			}
			else if (n == JOptionPane.NO_OPTION)
			{
				System.exit(0);
			}
		}
		InternetUtil.download(FreeSpaceOpenInstaller.INSTALLER_HOME_URL
				+ "installconfigs.txt", instdir + "Installer" + File.separator
				+ "installconfigs.txt");
		THFileIO configstxt = new THFileIO(instdir + "Installer"
				+ File.separator + "installconfigs.txt");
		configs = new ArrayList<InstallConfig>();
		while (configstxt.hasNext()) {
			try {
			configs.add(new InstallConfig(configstxt));
			}catch (Exception E) {
				E.printStackTrace(System.out);
			}
		}
		for (int i = 0; i < configs.size(); i++) {
			configpanel.add(configs.get(i).getPanel());
		}
		this.GUI().enableNext(true);
	}

	private static String getFileName(String path) {
		return path.substring(path.lastIndexOf("/") + 1);
	}

	public ArrayList<InstallConfig> getConfigs() {
		return configs;
	}

	public void actionPerformed(ActionEvent e) {
		String name = e.getActionCommand();
		System.out.println(name);
		InstallConfig changed = null;
		for (int i = 0; i < configs.size(); i++) {
			if (configs.get(i).name().equals(name))
				changed = configs.get(i);
		}
	}
}