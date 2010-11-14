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

package com.fsoinstaller.main;

import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;

import org.apache.log4j.Logger;

import com.fsoinstaller.wizard.InstallerWizard;

public class FreeSpaceOpenInstaller
{
	private static Logger logger = Logger.getLogger(FreeSpaceOpenInstaller.class);
	
	private FreeSpaceOpenInstaller()
	{
	}
	
	public static void main(String[] args)
	{
		logger.debug("Setting look-and-feel...");
		try
		{
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		}
		catch (ClassNotFoundException cnfe)
		{
			logger.error("Error setting look-and-feel!", cnfe);
		}
		catch (InstantiationException ie)
		{
			logger.error("Error setting look-and-feel!", ie);
		}
		catch (IllegalAccessException iae)
		{
			logger.error("Error setting look-and-feel!", iae);
		}
		catch (UnsupportedLookAndFeelException iae)
		{
			logger.error("Error setting look-and-feel!", iae);
		}
		
		// for now, we only have one possible operation: going through the wizard
		logger.debug("Launching wizard...");
		InstallerWizard wizard = new InstallerWizard();
		wizard.launch();
		
		// later we'll evaluate the runtime args to launch the txtfile builder, etc.
	}
}
