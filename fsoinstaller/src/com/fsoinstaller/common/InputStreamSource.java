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

import java.io.IOException;
import java.io.InputStream;


/**
 * Interface used to provide InputStreams to InputStreamInStream for use in
 * 7-zip extraction.
 * 
 * @author Goober5000
 */
public interface InputStreamSource
{
	/**
	 * This is a callback method to "start over" getting data from an
	 * InputStream. Reading from a 7-Zip file might require seeking backwards,
	 * which is generally not supported in streams. Stream buffers can mitigate
	 * the situation, but they have limited size. This method provides a way for
	 * the stream to be recycled at its source. The stream processor actually
	 * doesn't care whether the stream is a brand new one or the same as the
	 * current one, so long as the stream pointer has been reset to the
	 * beginning.
	 * <p>
	 * The <tt>oldInputStream</tt> parameter will contain the stream that is
	 * to be recycled. If it is not possible for the stream's pointer to be
	 * reset, then the implementing class should close the old stream and open a
	 * new one. A null <tt>oldInputStream</tt> indicates that the stream is
	 * being requested for the first time.
	 * 
	 * @throws IOException if there is a problem creating a new InputStream
	 */
	public InputStream recycleInputStream(InputStream oldInputStream) throws IOException;
}
