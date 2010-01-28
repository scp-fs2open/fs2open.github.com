package com.fsoinstaller.main;

import java.util.LinkedList;
import java.util.List;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JCheckBox;
import javax.swing.JPanel;

import com.fsoinstaller.gui.ConfigPage;
import com.fsoinstaller.util.THFileIO;


public class InstallConfig {
	private List<String> sections = new LinkedList<String>();
	private String name;
	private JCheckBox selectbox;
	
	public InstallConfig(THFileIO configfile) {
		if (configfile.readFromFile().equals("NAME")) {
			name = configfile.readFromFile();
		}
		String line = configfile.readFromFile();
		while (!line.equals("END")) {
			sections.add(line);
			line = configfile.readFromFile();
		}
		selectbox = new JCheckBox(name);
		ConfigPage configpage = (ConfigPage) FreeSpaceOpenInstaller.GUI().getPage(ConfigPage.class);
		selectbox.addActionListener(configpage);
		selectbox.setActionCommand(name);
	}
	
	public String name() {
		return name;
	}
	
	public List<String> getSections() {
		return sections;
	}
	
	public JPanel getPanel()
	{
		JPanel panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
		panel.add(selectbox);
		panel.add(Box.createHorizontalGlue());
		return panel;
	}
	
	public boolean isSelected()
	{
		return selectbox.isSelected();		
	}
}
