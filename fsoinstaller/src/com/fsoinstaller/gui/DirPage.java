package com.fsoinstaller.gui;

import javax.swing.*;

import com.fsoinstaller.main.FreeSpaceOpenInstaller;

import java.awt.*;
import java.awt.event.*;
import java.io.File;

public class DirPage extends InstallerPage implements ActionListener{
	private JButton dirchoose;
	private String instdir = FreeSpaceOpenInstaller.INSTALLER_DEFAULT_DIR;
	
	protected JPanel createPage()
	{
		JPanel panel = new JPanel();
		panel.setLayout(new BorderLayout());
		panel.add(new JLabel("Press the button to choose a directory to install to."), BorderLayout.NORTH);
		panel.add(new JLabel("You can only continue if you choose an existing directory."), BorderLayout.SOUTH);
		dirchoose = new JButton();
		dirchoose.setText(instdir);
		dirchoose.setActionCommand("GETDIR");
		dirchoose.addActionListener(this);
		panel.add(dirchoose, BorderLayout.CENTER);
		return panel;
	}
	
	public void actionPerformed(ActionEvent e)
	{
		if ("GETDIR".equals(e.getActionCommand()))
		{
            JFileChooser fc = new JFileChooser();
            fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
            fc.showOpenDialog(getPanel());
            File file = fc.getSelectedFile();
            if (file.getAbsolutePath() != null)
            	instdir = file.getAbsolutePath();
            if (instdir.charAt(instdir.length() - 1) != File.separatorChar)
            	instdir += File.separator;
            dirchoose.setText(instdir);
    		this.GUI().enableNext((new File(instdir)).isDirectory());
		}
	}
	
	public void activatePage()
	{
		this.GUI().enablePrev(true);
		this.GUI().enableNext((new File(instdir)).isDirectory());
		this.GUI().enableExit(true);
		dirchoose.setText(instdir);
	}

	public void GUI(InstallerGUI newgui)
	{
		super.setGUI(newgui);
	}
	
	public String instdir()
	{
		return instdir;
	}

}
