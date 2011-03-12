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

import javax.swing.JComponent;
import javax.swing.JFrame;

import com.fsoinstaller.utils.GraphicsUtils;


public class InstallerGUI extends JFrame
{
	private static final BufferedImage app_icon = GraphicsUtils.getResourceImage("fso_icon.png");
	
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
		
		// final frame tweaks
		setIconImage(app_icon);
		setResizable(false);
		setTitle("FreeSpace Open Installer");
	}
}
