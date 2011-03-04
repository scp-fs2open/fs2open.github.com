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

import java.awt.EventQueue;


/**
 * A simple extension of Thread that ensures that tasks execute with standard
 * priority. This avoids the possibility that a new thread may be created from
 * the event thread and accidentally given the event thread's higher priority.
 * 
 * @see http://java.sun.com/developer/JDCTechTips/2005/tt0727.html#1
 * @author Goober5000
 */
public class WorkerThread extends Thread
{
	public WorkerThread()
	{
		super();

		if (EventQueue.isDispatchThread())
			setPriority(Thread.NORM_PRIORITY);
	}

	public WorkerThread(Runnable target, String name)
	{
		super(target, name);

		if (EventQueue.isDispatchThread())
			setPriority(Thread.NORM_PRIORITY);
	}

	public WorkerThread(Runnable target)
	{
		super(target);

		if (EventQueue.isDispatchThread())
			setPriority(Thread.NORM_PRIORITY);
	}

	public WorkerThread(String name)
	{
		super(name);

		if (EventQueue.isDispatchThread())
			setPriority(Thread.NORM_PRIORITY);
	}

	public WorkerThread(ThreadGroup group, Runnable target, String name, long stackSize)
	{
		super(group, target, name, stackSize);

		if (EventQueue.isDispatchThread())
			setPriority(Thread.NORM_PRIORITY);
	}

	public WorkerThread(ThreadGroup group, Runnable target, String name)
	{
		super(group, target, name);

		if (EventQueue.isDispatchThread())
			setPriority(Thread.NORM_PRIORITY);
	}

	public WorkerThread(ThreadGroup group, Runnable target)
	{
		super(group, target);

		if (EventQueue.isDispatchThread())
			setPriority(Thread.NORM_PRIORITY);
	}

	public WorkerThread(ThreadGroup group, String name)
	{
		super(group, name);

		if (EventQueue.isDispatchThread())
			setPriority(Thread.NORM_PRIORITY);
	}

	// not a parent constructor
	public WorkerThread(Runnable target, String name, int priority)
	{
		super(target, name);
		setPriority(priority);
	}

	// not a parent constructor
	public WorkerThread(Runnable target, int priority)
	{
		super(target);
		setPriority(priority);
	}
}
