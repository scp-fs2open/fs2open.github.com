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

package com.fsoinstaller.internet;

import java.io.IOException;
import java.net.Authenticator;
import java.net.InetSocketAddress;
import java.net.PasswordAuthentication;
import java.net.Proxy;
import java.net.URL;
import java.net.URLConnection;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;


/**
 * Performs the actual connection to the Internet, either by launching a browser
 * or by opening a connection to a URL. Has the ability to connect through a
 * proxy, with an optional username and password. Rewritten from a similar class
 * originally created by Turey.
 * <p>
 * Connector is an immutable class (except for static management of
 * authentication) and therefore, without authentication, it is intrinsically
 * thread-safe. Furthermore, the authentication code should also be thread-safe
 * even for different Connector instances in multiple threads.
 * 
 * @author Turey
 * @author Goober5000
 */
public class Connector
{
	private static final Logger logger = Logger.getLogger(Connector.class);

	/**
	 * Used to identify the windows platform.
	 */
	private static final String WIN_ID = "Windows";
	/**
	 * The default system browser under windows.
	 */
	private static final String WIN_PATH = "rundll32";
	/**
	 * The flag to display a url.
	 */
	private static final String WIN_FLAG = "url.dll,FileProtocolHandler";
	/**
	 * The default browser under unix.
	 */
	private static final String UNIX_PATH = "firefox";
	/**
	 * The flag to display a url.
	 */
	private static final String UNIX_FLAG = "";
	/**
	 * The single authenticator used by this class for all Proxies.
	 */
	private static final ConnectorAuthenticator authenticator = new ConnectorAuthenticator();
	static
	{
		Authenticator.setDefault(authenticator);
	}

	protected final Proxy proxy;
	protected final boolean onWindows;

	public Connector()
	{
		proxy = null;
		onWindows = isWindowsPlatform();
	}

	public Connector(String proxyHost, int proxyPort) throws InvalidProxyException
	{
		try
		{
			proxy = new Proxy(Proxy.Type.HTTP, new InetSocketAddress(proxyHost, proxyPort));
		}
		catch (IllegalArgumentException iae)
		{
			throw new InvalidProxyException("Cannot create a proxy object from the host '" + proxyHost + "' at port " + proxyPort + "!");
		}
		onWindows = isWindowsPlatform();
	}

	public Connector(String proxyHost, int proxyPort, String proxyUsername, char[] proxyPassword) throws InvalidProxyException
	{
		this(proxyHost, proxyPort);

		authenticator.put(proxy, new PasswordAuthentication(proxyUsername, proxyPassword));
		Arrays.fill(proxyPassword, (char) 0);
	}

	/**
	 * Display a file in the system browser. If you want to display a file, you
	 * must include the absolute path name.
	 */
	public boolean browseToURL(URL url)
	{
		logger.info("Browsing to URL: " + url);

		String cmd = null;
		try
		{
			if (onWindows)
			{
				logger.debug("Launching browser in Windows OS");

				// cmd = 'rundll32 url.dll,FileProtocolHandler http://...'
				cmd = WIN_PATH + " " + WIN_FLAG + " " + url;
				Runtime.getRuntime().exec(cmd);
			}
			else
			{
				logger.debug("Launching browser in non-Windows OS");

				// Under Unix, Netscape has to be running for the "-remote"
				// command to work.  So, we try sending the command and
				// check for an exit value.  If the exit command is 0,
				// it worked, otherwise we need to start the browser.
				// cmd = 'netscape -remote openURL(http://www.java-tips.org)'
				cmd = UNIX_PATH + " " + UNIX_FLAG + url;
				Process process = Runtime.getRuntime().exec(cmd);
				try
				{
					logger.debug("Waiting for exit code");

					// wait for exit code -- if it's 0, command worked,
					// otherwise we need to start the browser up.
					int exitCode = process.waitFor();
					if (exitCode != 0)
					{
						logger.debug("Command failed; starting the browser explicitly");

						// cmd = 'netscape http://www.java-tips.org'
						cmd = UNIX_PATH + " " + url;
						Runtime.getRuntime().exec(cmd);
					}
				}
				catch (InterruptedException ie)
				{
					logger.error("Error bringing up browser; cmd='" + cmd + "'", ie);

					// restore interrupt
					Thread.currentThread().interrupt();

					return false;
				}
			}
		}
		catch (IOException ioe)
		{
			// couldn't exec browser
			logger.error("Could not invoke browser; cmd='" + cmd + "'", ioe);
			return false;
		}

		return true;
	}

	public URLConnection openConnection(URL url) throws IOException
	{
		logger.debug("Opening connection to URL: " + url);

		URLConnection conn;
		if (proxy == null)
			conn = url.openConnection();
		else
			conn = url.openConnection(proxy);

		return conn;
	}

	/**
	 * Try to determine whether this application is running under Windows or
	 * some other platform by examining the "os.name" property.
	 * 
	 * @return true if this application is running under a Windows OS
	 */
	private static boolean isWindowsPlatform()
	{
		String os = System.getProperty("os.name");
		return (os != null && os.startsWith(WIN_ID));
	}

	/**
	 * Only one instance of this class should be created, and it should be set
	 * as the system default. It allows multiple Connector instances to be
	 * created even though there is a single system-wide authenticator.
	 */
	private static class ConnectorAuthenticator extends Authenticator
	{
		/**
		 * This is synchronized because we might have different Connector
		 * instances in different threads.
		 */
		private Map<Proxy, PasswordAuthentication> map = Collections.synchronizedMap(new HashMap<Proxy, PasswordAuthentication>());

		public void put(Proxy proxy, PasswordAuthentication authentication)
		{
			map.put(proxy, authentication);
		}

		@Override
		protected PasswordAuthentication getPasswordAuthentication()
		{
			logger.info("Checking proxy authentication...");
			if (logger.isDebugEnabled())
			{
				logger.debug("requesting host=" + getRequestingHost());
				logger.debug("requesting port=" + getRequestingPort());
				logger.debug("requesting prompt=" + getRequestingPrompt());
				logger.debug("requesting protocol=" + getRequestingProtocol());
				logger.debug("requesting scheme=" + getRequestingScheme());
				logger.debug("requesting site=" + getRequestingSite());
				logger.debug("requesting url=" + getRequestingURL());
				logger.debug("requesting type=" + getRequestorType());
				logger.debug("-----------------------------------------");
			}

			int mapSize;
			synchronized (map)
			{
				mapSize = map.size();
				for (Map.Entry<Proxy, PasswordAuthentication> entry: map.entrySet())
				{
					InetSocketAddress address = ((InetSocketAddress) entry.getKey().address());

					if (logger.isDebugEnabled())
					{
						logger.debug("proxy type=" + entry.getKey().type());
						logger.debug("proxy host=" + address.getHostName());
						logger.debug("proxy port=" + address.getPort());
						logger.debug("proxy site=" + address.getAddress());
					}

					if (address.getHostName().equals(getRequestingHost()) && address.getPort() == getRequestingPort() && address.getAddress().toString().equals(getRequestingSite().toString()))
					{
						logger.info("Stored proxy information and requesting site match; performing authentication...");
						return entry.getValue();
					}
				}
			}

			logger.warn("Could not find a stored proxy that matched the requesting site!");
			if (mapSize == 1)
				logger.error("The one and only proxy stored in the map did not match the request; this is very bad!");
			return null;
		}
	}
}
