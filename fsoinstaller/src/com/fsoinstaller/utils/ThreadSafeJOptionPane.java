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

import java.awt.Component;
import java.awt.EventQueue;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import javax.swing.Icon;
import javax.swing.JOptionPane;


/**
 * A class which wraps the static JOptionPane.show() methods, so that they can
 * be called from threads other than the event-dispatching thread.
 * 
 * @author Goober5000
 */
public final class ThreadSafeJOptionPane
{
	private ThreadSafeJOptionPane()
	{
	}
	
	public static int showConfirmDialog(final Component parentComponent, final Object message)
	{
		final AtomicInteger value = new AtomicInteger();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showConfirmDialog(parentComponent, message));
			}
		});
		return value.get();
	}
	
	public static int showConfirmDialog(final Component parentComponent, final Object message, final String title, final int optionType)
	{
		final AtomicInteger value = new AtomicInteger();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showConfirmDialog(parentComponent, message, title, optionType));
			}
		});
		return value.get();
	}
	
	public static int showConfirmDialog(final Component parentComponent, final Object message, final String title, final int optionType, final int messageType)
	{
		final AtomicInteger value = new AtomicInteger();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showConfirmDialog(parentComponent, message, title, optionType, messageType));
			}
		});
		return value.get();
	}
	
	public static int showConfirmDialog(final Component parentComponent, final Object message, final String title, final int optionType, final int messageType, final Icon icon)
	{
		final AtomicInteger value = new AtomicInteger();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showConfirmDialog(parentComponent, message, title, optionType, messageType, icon));
			}
		});
		return value.get();
	}
	
	public static String showInputDialog(final Object message)
	{
		final AtomicReference<String> value = new AtomicReference<String>();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInputDialog(message));
			}
		});
		return value.get();
	}
	
	public static String showInputDialog(final Object message, final Object initialSelectionValue)
	{
		final AtomicReference<String> value = new AtomicReference<String>();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInputDialog(message, initialSelectionValue));
			}
		});
		return value.get();
	}
	
	public static String showInputDialog(final Component parentComponent, final Object message)
	{
		final AtomicReference<String> value = new AtomicReference<String>();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInputDialog(parentComponent, message));
			}
		});
		return value.get();
	}
	
	public static String showInputDialog(final Component parentComponent, final Object message, final Object initialSelectionValue)
	{
		final AtomicReference<String> value = new AtomicReference<String>();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInputDialog(parentComponent, message, initialSelectionValue));
			}
		});
		return value.get();
	}
	
	public static String showInputDialog(final Component parentComponent, final Object message, final String title, final int messageType)
	{
		final AtomicReference<String> value = new AtomicReference<String>();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInputDialog(parentComponent, message, title, messageType));
			}
		});
		return value.get();
	}
	
	public static Object showInputDialog(final Component parentComponent, final Object message, final String title, final int messageType, final Icon icon, final Object[] selectionValues, final Object initialSelectionValue)
	{
		final AtomicReference<Object> value = new AtomicReference<Object>();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInputDialog(parentComponent, message, title, messageType, icon, selectionValues, initialSelectionValue));
			}
		});
		return value.get();
	}
	
	public static void showMessageDialog(final Component parentComponent, final Object message)
	{
		EventQueue.invokeLater(new Runnable()
		{
			@Override
			public void run()
			{
				JOptionPane.showMessageDialog(parentComponent, message);
			}
		});
	}
	
	public static void showMessageDialog(final Component parentComponent, final Object message, final String title, final int messageType)
	{
		EventQueue.invokeLater(new Runnable()
		{
			@Override
			public void run()
			{
				JOptionPane.showMessageDialog(parentComponent, message, title, messageType);
			}
		});
	}
	
	public static void showMessageDialog(final Component parentComponent, final Object message, final String title, final int messageType, final Icon icon)
	{
		EventQueue.invokeLater(new Runnable()
		{
			@Override
			public void run()
			{
				JOptionPane.showMessageDialog(parentComponent, message, title, messageType, icon);
			}
		});
	}
	
	public static int showOptionDialog(final Component parentComponent, final Object message, final String title, final int optionType, final int messageType, final Icon icon, final Object[] options, final Object initialValue)
	{
		final AtomicInteger value = new AtomicInteger();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showOptionDialog(parentComponent, message, title, optionType, messageType, icon, options, initialValue));
			}
		});
		return value.get();
	}
	
	public static int showInternalConfirmDialog(final Component parentComponent, final Object message)
	{
		final AtomicInteger value = new AtomicInteger();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInternalConfirmDialog(parentComponent, message));
			}
		});
		return value.get();
	}
	
	public static int showInternalConfirmDialog(final Component parentComponent, final Object message, final String title, final int optionType)
	{
		final AtomicInteger value = new AtomicInteger();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInternalConfirmDialog(parentComponent, message, title, optionType));
			}
		});
		return value.get();
	}
	
	public static int showInternalConfirmDialog(final Component parentComponent, final Object message, final String title, final int optionType, final int messageType)
	{
		final AtomicInteger value = new AtomicInteger();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInternalConfirmDialog(parentComponent, message, title, optionType, messageType));
			}
		});
		return value.get();
	}
	
	public static int showInternalConfirmDialog(final Component parentComponent, final Object message, final String title, final int optionType, final int messageType, final Icon icon)
	{
		final AtomicInteger value = new AtomicInteger();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInternalConfirmDialog(parentComponent, message, title, optionType, messageType, icon));
			}
		});
		return value.get();
	}
	
	public static String showInternalInputDialog(final Component parentComponent, final Object message)
	{
		final AtomicReference<String> value = new AtomicReference<String>();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInternalInputDialog(parentComponent, message));
			}
		});
		return value.get();
	}
	
	public static String showInternalInputDialog(final Component parentComponent, final Object message, final String title, final int messageType)
	{
		final AtomicReference<String> value = new AtomicReference<String>();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInternalInputDialog(parentComponent, message, title, messageType));
			}
		});
		return value.get();
	}
	
	public static Object showInternalInputDialog(final Component parentComponent, final Object message, final String title, final int messageType, final Icon icon, final Object[] selectionValues, final Object initialSelectionValue)
	{
		final AtomicReference<Object> value = new AtomicReference<Object>();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInternalInputDialog(parentComponent, message, title, messageType, icon, selectionValues, initialSelectionValue));
			}
		});
		return value.get();
	}
	
	public static void showInternalMessageDialog(final Component parentComponent, final Object message)
	{
		EventQueue.invokeLater(new Runnable()
		{
			@Override
			public void run()
			{
				JOptionPane.showInternalMessageDialog(parentComponent, message);
			}
		});
	}
	
	public static void showInternalMessageDialog(final Component parentComponent, final Object message, final String title, final int messageType)
	{
		EventQueue.invokeLater(new Runnable()
		{
			@Override
			public void run()
			{
				JOptionPane.showInternalMessageDialog(parentComponent, message, title, messageType);
			}
		});
	}
	
	public static void showInternalMessageDialog(final Component parentComponent, final Object message, final String title, final int messageType, final Icon icon)
	{
		EventQueue.invokeLater(new Runnable()
		{
			@Override
			public void run()
			{
				JOptionPane.showInternalMessageDialog(parentComponent, message, title, messageType, icon);
			}
		});
	}
	
	public static int showInternalOptionDialog(final Component parentComponent, final Object message, final String title, final int optionType, final int messageType, final Icon icon, final Object[] options, final Object initialValue)
	{
		final AtomicInteger value = new AtomicInteger();
		MiscUtils.invokeAndWait(new Runnable()
		{
			@Override
			public void run()
			{
				value.set(JOptionPane.showInternalOptionDialog(parentComponent, message, title, optionType, messageType, icon, options, initialValue));
			}
		});
		return value.get();
	}
}
