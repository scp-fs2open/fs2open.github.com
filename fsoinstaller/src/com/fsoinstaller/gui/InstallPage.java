package com.fsoinstaller.gui;

import java.awt.*;
import java.io.*;
import javax.swing.*;

import com.fsoinstaller.main.FreeSpaceOpenInstaller;
import com.fsoinstaller.util.THFileIO;

import java.util.*;

public class InstallPage extends InstallerPage {
	protected JPanel createPage()
	{
		JPanel panel = new JPanel();
		panel.setLayout(new GridLayout(1,1));
		JScrollPane scrollpane = new JScrollPane(FreeSpaceOpenInstaller.INSTALL_STATUS.getTextArea());
		scrollpane.setAutoscrolls(true);
		scrollpane.getVerticalScrollBar().setUnitIncrement(24);
		panel.add(scrollpane);
		panel.setBorder(BorderFactory.createRaisedBevelBorder());
		return panel;
	}
	public void activatePage()
	{
		this.GUI().enablePrev(false);
		this.GUI().enableNext(false);
		this.GUI().enableExit(true);
		try{
        new Thread(new Runnable() {
            public void run() {
                install();
            }
        }).start();
		}
		catch(Exception e){}
	}
	private void install()
	{
		SelectPage selectpage = (SelectPage) this.GUI().getPage(SelectPage.class);
		DirPage dirpage = (DirPage) this.GUI().getPage(DirPage.class);
		ArrayList<InstallerSection> toInstall = selectpage.getSections();
		String instdir = dirpage.instdir();
		THFileIO versiontrack = InstallerSection.versiontrack();
		versiontrack.finishWriting();
		versiontrack.returnToStart();
		(new File(instdir + "Installer" + File.separator + "installedversions.txt")).delete();
		THFileIO destination = new THFileIO(instdir + "Installer" + File.separator + "installedversions.txt");
		while (versiontrack.hasNext())
			destination.writeToFile(versiontrack.readFromFile());
		destination.finishWriting();
		versiontrack = new THFileIO(instdir + "Installer" + File.separator + "installedversions.txt", true);
		InstallerSection.versiontrack(versiontrack);
		for (InstallerSection currentSection : toInstall){
			FreeSpaceOpenInstaller.INSTALL_STATUS.append("Starting section: " + currentSection.name() + "\n");
			this.GUI().refresh();
			currentSection.update();
			versiontrack.finishWriting();
			versiontrack.restartWriting();
		}
		InstallerSection.getProgressBar().setIndeterminate(false);
		InstallerSection.getProgressBar().setMinimum(0);
		InstallerSection.getProgressBar().setMaximum(100);
		InstallerSection.getProgressBar().setValue(100);
		InstallerSection.getProgressBar().setString("Done.");
		this.GUI().enableNext(true);
		this.GUI().enableExit(false);
		FreeSpaceOpenInstaller.INSTALL_STATUS.append("Install Finished.\n");		
	}
}
