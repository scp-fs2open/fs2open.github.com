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

package com.fsoinstaller.common;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;


public class BaseURL
{
	protected URI baseURL;

	public BaseURL(String baseURL) throws InvalidBaseURLException
	{
		if (baseURL == null)
			throw new NullPointerException("The 'baseURL' field cannot be null!");

		try
		{
			this.baseURL = new URI(baseURL);
		}
		catch (URISyntaxException urise)
		{
			throw new InvalidBaseURLException("Syntax error in the URL", urise);
		}

		if (!validateURL(this.baseURL))
		{
			throw new InvalidBaseURLException("The URL '" + baseURL + "' must use the HTTP protocol; must point to a folder, not a file; and must be absolute");
		}
	}

	/**
	 * Ensure that we use HTTP; that we are pointing to a folder, not a file;
	 * and that we are absolute. URIs are used because checking for correctness
	 * is "friendlier" than with URLs, in that URIs only require correct syntax.
	 * URLs require protocol checking and other such nastiness that we don't
	 * need to deal with until we're actually ready to go online. (Besides, this
	 * way we can filter out non-HTTP protocols before even constructing a URL
	 * object.)
	 */
	public static boolean validateURL(URI theURL)
	{
		// check protocol
		if (!theURL.getScheme().toLowerCase().equals("http"))
			return false;

		// check path
		if (!theURL.normalize().getPath().endsWith("/"))
			return false;

		// check absolute
		if (!theURL.isAbsolute())
			return false;

		return true;
	}

	public URL toURL() throws MalformedURLException
	{
		return new URL(baseURL.toString());
	}

	public URL toURL(String fileName) throws MalformedURLException
	{
		return new URL(baseURL.toString() + fileName);
	}

	@Override
	public int hashCode()
	{
		return baseURL.hashCode();
	}

	@Override
	public boolean equals(Object object)
	{
		if (this == object)
			return true;
		if (object == null)
			return false;
		if (getClass() != object.getClass())
			return false;
		return baseURL.equals(((BaseURL) object).baseURL);
	}

	@Override
	public String toString()
	{
		return baseURL.toString();
	}
}
