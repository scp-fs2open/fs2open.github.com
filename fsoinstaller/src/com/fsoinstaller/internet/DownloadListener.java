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

import java.util.EventListener;


/**
 * A listener for various events that might happen during, or because of, a file
 * download.
 * 
 * @author Goober5000
 */
public interface DownloadListener extends EventListener
{
	public void downloadNotNecessary(DownloadEvent event);

	public void downloadAboutToStart(DownloadEvent event);

	public void downloadProgressReport(DownloadEvent event);

	public void downloadComplete(DownloadEvent event);

	public void downloadFailed(DownloadEvent event);
}
