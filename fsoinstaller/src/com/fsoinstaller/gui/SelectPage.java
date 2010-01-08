package com.fsoinstaller.gui;

import java.awt.GridLayout;
import java.util.ArrayList;
import javax.swing.*;

import com.fsoinstaller.main.FreeSpaceOpenInstaller;
import com.fsoinstaller.main.InstallConfig;
import com.fsoinstaller.util.InternetUtil;
import com.fsoinstaller.util.THFileIO;

import java.io.File;
import java.awt.event.*;

public class SelectPage extends InstallerPage implements ActionListener {
	private JPanel sectionspanel, panel;
	private ArrayList<InstallerSection> sections;

	protected JPanel createPage() {
		panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
		sectionspanel = new JPanel();
		sectionspanel.setLayout(new GridLayout(0, 1));
		JScrollPane scrollpane = new JScrollPane(sectionspanel);
		scrollpane.setAutoscrolls(true);
		scrollpane.getVerticalScrollBar().setUnitIncrement(12);
		panel
				.add(new JLabel(
						"You can modify your installation here, or continue without modification."));
		panel.add(scrollpane);
		return panel;
	}

	public void activatePage() {
		sectionspanel.removeAll();
		this.GUI().enablePrev(true);
		this.GUI().enableNext(false);
		this.GUI().enableExit(true);
		DirPage dirpage = (DirPage) this.GUI().getPage(DirPage.class);
		ConfigPage configpage = (ConfigPage) this.GUI().getPage(
				ConfigPage.class);
		String instdir = dirpage.instdir();
		InternetUtil.download(FreeSpaceOpenInstaller.INSTALLER_HOME_URL
				+ "filenames.txt", instdir + "Installer" + File.separator
				+ "filenames.txt");
		THFileIO filenamestxt = new THFileIO(instdir + "Installer"
				+ File.separator + "filenames.txt");
		(new File(instdir + "temp" + File.separator + "installedversions.txt"))
				.delete();
		THFileIO versiontrack = new THFileIO(instdir + "temp" + File.separator
				+ "installedversions.txt");
		InstallerSection.versiontrack(versiontrack);
		boolean installedOnly = false;
		boolean auto = false;
		sections = new ArrayList<InstallerSection>();
		for (InstallConfig ic : configpage.getConfigs()) {
			if (ic.isSelected()) {
				for (String s : ic.getSections()) {
					if (s.equals("ALL")) {
						auto = true;
					} else if (s.equals("UPDATE")) {
						installedOnly = true;
					}
				}
			}
		}
		while (filenamestxt.hasNext()) {
			String txtfileurl = filenamestxt.readFromFile();
			String filename = getFileName(txtfileurl);
			try {
				InternetUtil.download(txtfileurl, instdir
						+ "Installer" + File.separator + filename);
				sections.addAll(new InstallerSection(instdir, filename)
						.getSections(auto, installedOnly));
			} catch (Exception e) {
				int selected = JOptionPane
						.showConfirmDialog(
								sectionspanel,
								"A config file could not be downloaded.\n("
										+ txtfileurl
										+ ")\n\nDo you want to continue without the missing sections?\n"
										+ "The installation might not work, but missing "
										+ "files can be downloaded when the resource has been fixed.",
								"Config file download failed.",
								JOptionPane.YES_NO_OPTION,
								JOptionPane.QUESTION_MESSAGE);
				if (selected == JOptionPane.YES_OPTION)
					continue;
				else
					System.exit(0);
			}
		}
		for (InstallConfig ic : configpage.getConfigs()) {
			if (ic.isSelected()) {
				for (String s : ic.getSections()) {
					boolean foundSection = false;
					if (!s.equals("ALL") && !s.equals("UPDATE")) {
						for (InstallerSection section : sections) {
							if (s.equals(section.name())) {
								section.setSelected(foundSection = true);
							}
						}
					} else {
						foundSection = true;
					}
					if (!foundSection) {
						int selected = JOptionPane
								.showConfirmDialog(
										sectionspanel,
										"A section specified in the install config could not be found.\n("
												+ s
												+ ")\n\nDo you want to continue without the missing section?\n"
												+ "The installation might not work, but missing "
												+ "files can be downloaded when the resource has been fixed.",
										"Section find failed.",
										JOptionPane.YES_NO_OPTION,
										JOptionPane.QUESTION_MESSAGE);
						if (selected == JOptionPane.YES_OPTION)
							continue;
						else
							System.exit(0);
					}
				}
			}
		}
		for (int i = 0; i < sections.size(); i++) {
			sectionspanel.add(sections.get(i).getPanel());
		}
		this.GUI().enableNext(true);
	}

	private static String getFileName(String path) {
		return path.substring(path.lastIndexOf("/") + 1);
	}

	public ArrayList<InstallerSection> getSections() {
		return sections;
	}

	public void actionPerformed(ActionEvent e) {
		String name = e.getActionCommand();
		System.out.println(name);
		InstallerSection changed = null;
		for (int i = 0; i < sections.size(); i++) {
			if (sections.get(i).name().equals(name))
				changed = sections.get(i);
		}
		if (changed != null)
			fixTree(changed);
	}

	private void fixTree(InstallerSection changed) {
		if (changed.parent() != null && !changed.parent().isTreeSelected())
			changed.setSelected(false);
		for (int i = 0; i < sections.size(); i++) {
			InstallerSection current = sections.get(i);
			if (current.parent() != null
					&& current.parent().name().equals(changed.name())
					&& !changed.isSelected()) {
				System.out.println(current.name());
				current.setSelected(false);
				fixTree(current);
			}
		}
	}
}