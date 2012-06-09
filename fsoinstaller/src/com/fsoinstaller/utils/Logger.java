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

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.logging.ConsoleHandler;
import java.util.logging.FileHandler;
import java.util.logging.Formatter;
import java.util.logging.Level;
import java.util.logging.SimpleFormatter;
import java.util.logging.StreamHandler;


/**
 * This is a wrapper over the Java logging API that provides an API similar to
 * log4j.
 * 
 * @author Goober5000
 */
public final class Logger
{
	private static final Object mapMutex = new Object();
	private static final Map<Class<?>, Logger> map = new HashMap<Class<?>, Logger>();
	
	private static final List<StreamHandler> handlers;
	private static final Formatter formatter = new SimpleFormatter();
	
	// do logging setup
	static
	{
		// set level of root logger
		java.util.logging.Logger.getLogger("").setLevel(Level.ALL);
		
		List<StreamHandler> temp = new ArrayList<StreamHandler>();
		
		// add all our handlers
		StreamHandler consoleHandler = new ConsoleHandler();
		consoleHandler.setFormatter(formatter);
		temp.add(consoleHandler);
		try
		{
			FileHandler fileHandler = new FileHandler("logs/fsoinstaller.log", true);
			fileHandler.setFormatter(formatter);
			temp.add(fileHandler);
		}
		catch (IOException ioe)
		{
			java.util.logging.Logger.getLogger("global").log(Level.SEVERE, "Could not create FileHandler for fsoinstaller.log!", ioe);
		}
		
		// assign to list
		handlers = Collections.unmodifiableList(temp);
	}
	
	// per-logger logging setup
	private static java.util.logging.Logger createWrappedLogger(Class<?> clazz)
	{
		java.util.logging.Logger wrapped_logger = java.util.logging.Logger.getLogger(clazz.getName());
		for (StreamHandler handler: handlers)
			wrapped_logger.addHandler(handler);
		return wrapped_logger;
	}
	
	public static Logger getLogger(Class<?> clazz)
	{
		synchronized (mapMutex)
		{
			// return logger if map already contains it
			if (map.containsKey(clazz))
				return map.get(clazz);
			
			// create logger if not
			Logger logger = new Logger(clazz, createWrappedLogger(clazz));
			map.put(clazz, logger);
			return logger;
		}
	}
	
	private final java.util.logging.Logger logger;
	private final String className;
	
	private Logger(Class<?> clazz, java.util.logging.Logger logger)
	{
		this.className = clazz.getName();
		this.logger = logger;
	}
	
	private void log(Level level, Object message)
	{
		logger.logp(level, className, "", message == null ? "null" : message.toString());
	}
	
	private void log(Level level, Object message, Throwable throwable)
	{
		logger.logp(level, className, "", message == null ? "null" : message.toString(), throwable);
	}
	
	public void fatal(Object message)
	{
		log(Level.SEVERE, message);
	}
	
	public void fatal(Object message, Throwable throwable)
	{
		log(Level.SEVERE, message, throwable);
	}
	
	public void error(Object message)
	{
		log(Level.SEVERE, message);
	}
	
	public void error(Object message, Throwable throwable)
	{
		log(Level.SEVERE, message, throwable);
	}
	
	public void warn(Object message)
	{
		log(Level.WARNING, message);
	}
	
	public void warn(Object message, Throwable throwable)
	{
		log(Level.WARNING, message, throwable);
	}
	
	public void info(Object message)
	{
		log(Level.INFO, message);
	}
	
	public void info(Object message, Throwable throwable)
	{
		log(Level.INFO, message, throwable);
	}
	
	public void debug(Object message)
	{
		log(Level.FINE, message);
	}
	
	public void debug(Object message, Throwable throwable)
	{
		log(Level.FINE, message, throwable);
	}
	
	public void trace(Object message)
	{
		log(Level.FINER, message);
	}
	
	public void trace(Object message, Throwable throwable)
	{
		log(Level.FINER, message, throwable);
	}
	
	public boolean isInfoEnabled()
	{
		return logger.isLoggable(Level.INFO);
	}
	
	public boolean isDebugEnabled()
	{
		return logger.isLoggable(Level.FINE);
	}
	
	public boolean isTraceEnabled()
	{
		return logger.isLoggable(Level.FINER);
	}
}
