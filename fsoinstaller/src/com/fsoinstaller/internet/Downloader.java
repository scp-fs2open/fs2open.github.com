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
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import net.sf.sevenzipjbinding.ArchiveFormat;
import net.sf.sevenzipjbinding.ExtractAskMode;
import net.sf.sevenzipjbinding.ExtractOperationResult;
import net.sf.sevenzipjbinding.IArchiveExtractCallback;
import net.sf.sevenzipjbinding.ISequentialOutStream;
import net.sf.sevenzipjbinding.ISevenZipInArchive;
import net.sf.sevenzipjbinding.PropID;
import net.sf.sevenzipjbinding.SevenZip;
import net.sf.sevenzipjbinding.SevenZipException;

import com.fsoinstaller.common.InputStreamInStream;
import com.fsoinstaller.common.InputStreamSource;
import com.fsoinstaller.common.OutputStreamSequentialOutStream;
import com.fsoinstaller.utils.Logger;


/**
 * A utility for downloading installation files from the Internet, either
 * directly or from within a compressed archive. Rewritten from a similar class
 * originally created by Turey.
 * <p>
 * This class should be thread-safe. For the most part (i.e. except for the
 * listeners), it is also immutable.
 * 
 * @author Turey
 * @author Goober5000
 */
public class Downloader
{
	private static final Logger logger = Logger.getLogger(Downloader.class);

	protected static final int BUFFER_SIZE = 2048;
	protected static final byte[] downloadBuffer = new byte[BUFFER_SIZE];

	protected final List<DownloadListener> downloadListeners;
	protected final Connector connector;

	public Downloader(Connector connector)
	{
		this.connector = connector;
		this.downloadListeners = Collections.synchronizedList(new LinkedList<DownloadListener>());
	}

	public boolean download(URL sourceURL, File destinationDirectory)
	{
		String fileName = new File(sourceURL.getPath()).getName();
		int periodPos = fileName.lastIndexOf('.');
		String extension = (periodPos >= 0) ? fileName.substring(periodPos + 1) : "";

		// download a zip
		if (extension.equalsIgnoreCase("zip"))
			return downloadFromZip(sourceURL, destinationDirectory);

		// download another supported archive
		for (ArchiveFormat format: ArchiveFormat.values())
		{
			if (format.getMethodName().equalsIgnoreCase(extension))
				return downloadFromArchive(sourceURL, destinationDirectory, format);
		}

		// download as a standard file
		return downloadFile(sourceURL, new File(destinationDirectory, fileName));
	}

	public boolean downloadFile(URL sourceURL, File destinationFile)
	{
		logger.info("Downloading from " + sourceURL + " to local file " + destinationFile);

		long totalBytes = 0;
		long lastModified = -1;
		InputStream inputStream = null;
		OutputStream outputStream = null;
		try
		{
			logger.debug("Opening connection...");
			URLConnection connection = connector.openConnection(sourceURL);
			totalBytes = connection.getContentLength();
			lastModified = connection.getLastModified();

			logger.debug("Checking if the file is up to date...");
			if (uptodate(destinationFile, totalBytes))
			{
				fireNoDownloadNecessary(destinationFile.getName(), 0, totalBytes);
				return true;
			}

			logger.debug("Opening input and output streams...");
			inputStream = connection.getInputStream();
			outputStream = openOutputStream(destinationFile);

			downloadUsingStreams(inputStream, outputStream, destinationFile.getName(), totalBytes);

			logger.debug("Closing output stream...");
			outputStream.close();
			outputStream = null;
			if (lastModified > 0)
				destinationFile.setLastModified(lastModified);

			logger.debug("Closing input stream...");
			inputStream.close();
			inputStream = null;

			return true;
		}
		catch (IOException ioe)
		{
			logger.error("An exception was thrown during download!", ioe);
			fireDownloadFailed(destinationFile.getName(), 0, totalBytes, ioe);

			return false;
		}
		finally
		{
			cleanup(inputStream, outputStream);
		}
	}

