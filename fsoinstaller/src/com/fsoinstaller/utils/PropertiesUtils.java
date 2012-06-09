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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Properties;


/**
 * Useful methods for operating with properties files.
 * 
 * @author Goober5000
 */
public class PropertiesUtils
{
	private static Logger logger = Logger.getLogger(PropertiesUtils.class);
	
	/**
	 * Prevent instantiation.
	 */
	private PropertiesUtils()
	{
	}
	
	public static Properties loadProperties(String resource)
	{
		Properties properties = null;
		
		// try file in local dir
		File file = new File(resource);
		if (file.exists())
			properties = loadPropertiesFromFile(file);
		if (properties != null)
			return properties;
		
		// try file in home dir
		file = new File(System.getProperty("user.home"), resource);
		if (file.exists())
			properties = loadPropertiesFromFile(file);
		if (properties != null)
			return properties;
		
		// try file in jar
		InputStream is = MiscUtils.getResourceStream(resource);
		if (is != null)
			properties = loadPropertiesFromStream(is);
		if (properties != null)
			return properties;
		
		return null;
	}
	
	public static void saveProperties(String resource, Properties properties)
	{
		// try file in home dir
		File file = new File(System.getProperty("user.home"), resource);
		savePropertiesToFile(file, properties);
	}
	
	public static Properties loadPropertiesFromFile(File file)
	{
		if (!file.exists())
			return null;
		
		logger.info("Loading properties from '" + file.getName() + "'");
		try
		{
			InputStream is = new FileInputStream(file);
			return loadPropertiesFromStream(is);
		}
		catch (FileNotFoundException fnfe)
		{
			logger.error("The properties file exists, but it could not be opened for reading!", fnfe);
		}
		
		return null;
	}
	
	public static void savePropertiesToFile(File file, Properties properties)
	{
		logger.info("Saving properties to '" + file.getName() + "'");
		try
		{
			OutputStream os = new FileOutputStream(file);
			savePropertiesToStream(os, properties);
		}
		catch (FileNotFoundException fnfe)
		{
			logger.error("The properties file could not be opened for writing!", fnfe);
		}
	}
	
	public static Properties loadPropertiesFromStream(InputStream is)
	{
		logger.info("Loading properties from input stream");
		try
		{
			try
			{
				Properties properties = new Properties();
				properties.load(is);
				return properties;
			}
			finally
			{
				is.close();
			}
		}
		catch (IOException ioe)
		{
			logger.error("There was a problem reading the properties object from the input stream!", ioe);
		}
		
		return null;
	}
	
	public static void savePropertiesToStream(OutputStream os, Properties properties)
	{
		logger.info("Saving properties to output stream");
		try
		{
			try
			{
				properties.store(os, "FSO Installer Properties");
			}
			finally
			{
				os.close();
			}
		}
		catch (IOException ioe)
		{
			logger.error("There was a problem writing the properties object to the output stream!", ioe);
		}
	}
}
