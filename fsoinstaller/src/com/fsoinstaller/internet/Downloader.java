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

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.LinkedList;
import java.util.List;
import java.util.zip.Adler32;
import java.util.zip.CheckedInputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import org.apache.log4j.Logger;


public class Downloader
{
	private static final Logger logger = Logger.getLogger(Downloader.class);

	protected static final int BUFFER_SIZE = 2048;

	protected List<DownloadListener> downloadListeners;

	public Downloader()
	{
		downloadListeners = new LinkedList<DownloadListener>();
	}

	public boolean download(String sourceAddress, String destinationFile)
	{
		logger.info("Downloading from " + sourceAddress + " to local file " + destinationFile);

		File file = new File(destinationFile);
		long totalBytes = 0;
		InputStream inputStream = null;
		OutputStream outputStream = null;
		try
		{
			URL url = new URL(sourceAddress);

			logger.debug("Opening connection...");
			URLConnection connection = InternetUtil.getConnection(url);
			totalBytes = connection.getContentLength();

			logger.debug("Checking if the file is up to date...");
			if (file.exists() && (totalBytes > 0) && (file.length() == totalBytes))
			{
				fireNoDownloadNecessary(this, file.getName(), 0, totalBytes);
				return true;
			}

			logger.debug("Opening input and output streams...");
			inputStream = connection.getInputStream();
			outputStream = openOutputStream(file);

			actuallyDownload(inputStream, outputStream, file.getName(), totalBytes);

			logger.debug("Closing input and output streams...");
			outputStream.close();
			outputStream = null;
			inputStream.close();
			inputStream = null;

			return true;
		}
		catch (IOException ioe)
		{
			logger.error("An exception was thrown during download!", ioe);
			fireDownloadFailed(this, file.getName(), 0, totalBytes);

			return false;
		}
		finally
		{
			try
			{
				if (inputStream != null)
					inputStream.close();
				if (outputStream != null)
					outputStream.close();
			}
			catch (IOException ioe)
			{
				logger.warn("Could not close download streams!", ioe);
			}
		}
	}

	public boolean downloadZip(String sourceAddress, String destinationPath)
	{
		logger.info("Downloading and extracting from " + sourceAddress + " to local destination " + destinationPath);

		String currentEntry = "";
		long totalBytes = 0;
		ZipInputStream zipInputStream = null;
		OutputStream outputStream = null;
		try
		{
			URL url = new URL(sourceAddress);

			logger.debug("Opening connection...");
			URLConnection connection = InternetUtil.getConnection(url);

			logger.debug("Opening input stream...");
			CheckedInputStream checksum = new CheckedInputStream(connection.getInputStream(), new Adler32());
			zipInputStream = new ZipInputStream(new BufferedInputStream(checksum));

			ZipEntry entry;
			while ((entry = zipInputStream.getNextEntry()) != null)
			{
				currentEntry = entry.getName();
				totalBytes = entry.getSize();

				logger.debug("Checking entry '" + currentEntry + "'");
				File file = new File(destinationPath + currentEntry);

				if (entry.isDirectory())
				{
					file.mkdir();
					zipInputStream.closeEntry();
					continue;
				}

				logger.debug("Checking if the file is up to date...");
				if (file.exists() && (totalBytes > 0) && (file.length() == totalBytes))
				{
					fireNoDownloadNecessary(this, file.getName(), 0, totalBytes);
					zipInputStream.closeEntry();
					continue;
				}

				logger.debug("Opening output stream...");
				outputStream = openOutputStream(file);

				actuallyDownload(zipInputStream, outputStream, currentEntry, totalBytes);

				logger.debug("Closing output stream...");
				outputStream.close();
				outputStream = null;

				zipInputStream.closeEntry();
			}

			logger.debug("Closing input stream...");
			zipInputStream.close();
			zipInputStream = null;

			return true;
		}
		catch (IOException ioe)
		{
			logger.error("An exception was thrown during download!", ioe);
			fireDownloadFailed(this, currentEntry, 0, totalBytes);

			return false;
		}
		finally
		{
			try
			{
				if (zipInputStream != null)
					zipInputStream.close();
				if (outputStream != null)
					outputStream.close();
			}
			catch (IOException ioe)
			{
				logger.warn("Could not close download streams!", ioe);
			}
		}
	}