	public boolean downloadFromZip(URL sourceURL, File destinationDirectory)
	{
		logger.info("Downloading and extracting from " + sourceURL + " to local directory " + destinationDirectory);

		String currentEntry = "";
		long totalBytes = 0;
		long lastModified = -1;
		ZipInputStream zipInputStream = null;
		OutputStream outputStream = null;
		try
		{
			logger.debug("Opening connection...");
			URLConnection connection = connector.openConnection(sourceURL);
			totalBytes = connection.getContentLength();

			logger.debug("Opening input stream...");
			zipInputStream = new ZipInputStream(new BufferedInputStream(connection.getInputStream()));

			ZipEntry entry;
			while ((entry = zipInputStream.getNextEntry()) != null)
			{
				currentEntry = entry.getName();
				totalBytes = entry.getSize();
				lastModified = entry.getTime();

				logger.debug("Checking entry '" + currentEntry + "'");
				if (entry.isDirectory())
				{
					zipInputStream.closeEntry();
					continue;
				}

				logger.debug("Checking if the file is up to date...");
				File destinationFile = new File(destinationDirectory, currentEntry);
				if (uptodate(destinationFile, totalBytes))
				{
					fireNoDownloadNecessary(destinationFile.getName(), 0, totalBytes);
					zipInputStream.closeEntry();
					continue;
				}

				logger.debug("Opening output stream...");
				outputStream = openOutputStream(destinationFile);

				downloadUsingStreams(zipInputStream, outputStream, currentEntry, totalBytes);

				logger.debug("Closing output stream...");
				outputStream.close();
				outputStream = null;
				if (lastModified > 0)
					destinationFile.setLastModified(lastModified);

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
			fireDownloadFailed(currentEntry, 0, totalBytes, ioe);

			return false;
		}
		finally
		{
			cleanup(zipInputStream, outputStream);
		}
	}

	public boolean downloadFromArchive(URL sourceURL, File destinationDirectory, ArchiveFormat format)
	{
		logger.info("Downloading and extracting from " + sourceURL + " to local directory " + destinationDirectory);

		String currentEntry = "";
		long totalBytes = 0;
		ISevenZipInArchive archive = null;
		InputStreamInStream inStream = null;
		IArchiveExtractCallback callback = null;
		try
		{
			logger.debug("Opening connection...");
			URLConnection connection = connector.openConnection(sourceURL);
			totalBytes = connection.getContentLength();

			logger.debug("Opening archive...");
			inStream = new InputStreamInStream(getInputStreamSource(connector, sourceURL, connection), totalBytes);
			archive = SevenZip.openInArchive(format, inStream);
			int numItems = archive.getNumberOfItems();

			List<Integer> extractionIndexes = new ArrayList<Integer>();
			String[] archiveEntries = new String[numItems];
			long[] archiveSizes = new long[numItems];
			long[] archiveModifiedTimes = new long[numItems];

			for (int item = 0; item < numItems; item++)
			{
				currentEntry = archiveEntries[item] = archive.getStringProperty(item, PropID.PATH);
				totalBytes = archiveSizes[item] = (Long) archive.getProperty(item, PropID.SIZE);
				Date lastModifiedDate = (Date) archive.getProperty(item, PropID.LAST_WRITE_TIME);
				archiveModifiedTimes[item] = lastModifiedDate == null ? -1 : lastModifiedDate.getTime();

				logger.debug("Checking entry '" + currentEntry + "'");
				if ((Boolean) archive.getProperty(item, PropID.IS_FOLDER))
				{
					continue;
				}

				logger.debug("Checking if the file is up to date...");
				File destinationFile = new File(destinationDirectory, currentEntry);
				if (uptodate(destinationFile, totalBytes))
				{
					fireNoDownloadNecessary(destinationFile.getName(), 0, totalBytes);
					continue;
				}

				// we will extract this item
				extractionIndexes.add(item);
			}

			if (extractionIndexes.size() > 0)
			{
				logger.debug("Opening extractor...");
				callback = getExtractCallback(destinationDirectory, archiveEntries, archiveSizes, archiveModifiedTimes);

				// extract them all at once
				int[] items = new int[extractionIndexes.size()];
				for (int i = 0; i < extractionIndexes.size(); i++)
					items[i] = extractionIndexes.get(i);
				archive.extract(items, false, callback);
			}

			logger.debug("Closing archive...");
			archive.close();
			archive = null;

			logger.debug("Closing input stream...");
			inStream.close();
			inStream = null;

			return true;
		}
		catch (SevenZipException sze)
		{
			logger.error("An exception was thrown during download!", sze);
			fireDownloadFailed(currentEntry, 0, totalBytes, sze);

			return false;
		}
		catch (IOException ioe)
		{
			logger.error("An exception was thrown during download!", ioe);
			fireDownloadFailed(currentEntry, 0, totalBytes, ioe);

			return false;
		}
		finally
		{
			cleanup(archive, inStream);
		}
	}

	protected OutputStream openOutputStream(File file) throws IOException
	{
		logger.debug("output file: " + file.getAbsolutePath());

		if (!file.getParentFile().exists())
		{
			logger.debug("parent directory not found; creating it");
			if (!file.getParentFile().mkdirs())
				throw new IOException("Failed to create parent directory for '" + file.getAbsolutePath() + "'");
		}

		if (!file.exists())
		{
			logger.debug("local file not found; creating it");
			if (!file.createNewFile())
				throw new IOException("Failed to create new file '" + file.getAbsolutePath() + "'!");
		}

		return new BufferedOutputStream(new FileOutputStream(file));
	}

	protected InputStreamSource getInputStreamSource(Connector connector, URL sourceURL, URLConnection firstConnection)
	{
		final Connector _connector = connector;
		final URL _sourceURL = sourceURL;
		final URLConnection _firstConnection = firstConnection;

		return new InputStreamSource()
		{
			private boolean ever_recycled = false;

			@Override
			public InputStream recycleInputStream(InputStream oldInputStream) throws IOException
			{
				if (oldInputStream != null)
				{
					try
					{
						logger.debug("Closing old input stream...");
						oldInputStream.close();
					}
					catch (IOException ioe)
					{
						logger.warn("Could not close download stream!", ioe);
					}
				}

				// this is because we need to open the connection once, before recycling,
				// to get the length in bytes
				URLConnection connection;
				if (ever_recycled)
				{
					logger.debug("Opening connection...");
					connection = _connector.openConnection(_sourceURL);
				}
				else
				{
					connection = _firstConnection;
					ever_recycled = true;
				}

				logger.debug("Opening new input stream...");
				return connection.getInputStream();
			}
		};
	}

	protected IArchiveExtractCallback getExtractCallback(File destinationDirectory, String[] archiveEntries, long[] archiveSizes, long[] archiveModifiedTimes)
	{
		final File _destinationDirectory = destinationDirectory;
		final String[] _archiveEntries = archiveEntries;
		final long[] _archiveSizes = archiveSizes;
		final long[] _archiveModifiedTimes = archiveModifiedTimes;

		return new IArchiveExtractCallback()
		{
			private int currentIndex = -1;
			private long currentCompletionValue = 0;
			private File currentFile = null;
			private OutputStreamSequentialOutStream currentOutStream = null;

			@Override
			public ISequentialOutStream getStream(int index, ExtractAskMode extractAskMode) throws SevenZipException
			{
				switch (extractAskMode)
				{
					case EXTRACT:
						try
						{
							logger.debug("Opening output stream...");
							currentIndex = index;
							currentCompletionValue = 0;
							currentFile = new File(_destinationDirectory, _archiveEntries[index]);
							currentOutStream = new OutputStreamSequentialOutStream(openOutputStream(currentFile));
						}
						catch (IOException ioe)
						{
							throw new SevenZipException("Error opening output stream", ioe);
						}
						break;

					case TEST:
						throw new UnsupportedOperationException("Testing of archives not supported");

					case SKIP:
						currentIndex = -1;
						currentCompletionValue = 0;
						currentFile = null;
						currentOutStream = null;
						break;

					default:
						throw new IllegalArgumentException("Unknown ask mode");
				}

				return currentOutStream;
			}

			@Override
			public void prepareOperation(ExtractAskMode extractAskMode) throws SevenZipException
			{
				if (extractAskMode == ExtractAskMode.EXTRACT)
				{
					logger.debug("Downloading...");
					fireAboutToStart(_archiveEntries[currentIndex], currentCompletionValue, _archiveSizes[currentIndex]);
				}
			}

			@Override
			public void setOperationResult(ExtractOperationResult extractOperationResult) throws SevenZipException
			{
				switch (extractOperationResult)
				{
					case OK:
						if (currentIndex >= 0)
						{
							logger.debug("Download complete");
							fireDownloadComplete(_archiveEntries[currentIndex], currentCompletionValue, _archiveSizes[currentIndex]);
						}
						break;

					case UNSUPPORTEDMETHOD:
						logger.warn("Extraction failed due to unknown compression method!");
						if (currentIndex >= 0)
							fireDownloadFailed(_archiveEntries[currentIndex], currentCompletionValue, _archiveSizes[currentIndex], new SevenZipException("Unknown compression method"));
						break;

					case DATAERROR:
						logger.warn("Extraction failed due to data error!");
						if (currentIndex >= 0)
							fireDownloadFailed(_archiveEntries[currentIndex], currentCompletionValue, _archiveSizes[currentIndex], new SevenZipException("Data error"));
						break;

					case CRCERROR:
						logger.warn("Extraction failed due to CRC error!");
						if (currentIndex >= 0)
							fireDownloadFailed(_archiveEntries[currentIndex], currentCompletionValue, _archiveSizes[currentIndex], new SevenZipException("CRC error"));
						break;

					default:
						throw new IllegalArgumentException("Unknown operation result");
				}

				if (currentIndex >= 0)
				{
					try
					{
						logger.debug("Closing output stream...");
						currentOutStream.close();
						if (_archiveModifiedTimes[currentIndex] > 0)
							currentFile.setLastModified(_archiveModifiedTimes[currentIndex]);
					}
					catch (IOException ioe)
					{
						logger.warn("Could not close file stream!", ioe);
					}
					finally
					{
						currentIndex = -1;
						currentCompletionValue = 0;
						currentFile = null;
						currentOutStream = null;
					}
				}
			}

			@Override
			public void setCompleted(long completeValue) throws SevenZipException
			{
				if (currentIndex >= 0)
				{
					currentCompletionValue = completeValue;
					fireProgressReport(_archiveEntries[currentIndex], currentCompletionValue, _archiveSizes[currentIndex]);
				}
			}

			@Override
			public void setTotal(long total) throws SevenZipException
			{
				if (currentIndex >= 0)
				{
					if (total != _archiveSizes[currentIndex])
						logger.error("Callback total of " + total + " does not agree with entry size of " + _archiveSizes[currentIndex] + "!");
				}
			}
		};
	}

	protected void downloadUsingStreams(InputStream inputStream, OutputStream outputStream, String downloadName, long downloadTotalSize) throws IOException
	{
		long totalBytesWritten = 0;

		logger.debug("Downloading...");
		fireAboutToStart(downloadName, totalBytesWritten, downloadTotalSize);

		int bytesRead = 0;
		while ((bytesRead = inputStream.read(downloadBuffer)) != -1)
		{
			outputStream.write(downloadBuffer, 0, bytesRead);
			totalBytesWritten += bytesRead;

			fireProgressReport(downloadName, totalBytesWritten, downloadTotalSize);
		}

		logger.debug("Download complete");
		fireDownloadComplete(downloadName, totalBytesWritten, downloadTotalSize);
	}

	protected boolean uptodate(File destinationFile, long totalBytes)
	{
		return destinationFile.exists() && (totalBytes > 0) && (destinationFile.length() == totalBytes);
	}

	protected void cleanup(InputStream inputStream, OutputStream outputStream)
	{
		if (outputStream != null)
		{
			try
			{
				outputStream.close();
			}
			catch (IOException ioe)
			{
				logger.warn("Could not close file stream!", ioe);
			}
		}

		if (inputStream != null)
		{
			try
			{
				inputStream.close();
			}
			catch (IOException ioe)
			{
				logger.warn("Could not close download stream!", ioe);
			}
		}
	}

	protected void cleanup(ISevenZipInArchive archive, InputStreamInStream inStream)
	{
		if (archive != null)
		{
			try
			{
				archive.close();
			}
			catch (SevenZipException sze)
			{
				logger.warn("Could not close archive!", sze);
			}
		}

		if (inStream != null)
		{
			try
			{
				inStream.close();
			}
			catch (IOException ioe)
			{
				logger.warn("Could not close download stream!", ioe);
			}
		}
	}

	public void addDownloadListener(DownloadListener listener)
	{
		downloadListeners.add(listener);
	}

	public void removeDownloadListener(DownloadListener listener)
	{
		downloadListeners.remove(listener);
	}

	protected void fireNoDownloadNecessary(String downloadName, long downloadedBytes, long totalBytes)
	{
		synchronized (downloadListeners)
		{
			DownloadEvent event = null;
			for (DownloadListener listener: downloadListeners)
			{
				// lazy instantiation of the event
				if (event == null)
					event = new DownloadEvent(this, downloadName, downloadedBytes, totalBytes);

				// fire it
				listener.downloadNotNecessary(event);
			}
		}
	}

	protected void fireAboutToStart(String downloadName, long downloadedBytes, long totalBytes)
	{
		synchronized (downloadListeners)
		{
			DownloadEvent event = null;
			for (DownloadListener listener: downloadListeners)
			{
				// lazy instantiation of the event
				if (event == null)
					event = new DownloadEvent(this, downloadName, downloadedBytes, totalBytes);

				// fire it
				listener.downloadAboutToStart(event);
			}
		}
	}

	protected void fireProgressReport(String downloadName, long downloadedBytes, long totalBytes)
	{
		synchronized (downloadListeners)
		{
			DownloadEvent event = null;
			for (DownloadListener listener: downloadListeners)
			{
				// lazy instantiation of the event
				if (event == null)
					event = new DownloadEvent(this, downloadName, downloadedBytes, totalBytes);

				// fire it
				listener.downloadProgressReport(event);
			}
		}
	}

	protected void fireDownloadComplete(String downloadName, long downloadedBytes, long totalBytes)
	{
		synchronized (downloadListeners)
		{
			DownloadEvent event = null;
			for (DownloadListener listener: downloadListeners)
			{
				// lazy instantiation of the event
				if (event == null)
					event = new DownloadEvent(this, downloadName, downloadedBytes, totalBytes);

				// fire it
				listener.downloadComplete(event);
			}
		}
	}

	protected void fireDownloadFailed(String downloadName, long downloadedBytes, long totalBytes, Exception exception)
	{
		synchronized (downloadListeners)
		{
			DownloadEvent event = null;
			for (DownloadListener listener: downloadListeners)
			{
				// lazy instantiation of the event
				if (event == null)
					event = new DownloadEvent(this, downloadName, downloadedBytes, totalBytes, exception);

				// fire it
				listener.downloadFailed(event);
			}
		}
	}
}
