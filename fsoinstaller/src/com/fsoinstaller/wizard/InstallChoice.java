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

import java.awt.image.BufferedImage;

import com.fsoinstaller.utils.GraphicsUtils;


public enum InstallChoice
{
	BASIC("Basic", "basic.png", "Install the latest FreeSpace 2 Open and MediaVPs, but no mods."),
	COMPLETE("Complete", "complete.png", "Install everything: FreeSpace 2 Open, the MediaVPs, all mods, and all optional downloads."),
	CUSTOM("Custom", "custom.png", "Choose the mods to install");
	
	private final String name;
	private final BufferedImage image;
	private final String description;
	
	private InstallChoice(String name, String image, String description)
	{
		this.name = name;
		this.image = GraphicsUtils.getResourceImage(image);
		this.description = description;
	}
	
	public String getName()
	{
		return name;
	}
	
	public BufferedImage getImage()
	{
		return image;
	}
	
	public String getDescription()
	{
		return description;
	}
}
