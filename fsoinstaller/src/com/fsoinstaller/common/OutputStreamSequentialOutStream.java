
package com.fsoinstaller.common;

import java.io.IOException;
import java.io.OutputStream;

import net.sf.sevenzipjbinding.ISequentialOutStream;
import net.sf.sevenzipjbinding.SevenZipException;


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
