package com.fsoinstaller.gui;

import java.awt.GridLayout;
import javax.swing.JPanel;
import javax.swing.JTextArea;

import com.fsoinstaller.main.FreeSpaceOpenInstaller;


public class IntroPage extends InstallerPage {
	protected JPanel createPage()
	{
		JPanel panel = new JPanel();
		panel.setLayout(new GridLayout(1,1));
		JTextArea text = new JTextArea("You are about to use the " + FreeSpaceOpenInstaller.INSTALLER_NAME + ". Make sure that your internet connection is working and that your firewall isn't blocking this application.", 20, 50);
		text.setLineWrap(true);
		text.setWrapStyleWord(true);
		text.setEditable(false);
		panel.add(text);
		return panel;
	}
	public void activatePage()
	{
		this.GUI().enablePrev(false);
		this.GUI().enableNext(true);
		this.GUI().enableExit(true);
	}
}
