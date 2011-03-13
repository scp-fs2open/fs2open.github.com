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

package com.fsoinstaller.utils;

import java.awt.Container;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Toolkit;
import java.io.File;
import java.io.InputStream;
import java.net.URL;


/**
 * Miscellaneous useful methods.
 * 
 * @author Goober5000
 */
public class MiscUtils
{
	private static Logger logger = Logger.getLogger(MiscUtils.class);
	
	/**
	 * Prevent instantiation.
	 */
	private MiscUtils()
	{
	}
	
	public static void centerWindowOnScreen(Container window)
	{
		// find the coordinates to center the whole window
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		int x = (int) ((screenSize.getWidth() - window.getWidth()) / 2.0 + 0.5);
		int y = (int) ((screenSize.getHeight() - window.getHeight()) / 2.0 + 0.5);
		
		// center it
		window.setLocation(x, y);
	}
	
	public static void centerWindowOnParent(Container window, Container parent)
	{
		// find the coordinates to center the whole window
		int x = (int) (parent.getX() + ((parent.getWidth() - window.getWidth()) / 2.0 + 0.5));
		int y = (int) (parent.getY() + ((parent.getHeight() - window.getHeight()) / 2.0 + 0.5));
		
		// center it
		window.setLocation(x, y);
	}
	
	public static Frame getActiveFrame()
	{
		Frame[] frames = Frame.getFrames();
		for (Frame frame: frames)
			if (frame.isVisible())
				return frame;
		return null;
	}
	
	public static InputStream getResourceStream(String resource)
	{
		InputStream is = ClassLoader.getSystemResourceAsStream(resource);
		if (is != null)
		{
			logger.info("Loading '" + resource + "' via system class loader");
			return is;
		}
		
		is = MiscUtils.class.getResourceAsStream(resource);
		if (is != null)
		{
			logger.info("Loading '" + resource + "' via MiscUtils class loader");
			return is;
		}
		
		return null;
	}
	
	public static URL getResourceURL(String resource)
	{
		URL url = ClassLoader.getSystemResource(resource);
		if (url != null)
		{
			logger.info("Loading '" + resource + "' via system class loader");
			return url;
		}
		
		url = MiscUtils.class.getResource(resource);
		if (url != null)
		{
			logger.info("Loading '" + resource + "' via MiscUtils class loader");
			return url;
		}
		
		return null;
	}
	
	public static File validateApplicationDir(String dirName)
	{
		if (dirName == null)
			return null;
		
		File file = new File(dirName);
		if (validateApplicationDir(file))
			return file;
		else
			return null;
	}
	
	public static boolean validateApplicationDir(File dir)
	{
		// must be a directory
		if (!dir.isDirectory())
			return false;
		
		// should be valid
		return true;
	}
}
