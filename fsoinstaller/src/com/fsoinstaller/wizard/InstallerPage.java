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

import java.awt.BorderLayout;
import java.awt.image.BufferedImage;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JPanel;

import com.fsoinstaller.utils.GraphicsUtils;


public abstract class InstallerPage extends JPanel
{
	private static final BufferedImage banner = GraphicsUtils.getResourceImage("top.png");
	
	private final JButton backButton = new JButton("< Back");
	private final JButton nextButton = new JButton("Next >");
	private final JButton cancelButton = new JButton("Cancel");
	
	public InstallerPage()
	{
		setLayout(new BorderLayout());
		add(createHeaderPanel(), BorderLayout.NORTH);
		add(createCenterPanel(), BorderLayout.CENTER);
		add(createFooterPanel(), BorderLayout.SOUTH);
	}
	
	public JPanel createHeaderPanel()
	{
		return new ImagePanel(banner);
	}
	
	public abstract JPanel createCenterPanel();
	
	public JPanel createFooterPanel()
	{
		JPanel footer = new JPanel();
		footer.setLayout(new BoxLayout(footer, BoxLayout.X_AXIS));
		footer.add(Box.createHorizontalGlue());
		footer.add(backButton);
		footer.add(Box.createHorizontalStrut(GUIConstants.DEFAULT_MARGIN));
		footer.add(nextButton);
		footer.add(Box.createHorizontalStrut(GUIConstants.DEFAULT_MARGIN * 2));
		footer.add(cancelButton);
		return footer;
	}
}
