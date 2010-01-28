package com.fsoinstaller.gui;

import java.awt.GridLayout;
import java.io.File;
import java.util.*;
import javax.swing.*;

import com.fsoinstaller.main.FreeSpaceOpenInstaller;
import com.fsoinstaller.util.InternetUtil;
import com.fsoinstaller.util.THFileIO;


public class ExitPage extends InstallerPage {
	private JScrollPane installscroll;
	
	public JPanel createPage()
	{
		JPanel panel = new JPanel();
		panel.setLayout(new GridLayout(1,1));
		JScrollPane scrollpane = new JScrollPane(FreeSpaceOpenInstaller.INSTALL_NOTES.getTextArea());
		scrollpane.setAutoscrolls(true);
		scrollpane.getVerticalScrollBar().setUnitIncrement(12);
		panel.add(scrollpane);
		return panel;
	}
	public void activatePage()
	{
		// get the supported ids for GMT-08:00 (Pacific Standard Time)
		String[] ids = TimeZone.getAvailableIDs(-8 * 60 * 60 * 1000);
		// if no ids were returned, something is wrong. get out.
		if (ids.length == 0)
			System.exit(0);
		// create a Pacific Standard Time time zone
		SimpleTimeZone pdt = new SimpleTimeZone(-8 * 60 * 60 * 1000, ids[0]);
		// set up rules for daylight savings time
		pdt.setStartRule(Calendar.APRIL, 1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
		pdt.setEndRule(Calendar.OCTOBER, -1, Calendar.SUNDAY, 2 * 60 * 60 * 1000);
		// create a GregorianCalendar with the Pacific Daylight time zone
		// and the current date and time
		Calendar calendar = new GregorianCalendar(pdt);
		Date now = new Date();
		calendar.setTime(now);
		// print out a bunch of interesting things
		String day = String.valueOf(calendar.get(Calendar.DAY_OF_MONTH));
		day = (day.length() < 2) ? "0" + day : day;
		String month = String.valueOf(calendar.get(Calendar.MONTH) + 1);
		month = (month.length() < 2) ? "0" + month : month;
		String hour = String.valueOf(calendar.get(Calendar.HOUR_OF_DAY));
		hour = (hour.length() < 2) ? "0" + hour : hour;
		String min = String.valueOf(calendar.get(Calendar.MINUTE));
		min = (min.length() < 2) ? "0" + min : min;
		String sec = String.valueOf(calendar.get(Calendar.SECOND));
		sec = (sec.length() < 2) ? "0" + sec : sec;
		String nowstring = calendar.get(Calendar.YEAR) + "_" + month + "_" + day + "_" +
							hour + "_" + min + "_" + sec + "_" +
							calendar.get(Calendar.MILLISECOND);
		this.GUI().enablePrev(false);
		this.GUI().enableNext(false);
		this.GUI().enableExit(true);
		DirPage dirpage = (DirPage) this.GUI().getPage(DirPage.class);
		String instdir = dirpage.instdir();
		InternetUtil.download(FreeSpaceOpenInstaller.INSTALLER_HOME_URL + "endnote.txt",
				instdir + "Installer" + File.separator + "endnote.txt");
		THFileIO endnote = new THFileIO(instdir + "Installer" + File.separator + "endnote.txt");
		String installnotes = endnote.readFromFile();
		while(endnote.hasNext())
			installnotes += endnote.readFromFile() + "\n";
		installnotes += "\n\n\n" + FreeSpaceOpenInstaller.INSTALL_NOTES.getTextArea().getText();
		THFileIO savednote = new THFileIO(instdir + "Installer" + File.separator + "install_notes_" + nowstring + ".txt");
		savednote.writeToFile(installnotes);
		savednote.finishWriting();
		installnotes += "In case you ever need to re-read this information, it has been saved" +
		" as " + instdir + "Installer" + File.separator + "install_notes_" + nowstring + ".txt";
		FreeSpaceOpenInstaller.INSTALL_NOTES.getTextArea().setText(installnotes);
	}
}
