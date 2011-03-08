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

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.image.BufferedImage;

import javax.swing.JPanel;

import com.fsoinstaller.utils.GraphicsUtils;


/**
 * This is a panel that just paints an image while staying the size of that
 * image.
 * 
 * @author Goober5000
 */
public class ImagePanel extends JPanel
{
	private final BufferedImage image;
	
	public ImagePanel()
	{
		this(null);
	}
	
	public ImagePanel(Image image)
	{
		this.image = GraphicsUtils.getBufferedImage(image);
		
		Dimension size = getImageSize();
		setMinimumSize(size);
		setMaximumSize(size);
		setPreferredSize(size);
	}
	
	public Dimension getImageSize()
	{
		if (image == null)
			return new Dimension(0, 0);
		
		return new Dimension(image.getWidth(), image.getHeight());
	}
	
	@Override
	public void paintComponent(Graphics g)
	{
		super.paintComponent(g);
		
		if (image == null)
			return;
		
		g.drawImage(image, 0, 0, image.getWidth(), image.getHeight(), this);
	}
}
