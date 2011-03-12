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

import java.io.File;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import com.fsoinstaller.utils.MiscUtils;
import com.fsoinstaller.utils.PropertiesUtils;


public class Configuration
{
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
		settings = new HashMap<String, Object>();
		properties = PropertiesUtils.loadProperties("fsoinstaller.properties");
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
	
	public void setProxyHost(String host)
	{
		if (host == null || host.equalsIgnoreCase("none") || host.isEmpty())
			host = "none";
		properties.setProperty("proxy.host", host);
	}
	
	public int getProxyPort()
	{
		String port = properties.getProperty("proxy.port");
		try
		{
			return Integer.valueOf(port);
		}
		catch (RuntimeException re)
		{
			return -1;
		}
	}
	
	public void setProxyPort(int port)
	{
		String portStr = (port < 0) ? "none" : Integer.toString(port);
		properties.setProperty("proxy.port", portStr);
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
	
	public String getLoggerLevel()
	{
		return properties.getProperty("logger.level");
	}
}
