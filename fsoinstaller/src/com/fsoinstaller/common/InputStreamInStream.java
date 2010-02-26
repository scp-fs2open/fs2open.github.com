
package com.fsoinstaller.common;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;

import net.sf.sevenzipjbinding.IInStream;
import net.sf.sevenzipjbinding.SevenZipException;


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
		else if (bufferPos >= buffer.length)
		{
			// get to the correct stream position
			seekForward(bufferPos - buffer.length);

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
		if (offset < 0)
			throw new IllegalArgumentException("This method is only for seeking forward");

		for (int i = 0; i < MAX_SEEK_TRIES && (offset > 0); i++)
		{
			long skipped = currentInputStream.skip(offset);
			offset -= skipped;
		}

		if (offset > 0)
			throw new IOException("Number of seek attempts exceeded MAX_SEEK_TRIES");
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

	public static void main(String[] args) throws SevenZipException
	{
		String thing = "The quick brown fox jumped over the lazy dogs.";
		final byte[] array = thing.getBytes();

		IInStream iis = new InputStreamInStream(new InputStreamSource()
		{
			@Override
			public InputStream recycleInputStream(InputStream oldInputStream) throws IOException
			{
				return new ByteArrayInputStream(array);
			}
		}, array.length, 8);

		int tempyoffset = 0;

		byte[] myArray = new byte[12];
		while (true)
		{
			int numRead = iis.read(myArray);
			if (numRead == 0)
				break;

			if (tempyoffset != 0)
				iis.seek(tempyoffset, SEEK_CUR);

			for (int i = 0; i < numRead; i++)
				System.out.print((char) myArray[i]);
		}
	}
}
