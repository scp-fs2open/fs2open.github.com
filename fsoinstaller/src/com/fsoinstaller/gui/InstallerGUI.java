package com.fsoinstaller.gui;

import javax.swing.*;

import com.fsoinstaller.main.FreeSpaceOpenInstaller;

import java.net.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;

public class InstallerGUI implements ActionListener {
	private ArrayList<InstallerPage> pages;
	private int pagenum;
	private JFrame frame;
	private JPanel nav, pagedisplay;
	private JButton next, prev, exit;
	private JProgressBar downloadprogress;
	
	public InstallerGUI()
	{
		this(null);
	}
	public InstallerGUI(Collection<InstallerPage> startingpages)
	{
		nav = new JPanel();
		nav.setBorder(BorderFactory.createLoweredBevelBorder());
		frame = new JFrame(FreeSpaceOpenInstaller.INSTALLER_NAME);
		frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		pagenum = 0;
		if (startingpages != null)
		{
			pages = new ArrayList<InstallerPage>(startingpages);
			for (int i = 0; i < pages.size(); i++)
				pages.get(i).setGUI(this);
		}
		else
		{
			pages = new ArrayList<InstallerPage>();
		}
		nav.setLayout(new GridBagLayout());
		frame.setLayout(new BorderLayout());
		createNav();
		createPageDisplay();
		URL top = null;
		URL lside = null;
		URL rside = null;
		String url = FreeSpaceOpenInstaller.INSTALLER_HOME_URL.substring(7);
		String server = url.substring(0, url.indexOf("/"));
		String path = url.substring(url.indexOf("/"));
		System.out.println(server);
		System.out.println(path);		
		try
		{
			top = new URL("http", server, path + "top.png");
			lside = new URL("http", server, path + "left.png");
			rside = new URL("http", server, path + "right.png");
		}
		catch (Exception E)
		{
			E.printStackTrace();
		}
		frame.getContentPane().add(new JLabel(new ImageIcon(lside)), BorderLayout.WEST);
		frame.getContentPane().add(new JLabel(new ImageIcon(rside)), BorderLayout.EAST);
		frame.getContentPane().add(new JLabel(new ImageIcon(top)), BorderLayout.NORTH);
		frame.getContentPane().add(pagedisplay, BorderLayout.CENTER);
		frame.getContentPane().add(nav, BorderLayout.SOUTH);
		frame.setResizable(false);
		frame.pack();
		centerWindowOnScreen(frame);
		frame.setVisible(true);
	}
	
	public static void centerWindowOnScreen(Container window)
	{
		// find the coordinates to center the whole window
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		int x = (int) ((screenSize.getWidth() - window.getWidth()) / 2);
		int y = (int) ((screenSize.getHeight() - window.getHeight()) / 2);

		// center it
		window.setLocation(x, y);
	}
	
	private void createNav()
	{
		JPanel panel2 = nav;
		nav = new JPanel();
		nav.setLayout(new BorderLayout());
		downloadprogress = new JProgressBar(0, 100);
		downloadprogress.setValue(0);
		downloadprogress.setStringPainted(true);
		InstallerSection.setProgressBar(downloadprogress);
		next = new JButton("Next");
		prev = new JButton("Prev");
		exit = new JButton("Exit");
		next.setActionCommand("next");
		prev.setActionCommand("prev");
		exit.setActionCommand("exit");
		next.addActionListener(this);
		prev.addActionListener(this);
		exit.addActionListener(this);
		pages.get(pagenum).getPanel();
		nav.removeAll();
		
		panel2.add(new JLabel("FreeSpace Open Installer Copyright 2006-2007 Thomas \"Turey\" Hall    "));
		panel2.add(prev);
		panel2.add(next);
		panel2.add(exit);
		nav.add(downloadprogress, BorderLayout.NORTH);
		nav.add(panel2);
		nav.setVisible(true);
	}
	public void addPage (InstallerPage newpage)
	{
		pages.add(newpage);
		int i = pages.size() - 1;
		pagedisplay.add(pages.get(i).getPanel(), pages.get(i).getName());
	}
	public void actionPerformed(ActionEvent e)
	{
		System.out.println("actionPerformed: " + e.getActionCommand());
        if ("exit".equals(e.getActionCommand())) {
            System.exit(0);
        } else {
        	if ("next".equals(e.getActionCommand()) && (pagenum < pages.size() - 1))
        	{
        		pagenum++;
                CardLayout cl = (CardLayout)(pagedisplay.getLayout());
                cl.show(pagedisplay, pages.get(pagenum).getName());
            	pages.get(pagenum).activatePage();
        	}
        	else if ("prev".equals(e.getActionCommand()) && (pagenum > 0))
        	{
        		pagenum--;
                CardLayout cl = (CardLayout)(pagedisplay.getLayout());
                cl.show(pagedisplay, pages.get(pagenum).getName());
            	pages.get(pagenum).activatePage();
        	}
        }
	}
	
    private void createPageDisplay()
    {
        pagedisplay = new JPanel(new CardLayout());
    	for (int i = 0; i < pages.size(); i++)
    	{
    		System.out.println(pages.get(i).getName());
    		pagedisplay.add(pages.get(i).getPanel(), pages.get(i).getName());
    	}
    	pages.get(pagenum).activatePage();
    }
    public void goNext()
    {
    	next.doClick();
    }
    public void goPrev()
    {
    	prev.doClick();
    }
    public void enableNext(boolean b)
    {
    	next.setEnabled(b);
    }
    public void enablePrev(boolean b)
    {
    	prev.setEnabled(b);
    }
    public void enableExit(boolean b)
    {
    	exit.setEnabled(b);
    }
    public void refresh()
    {
        CardLayout cl = (CardLayout)(pagedisplay.getLayout());
        cl.show(pagedisplay, pages.get(pagenum).getName());
    }
    public InstallerPage getPage(Class<? extends InstallerPage> e)
    {
    	for (int i = 0; i < pages.size(); i++)
    		if (e.isAssignableFrom(pages.get(i).getClass()))
    			return pages.get(i);
    	return null;
    }
}
