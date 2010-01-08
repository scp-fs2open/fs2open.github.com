package com.fsoinstaller.gui;

import javax.swing.*;


public abstract class InstallerPage {
	private JPanel page;
	private InstallerGUI gui;
	
	public InstallerPage()
	{
		page = createPage();
	}
	
	public InstallerGUI GUI()
	{
		return gui;
	}

	public void setGUI(InstallerGUI newgui)
	{
		gui = newgui;		
	}

	public JPanel getPanel()
	{
		return page;
	}
	
	public abstract void activatePage();

	public final String getName()
	{
		return this.getClass().getName() + "(" + System.identityHashCode(this) + ")";
	}
	
	protected abstract JPanel createPage();
}