	protected OutputStream openOutputStream(File file) throws IOException
	{
		try
		{
			return new BufferedOutputStream(new FileOutputStream(file));
		}
		catch (FileNotFoundException fnfe)
		{
			logger.trace("local file not found; creating it");
			file.createNewFile();
			return new BufferedOutputStream(new FileOutputStream(file));
		}
	}

	protected void actuallyDownload(InputStream inputStream, OutputStream outputStream, String downloadName, long downloadTotalSize) throws IOException
	{
		byte[] buffer = new byte[BUFFER_SIZE];
		long totalBytesWritten = 0;

		logger.debug("Downloading...");
		fireAboutToStart(this, downloadName, totalBytesWritten, downloadTotalSize);

		int bytesRead = 0;
		while ((bytesRead = inputStream.read(buffer)) != -1)
		{
			outputStream.write(buffer, 0, bytesRead);
			totalBytesWritten += bytesRead;

			fireProgressReport(this, downloadName, totalBytesWritten, downloadTotalSize);
		}

		logger.debug("Download complete");
		fireDownloadComplete(this, downloadName, totalBytesWritten, downloadTotalSize);
	}

	public void addDownloadListener(DownloadListener listener)
	{
		downloadListeners.add(listener);
	}

	public void removeDownloadListener(DownloadListener listener)
	{
		downloadListeners.remove(listener);
	}

	protected void fireNoDownloadNecessary(Object source, String downloadName, long downloadedBytes, long totalBytes)
	{
		DownloadEvent event = null;
		for (DownloadListener listener: downloadListeners)
		{
			// lazy instantiation of the event
			if (event == null)
				event = new DownloadEvent(source, downloadName, downloadedBytes, totalBytes);
			
			// fire it
			listener.downloadNotNecessary(event);
		}
	}

	protected void fireAboutToStart(Object source, String downloadName, long downloadedBytes, long totalBytes)
	{
		DownloadEvent event = null;
		for (DownloadListener listener: downloadListeners)
		{
			// lazy instantiation of the event
			if (event == null)
				event = new DownloadEvent(source, downloadName, downloadedBytes, totalBytes);
			
			// fire it
			listener.downloadAboutToStart(event);
		}
	}

	protected void fireProgressReport(Object source, String downloadName, long downloadedBytes, long totalBytes)
	{
		DownloadEvent event = null;
		for (DownloadListener listener: downloadListeners)
		{
			// lazy instantiation of the event
			if (event == null)
				event = new DownloadEvent(source, downloadName, downloadedBytes, totalBytes);
			
			// fire it
			listener.downloadProgressReport(event);
		}
	}

	protected void fireDownloadComplete(Object source, String downloadName, long downloadedBytes, long totalBytes)
	{
		DownloadEvent event = null;
		for (DownloadListener listener: downloadListeners)
		{
			// lazy instantiation of the event
			if (event == null)
				event = new DownloadEvent(source, downloadName, downloadedBytes, totalBytes);
			
			// fire it
			listener.downloadComplete(event);
		}
	}

	protected void fireDownloadFailed(Object source, String downloadName, long downloadedBytes, long totalBytes)
	{
		DownloadEvent event = null;
		for (DownloadListener listener: downloadListeners)
		{
			// lazy instantiation of the event
			if (event == null)
				event = new DownloadEvent(source, downloadName, downloadedBytes, totalBytes);
			
			// fire it
			listener.downloadFailed(event);
		}
	}
}
