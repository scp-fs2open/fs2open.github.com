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

package com.fsoinstaller.utils;

import java.awt.BorderLayout;
import java.awt.Cursor;
import java.awt.EventQueue;
import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.swing.BorderFactory;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JProgressBar;
import javax.swing.WindowConstants;
import javax.swing.border.BevelBorder;
import javax.swing.border.Border;

import com.fsoinstaller.main.FreeSpaceOpenInstaller;
import com.fsoinstaller.wizard.GUIConstants;


/**
 * A neat little class to run a task while displaying a progress bar indicating
 * time to completion. The task is specified as a Callable interface, and it has
 * the responsibilty of updating the dialog's progress as needed.
 */
public class ProgressBarDialog
{
	private static final Logger logger = Logger.getLogger(ProgressBarDialog.class);
	
	public static final String INDETERMINATE_STRING = "Working...";
	
	private final String text;
	private final String title;
	private final AtomicBoolean started;
	
	private volatile JProgressBar temp;
	private final JProgressBar progressBar;
	
	public ProgressBarDialog()
	{
		this(null, null);
	}
	
	public ProgressBarDialog(String text)
	{
		this(text, null);
	}
	
	public ProgressBarDialog(String text, String title)
	{
		if (title == null)
			title = FreeSpaceOpenInstaller.INSTALLER_TITLE;
		
		this.text = text;
		this.title = title;
		started = new AtomicBoolean(false);
		
		// bah, do this on the event dispatching thread
		// (we'd do this in runTask, except programmers may want to configure the bar before starting the task)
		MiscUtils.invokeAndWait(new Runnable()
		{
			public void run()
			{
				temp = new JProgressBar(0, 100);
				temp.setIndeterminate(true);
				temp.setString(INDETERMINATE_STRING);
				temp.setStringPainted(true);
			}
		});
		progressBar = temp;
	}
	
	/**
	 * Runs the specified task in a background thread. If an exception occurs
	 * while the task is running, it will be logged.
	 */
	public void runTask(Callable<Void> task)
	{
		runTask(task, null);
	}
	
	/**
	 * Runs the specified task in a background thread. If an exception occurs
	 * while the task is running, the callback will be invoked.
	 */
	public void runTask(final Callable<Void> task, final ExceptionCallback callback)
	{
		// see if we've ever called this already
		if (started.getAndSet(true))
			throw new IllegalStateException("A ProgressBarDialog can only run one task a single time!");
		
		// gui operations go on the event dispatching thread
		EventQueue.invokeLater(new Runnable()
		{
			public void run()
			{
				// create dialog
				JDialog dialog = new JDialog(MiscUtils.getActiveFrame(), title, true);
				dialog.setResizable(false);
				dialog.setUndecorated(true);
				
				// populate content pane
				JComponent contentPane = (JComponent) dialog.getContentPane();
				contentPane.setLayout(new BorderLayout());
				if (text != null)
					contentPane.add(new JLabel(text), BorderLayout.NORTH);
				contentPane.add(progressBar, BorderLayout.CENTER);
				
				// this approximates the look of a dialog border
				Border edge = BorderFactory.createEmptyBorder(1, 1, 0, 0);
				Border bevel = BorderFactory.createBevelBorder(BevelBorder.RAISED);
				Border margin = BorderFactory.createEmptyBorder(GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN, GUIConstants.DEFAULT_MARGIN);
				contentPane.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createCompoundBorder(edge, bevel), margin));
				
				// configure display settings
				dialog.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
				dialog.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
				dialog.pack();
				MiscUtils.centerWindowOnParent(dialog, MiscUtils.getActiveFrame());
				
				// start the task and display the dialog
				new ProgressBarTask(task, callback, text, dialog).execute();
				dialog.setVisible(true);
			}
		});
	}
	
	public void setIndeterminate(final boolean indeterminate)
	{
		logger.debug("Task is now " + (indeterminate ? "indeterminate" : "determinate"));
		
		EventQueue.invokeLater(new Runnable()
		{
			public void run()
			{
				progressBar.setIndeterminate(indeterminate);
				progressBar.setString(indeterminate ? INDETERMINATE_STRING : null);
			}
		});
	}
	
	public void setPercentComplete(int percent)
	{
		if (percent < 0)
			percent = 0;
		if (percent > 100)
			percent = 100;
		logger.debug("Progress: " + percent + "%");
		
		final int _percent = percent;
		EventQueue.invokeLater(new Runnable()
		{
			public void run()
			{
				progressBar.setValue(_percent);
			}
		});
	}
	
	public void setRatioComplete(double ratio)
	{
		setPercentComplete((int) (ratio * 100.0));
	}
	
	private static final class ProgressBarTask
	{
		private final Callable<Void> task;
		private final ExceptionCallback callback;
		
		private final String description;
		private final JDialog dialog;
		
		public ProgressBarTask(Callable<Void> task, ExceptionCallback callback, String description, JDialog dialog)
		{
			this.task = task;
			this.callback = callback;
			
			this.description = description;
			this.dialog = dialog;
		}
		
		public void execute()
		{
			WorkerThread thread = new WorkerThread(new Runnable()
			{
				public void run()
				{
					Exception exception = null;
					try
					{
						logger.info("Running task: '" + description + "'");
						task.call();
					}
					catch (Exception e)
					{
						logger.error("The task aborted because of an exception!", e);
						exception = e;
					}
					
					final Exception _exception = exception;
					EventQueue.invokeLater(new Runnable()
					{
						public void run()
						{
							logger.info("Finished task: '" + description + "'");
							dialog.dispose();
							
							// handle any exceptions
							if (_exception != null && callback != null)
								callback.handleException(_exception);
						}
					});
				}
			}, "ProgressBarDialogThread", Thread.NORM_PRIORITY);
			thread.start();
		}
	}
	
	public static interface ExceptionCallback
	{
		public void handleException(Exception exception);
	}
}
