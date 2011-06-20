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

/**
 * These tokens denote subsections of an installer document.
 * 
 * @author Goober5000
 */
public enum InstallerNodeToken
{
	NAME("NAME"),
	DESC("DESC"),
	ENDDESC("ENDDESC"),
	FOLDER("FOLDER"),
	DELETE("DELETE"),
	RENAME("RENAME"),
	URL("URL"),
	MULTIURL("MULTIURL"),
	ENDMULTI("ENDMULTI"),
	HASH("HASH"),
	VERSION("VERSION"),
	NOTE("NOTE"),
	ENDNOTE("ENDNOTE"),
	END("END");
	
	private final String token;
	
	private InstallerNodeToken(String token)
	{
		this.token = token;
		
		if (!InstallerNodeFactory.TOKEN_PATTERN.matcher(token).matches())
			throw new IllegalArgumentException("InstallerNodeToken must match token pattern!");
	}
	
	public String getToken()
	{
		return token;
	}
	
	@Override
	public String toString()
	{
		return token;
	}
}
