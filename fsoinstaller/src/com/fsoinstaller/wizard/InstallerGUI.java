/*
 * This file is part of the FreeSpace Open Installer
 * Copyright (C) 2010 The FreeSpace 2 Source Code Project
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

package com.fsoinstaller.wizard;

import java.awt.CardLayout;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.net.URL;

import javax.imageio.ImageIO;
import javax.swing.JComponent;
import javax.swing.JFrame;

import com.fsoinstaller.utils.Logger;
import com.fsoinstaller.utils.MiscUtils;


public class InstallerGUI extends JFrame
{
	private static final Logger logger = Logger.getLogger(InstallerGUI.class);
	
	private static final BufferedImage app_icon;
	static
	{
		BufferedImage temp = null;
		try
		{
			URL url = MiscUtils.getResourceURL("resources/fso_icon.png");
			if (url != null)
				temp = ImageIO.read(url);
		}
		catch (IOException ioe)
		{
			logger.error("Could not read banner image", ioe);
		}
		app_icon = temp;
	}
	
	public InstallerGUI()
	{
		JComponent contentPane = (JComponent) getContentPane();
		contentPane.setLayout(new CardLayout());
		
		// instantiate all the pages we'll be using
		InstallerPage[] pages = new InstallerPage[]
		{
			new ConfigPage()
		};
		
		// add pages to layout
		contentPane.removeAll();
		for (InstallerPage page: pages)
			contentPane.add(page, page.getName());
		
		setIconImage(app_icon);
		setResizable(false);
		setTitle("FreeSpace Open Installer");
	}
}
