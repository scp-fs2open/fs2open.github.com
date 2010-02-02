
package com.fsoinstaller.internet;

import java.util.EventObject;


public class DownloadEvent extends EventObject
{
	protected String downloadName;
	protected long downloadedBytes;
	protected long totalBytes;

	public DownloadEvent(Object source, String downloadName, long downloadedBytes, long totalBytes)
	{
		super(source);
		this.downloadName = downloadName;
		this.downloadedBytes = downloadedBytes;
		this.totalBytes = totalBytes;
	}

	public String getDownloadName()
	{
		return downloadName;
	}

	public long getDownloadedBytes()
	{
		return downloadedBytes;
	}

	public long getTotalBytes()
	{
		return totalBytes;
	}
}
