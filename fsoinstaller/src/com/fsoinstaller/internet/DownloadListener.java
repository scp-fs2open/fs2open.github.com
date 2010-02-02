
package com.fsoinstaller.internet;

import java.util.EventListener;


public interface DownloadListener extends EventListener
{
	public void downloadNotNecessary(DownloadEvent event);

	public void downloadAboutToStart(DownloadEvent event);

	public void downloadProgressReport(DownloadEvent event);

	public void downloadComplete(DownloadEvent event);

	public void downloadFailed(DownloadEvent event);
}
