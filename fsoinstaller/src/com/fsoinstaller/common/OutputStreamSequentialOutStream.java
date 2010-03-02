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
import java.io.OutputStream;

import net.sf.sevenzipjbinding.ISequentialOutStream;
import net.sf.sevenzipjbinding.SevenZipException;


/**
 * This class acts as a bridge between the OutputStream API used by Java and the
 * ISequentialOutStream API used by 7-Zip-JBinding.
 * 
 * @author Goober5000
 */
public class OutputStreamSequentialOutStream implements ISequentialOutStream
{
	protected OutputStream currentOutputStream;

	public OutputStreamSequentialOutStream(OutputStream outputStream)
	{
		if (outputStream == null)
			throw new NullPointerException("OutputStream must not be null!");
		this.currentOutputStream = outputStream;
	}

	@Override
	public int write(byte[] data) throws SevenZipException
	{
		if (data.length == 0)
			return 0;

		try
		{
			currentOutputStream.write(data);
			return data.length;
		}
		catch (IOException ioe)
		{
			throw new SevenZipException("Error writing to output stream");
		}
	}

	public void close() throws IOException
	{
		currentOutputStream.close();
	}
}
