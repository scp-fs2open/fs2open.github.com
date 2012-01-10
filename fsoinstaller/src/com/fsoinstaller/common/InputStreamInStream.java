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

import net.sf.sevenzipjbinding.IInStream;
import net.sf.sevenzipjbinding.SevenZipException;


/**
 * This class acts as a bridge between the IInStream API used by 7-Zip-JBinding
 * and the InputStream API used by Java.
 * 
 * @author Goober5000
 */
public class InputStreamInStream implements IInStream
{
	private static final int defaultBufferSize = 8192;
	private static final int MAX_SEEK_TRIES = 10;

	protected final InputStreamSource inputStreamSource;
	protected InputStream currentInputStream;

	protected final byte[] buffer;
	protected final int bufferMiddle;
	protected long bufferPos;
	protected long overallPos;
	protected int bufferCount;
	protected final long overallCount;

	public InputStreamInStream(InputStreamSource inputStreamSource, long totalBytes)
	{
		this(inputStreamSource, totalBytes, defaultBufferSize);
	}

	public InputStreamInStream(InputStreamSource inputStreamSource, long totalBytes, int bufferSize)
	{
		if (inputStreamSource == null)
			throw new NullPointerException("InputStreamSource must not be null!");
		if (bufferSize <= 1)
			throw new IllegalArgumentException("Buffer size must be greater than 1");

		this.inputStreamSource = inputStreamSource;
		this.currentInputStream = null;

		this.buffer = new byte[bufferSize];
		this.bufferMiddle = buffer.length / 2;
		this.bufferCount = 0;
		this.bufferPos = 0;
		this.overallPos = 0;
		this.overallCount = totalBytes;
	}

	private void fillBuffer() throws IOException
	{
		// we no longer remember those bytes, so we have to restart
		if (bufferPos < 0)
		{
			// get to the correct stream position
			currentInputStream = inputStreamSource.recycleInputStream(currentInputStream);
			seekForward(overallPos);

			// reset the buffer
			bufferPos = 0;
			bufferCount = 0;
		}
		// we overran the buffer and need to catch up
		else if (bufferPos >= bufferCount)
		{
			// special case: if we're within one buffer of the end
			if (overallPos >= overallCount - buffer.length)
			{
				// calculate amount we'd need to seek
				int newBufferPos = (int) (overallPos - (overallCount - buffer.length));
				int offset = (int) (bufferPos - bufferCount) - newBufferPos;

				// it's possible that we've arrived in bounds already
				if (offset < 0)
				{
					// finish the buffer
					// (the buffer info will be properly adjusted below)
					readFully(buffer, -offset, buffer.length + offset);
				}
				// not there yet
				else
				{
					// seek, then read the whole buffer
					seekForward(offset);
					readFully(buffer, 0, buffer.length);
				}

				// now set the new buffer info
				bufferPos = newBufferPos;
				bufferCount = buffer.length;
				return;
			}

			// get to the correct stream position
			seekForward(bufferPos - bufferCount);

			// reset the buffer
			bufferPos = 0;
			bufferCount = 0;
		}
		// keep the pointer in the middle of the buffer, so that we can seek both forwards and backwards
		else if (bufferPos > bufferMiddle)
		{
			int shift = ((int) bufferPos) - bufferMiddle;
			bufferCount -= shift;
			bufferPos -= shift;
			System.arraycopy(buffer, shift, buffer, 0, bufferCount);
		}

		if (currentInputStream == null)
			currentInputStream = inputStreamSource.recycleInputStream(null);

		// try to read the rest of the buffer
		int bytesRead = currentInputStream.read(buffer, bufferCount, buffer.length - bufferCount);
		if (bytesRead > 0)
			bufferCount += bytesRead;
	}

	private void seekForward(long offset) throws IOException
	{
		int tries;

		if (offset == 0)
			return;
		else if (offset < 0)
			throw new IllegalArgumentException("This method is only for seeking forward");

		// try skip-seek
		tries = 0;
		while (offset > 0 && tries < MAX_SEEK_TRIES)
		{
			long skipped = currentInputStream.skip(offset);
			if (skipped > 0)
				offset -= skipped;
			else
				tries++;
		}

		// try read-seek
		tries = 0;
		while (offset > 0 && tries < MAX_SEEK_TRIES)
		{
			long read = currentInputStream.read(buffer, 0, (offset < buffer.length) ? (int) offset : buffer.length);
			if (read > 0)
				offset -= read;
			else
				tries++;
		}

		if (offset > 0)
			throw new IOException("Number of seek attempts exceeded MAX_SEEK_TRIES");
	}

	private void readFully(byte[] array, int start, int length) throws IOException
	{
		int tries;

		if (length == 0)
			return;
		else if (length - start > array.length)
			throw new IllegalArgumentException("Bytes from start to length must fit into the array!");

		// read it
		tries = 0;
		while (length > 0 && tries < MAX_SEEK_TRIES)
		{
			int read = currentInputStream.read(array, start, length);
			if (read > 0)
			{
				length -= read;
				start += read;
			}
			else
				tries++;
		}

		if (length > 0)
			throw new IOException("Number of read tries exceeded MAX_SEEK_TRIES");
	}

	@Override
	public long seek(long offset, int seekOrigin) throws SevenZipException
	{
		switch (seekOrigin)
		{
			// seek from the beginning of the stream
			case SEEK_SET:
				return seek(offset - overallPos, SEEK_CUR);

			// seek from the current position
			case SEEK_CUR:
				bufferPos += offset;
				overallPos += offset;
				if (overallPos < 0)
					throw new SevenZipException("Can't read a negative stream position!");
				return overallPos;

			// seek from the end of the stream
			case SEEK_END:
				return seek(overallCount + offset - overallPos, SEEK_CUR);

			default:
				throw new IllegalArgumentException("Unrecognized seek method!");
		}
	}

	@Override
	public int read(byte[] data) throws SevenZipException
	{
		if (data.length == 0)
			return 0;

		if (overallPos < 0)
			throw new SevenZipException("Can't read a negative stream position!");
		else if (overallPos >= overallCount)
			return 0;

		// ensure buffer is available
		if (bufferPos < 0 || bufferPos >= bufferCount)
		{
			try
			{
				fillBuffer();
			}
			catch (IOException ioe)
			{
				throw new SevenZipException("Error reading input stream", ioe);
			}

			// unable to read more bytes
			if (bufferPos >= bufferCount)
				return 0;
		}

		// we want to get as many bytes as possible, but not more than we can hold
		int available = bufferCount - (int) bufferPos;
		if (available > data.length)
			available = data.length;

		// copy them
		System.arraycopy(buffer, (int) bufferPos, data, 0, available);
		bufferPos += available;
		overallPos += available;

		return available;
	}

	public void close() throws IOException
	{
		if (currentInputStream != null)
		{
			currentInputStream.close();
			currentInputStream = null;
		}
	}
}
