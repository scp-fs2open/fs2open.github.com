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
import java.awt.EventQueue;
import java.awt.FontMetrics;
import java.awt.Frame;
import java.awt.Toolkit;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.net.URL;
import java.text.BreakIterator;
import java.util.ArrayList;
import java.util.List;

import javax.swing.SwingUtilities;


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
		
		// ensure the window isn't partially off the top left of the screen
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		
		// center it
		window.setLocation(x, y);
	}
	
	public static void centerWindowOnParent(Container window, Container parent)
	{
		if (parent == null)
		{
			centerWindowOnScreen(window);
			return;
		}
		
		// find the coordinates to center the whole window
		int x = (int) (parent.getX() + ((parent.getWidth() - window.getWidth()) / 2.0 + 0.5));
		int y = (int) (parent.getY() + ((parent.getHeight() - window.getHeight()) / 2.0 + 0.5));
		
		// ensure the window isn't partially off the top left of the screen
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		
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
	
	public static void invokeAndWait(Runnable runnable)
	{
		try
		{
			// be sure we don't block the event dispatch thread
			if (EventQueue.isDispatchThread())
				runnable.run();
			else
				EventQueue.invokeAndWait(runnable);
		}
		catch (InvocationTargetException ite)
		{
			logger.error("An InvocationTargetException occurred!", ite);
			if (ite.getCause() instanceof Error)
				throw (Error) ite.getCause();
			else if (ite.getCause() instanceof RuntimeException)
				throw (RuntimeException) ite.getCause();
			else
				throw new IllegalStateException("Illegal exception type?", ite.getCause());
		}
		catch (InterruptedException ie)
		{
			logger.error("Thread interrupted!", ie);
			Thread.currentThread().interrupt();
		}
	}
	
	public static void sleep(long millis)
	{
		try
		{
			Thread.sleep(millis);
		}
		catch (InterruptedException ie)
		{
			logger.error("Thread interrupted!", ie);
			Thread.currentThread().interrupt();
		}
	}
	
	public static InputStream getResourceStream(String resource)
	{
		InputStream is = ClassLoader.getSystemResourceAsStream("resources/" + resource);
		if (is != null)
		{
			logger.info("Loading '" + resource + "' via system class loader");
			return is;
		}
		
		is = MiscUtils.class.getResourceAsStream("resources/" + resource);
		if (is != null)
		{
			logger.info("Loading '" + resource + "' via MiscUtils class loader");
			return is;
		}
		
		return null;
	}
	
	public static URL getResourceURL(String resource)
	{
		URL url = ClassLoader.getSystemResource("resources/" + resource);
		if (url != null)
		{
			logger.info("Loading '" + resource + "' via system class loader");
			return url;
		}
		
		url = MiscUtils.class.getResource("resources/" + resource);
		if (url != null)
		{
			logger.info("Loading '" + resource + "' via MiscUtils class loader");
			return url;
		}
		
		return null;
	}
	
	public static File validateApplicationDir(String dirName)
	{
		if (dirName == null || dirName.isEmpty())
			return null;
		
		File file = new File(dirName);
		if (validateApplicationDir(file))
			return file;
		else
			return null;
	}
	
	public static boolean validateApplicationDir(File dir)
	{
		// should be valid
		return true;
	}
	
	public static List<String> readTextFile(File file)
	{
		List<String> lines = new ArrayList<String>();
		try
		{
			BufferedReader reader = null;
			try
			{
				// open file for buffered input
				reader = new BufferedReader(new FileReader(file));
				
				// read lines
				String line;
				while ((line = reader.readLine()) != null)
					lines.add(line);
			}
			finally
			{
				if (reader != null)
					reader.close();
			}
		}
		catch (IOException ioe)
		{
			logger.error("Error reading file '" + file.getName() + "'!", ioe);
			lines.clear();
		}
		return lines;
	}
	
	public enum OperatingSystem
	{
		WINDOWS,
		MAC,
		UNIX,
		OTHER
	}
	
	public static OperatingSystem determineOS()
	{
		String os_name_lower = System.getProperty("os.name").toLowerCase();
		if (os_name_lower.startsWith("windows"))
			return OperatingSystem.WINDOWS;
		else if (os_name_lower.startsWith("mac"))
			return OperatingSystem.MAC;
		else if (os_name_lower.contains("nix") || os_name_lower.contains("nux"))
			return OperatingSystem.UNIX;
		else
			return OperatingSystem.OTHER;
	}
	
	public static boolean validForOS(String modName)
	{
		OperatingSystem os = determineOS();
		
		// if we have a specific OS, make sure the name doesn't exclude itself
		if (os != OperatingSystem.OTHER)
		{
			String mod_lower = modName.toLowerCase();
			if (mod_lower.contains("windows") && os != OperatingSystem.WINDOWS)
				return false;
			if ((mod_lower.contains("macintosh") || mod_lower.contains("osx") || mod_lower.contains("os x")) && os != OperatingSystem.MAC)
				return false;
			if ((mod_lower.contains("linux") || mod_lower.contains("unix")) && os != OperatingSystem.UNIX)
				return false;
		}
		
		return true;
	}
	
	/**
	 * Handy-dandy text wrapping function, adapted from
	 * http://www.geekyramblings.net/2005/06/30/wrap-jlabel-text/.
	 */
	public static String wrapText(String text, FontMetrics metrics, int maxWidth)
	{
		BreakIterator boundary = BreakIterator.getWordInstance();
		boundary.setText(text);
		
		StringBuilder line = new StringBuilder();
		StringBuilder paragraph = new StringBuilder();
		
		// build it with embedded newlines
		for (int start = boundary.first(), end = boundary.next(); end != BreakIterator.DONE; start = end, end = boundary.next())
		{
			String word = text.substring(start, end);
			
			line.append(word);
			int lineWidth = SwingUtilities.computeStringWidth(metrics, line.toString());
			
			if (lineWidth > maxWidth)
			{
				// trim off whitespace at the beginning of the word
				word = word.replaceAll("^\\s+", "");
				
				// append a newline, and start a new line
				line = new StringBuilder(word);
				paragraph.append("\n");
			}
			
			paragraph.append(word);
		}
		
		return paragraph.toString();
	}
}
