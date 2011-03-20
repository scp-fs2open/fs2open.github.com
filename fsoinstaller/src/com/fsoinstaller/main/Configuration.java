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

package com.fsoinstaller.main;

import java.io.File;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import com.fsoinstaller.utils.Logger;
import com.fsoinstaller.utils.MiscUtils;
import com.fsoinstaller.utils.PropertiesUtils;


/**
 * Thread-safe class which manages access to global state.
 * 
 * @author Goober5000
 */
public class Configuration
{
	private static final Logger logger = Logger.getLogger(Configuration.class);
	
	public static final String PROXY_KEY = "PROXY";
	public static final String CONNECTOR_KEY = "CONNECTOR";
	public static final String DOWNLOADER_KEY = "DOWNLOADER";
	public static final String REMOTE_VERSION_KEY = "REMOTE-VERSION";
	public static final String MOD_URLS_KEY = "MOD-URLS";
	public static final String MOD_NODES_KEY = "MOD-NODES";
	
	/**
	 * Use the Initialization On Demand Holder idiom for thread-safe
	 * non-synchronized singletons.
	 */
	private static final class InstanceHolder
	{
		private static final Configuration INSTANCE = new Configuration();
	}
	
	public static Configuration getInstance()
	{
		return InstanceHolder.INSTANCE;
	}
	
	private final Map<String, Object> settings;
	private final Properties properties;
	
	// prevent instantiation
	private Configuration()
	{
		Properties temp = PropertiesUtils.loadProperties("fsoinstaller.properties");
		if (temp == null)
		{
			logger.info("No fsoinstaller.properties file could be found; a new one will be created");
			temp = new Properties();
		}
		
		settings = Collections.synchronizedMap(new HashMap<String, Object>());
		// since Properties inherits from Hashtable, it is already thread-safe
		properties = temp;
	}
	
	public void saveProperties()
	{
		PropertiesUtils.saveProperties("fsoinstaller.properties", properties);
	}
	
	public Map<String, Object> getSettings()
	{
		return settings;
	}
	
	public Properties getProperties()
	{
		return properties;
	}
	
	public String getProxyHost()
	{
		String host = properties.getProperty("proxy.host");
		if (host == null || host.equalsIgnoreCase("none") || host.isEmpty())
			return null;
		return host;
	}
	
	public int getProxyPort()
	{
		String port = properties.getProperty("proxy.port");
		try
		{
			return Integer.valueOf(port);
		}
		catch (NumberFormatException re)
		{
			return -1;
		}
	}
	
	public void setProxyInfo(String host, int port)
	{
		// resolve host
		if (host == null || host.equalsIgnoreCase("none") || host.isEmpty())
			host = null;
		
		// store either both or none
		boolean valid = (host != null && port >= 0);
		
		properties.setProperty("proxy.host", valid ? host : "none");
		properties.setProperty("proxy.port", valid ? Integer.toString(port) : "none");
	}
	
	public String getApplicationTitle()
	{
		return properties.getProperty("application.title");
	}
	
	// this is not likely to ever get called unless we're building a custom installer
	public void setApplicationTitle(String title)
	{
		properties.setProperty("application.title", title);
	}
	
	public File getApplicationDir()
	{
		String fileName = properties.getProperty("application.dir");
		return MiscUtils.validateApplicationDir(fileName);
	}
	
	public void setApplicationDir(File dir)
	{
		String dirStr = MiscUtils.validateApplicationDir(dir) ? dir.getAbsolutePath() : "none";
		properties.setProperty("application.dir", dirStr);
	}
}
