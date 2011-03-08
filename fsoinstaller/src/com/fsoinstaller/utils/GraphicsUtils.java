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

import java.awt.Graphics;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.HeadlessException;
import java.awt.Image;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.awt.image.PixelGrabber;

import javax.swing.ImageIcon;


/**
 * Utility methods for working with graphics.
 * 
 * @author Goober5000
 */
public class GraphicsUtils
{
	private GraphicsUtils()
	{
	}
	
	/**
	 * Adapted from http://javaalmanac.com/egs/java.awt.image/Image2Buf.html
	 */
	public static BufferedImage createBufferedImage(int width, int height, int transparencyType)
	{
		// Create a buffered image with a format that's compatible with the screen
		BufferedImage bimage = null;
		GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
		try
		{
			// Create the buffered image
			GraphicsDevice gs = ge.getDefaultScreenDevice();
			GraphicsConfiguration gc = gs.getDefaultConfiguration();
			bimage = gc.createCompatibleImage(width, height, transparencyType);
		}
		catch (HeadlessException he)
		{
			// The system does not have a screen?
			throw new UnsupportedOperationException("The system is headless!", he);
		}
		
		// Create a buffered image using the default color model, if the first method failed
		if (bimage == null)
		{
			int bufferedImageType = (transparencyType == Transparency.OPAQUE) ? BufferedImage.TYPE_INT_RGB : BufferedImage.TYPE_INT_ARGB;
			
			// Create the buffered image
			bimage = new BufferedImage(width, height, bufferedImageType);
		}
		
		return bimage;
	}
	
	/**
	 * Adapted from http://javaalmanac.com/egs/java.awt.image/HasAlpha.html
	 */
	public static boolean hasAlpha(Image image)
	{
		// If buffered image, the color model is readily available
		if (image instanceof BufferedImage)
		{
			return ((BufferedImage) image).getColorModel().hasAlpha();
		}
		
		// Use a pixel grabber to retrieve the image's color model;
		// grabbing a single pixel is usually sufficient
		PixelGrabber pg = new PixelGrabber(image, 0, 0, 1, 1, false);
		boolean result = false;
		try
		{
			result = pg.grabPixels();
		}
		catch (InterruptedException ie)
		{
			// bleh, restore the interrupt
			Thread.currentThread().interrupt();
		}
		
		// Get the image's color model; assume no alpha on failure
		return result && pg.getColorModel().hasAlpha();
	}
	
	public static BufferedImage getBufferedImage(Image image)
	{
		return getBufferedImage(image, false);
	}
	
	/**
	 * Adapted from http://javaalmanac.com/egs/java.awt.image/Image2Buf.html
	 */
	public static BufferedImage getBufferedImage(Image image, boolean addAlpha)
	{
		if (image == null)
			return null;
		
		if (image instanceof BufferedImage)
		{
			if (!addAlpha || hasAlpha(image))
				return (BufferedImage) image;
		}
		
		// this code ensures that all the pixels in the image are loaded
		image = new ImageIcon(image).getImage();
		
		// determine our transparency type
		int transparencyType = (addAlpha || hasAlpha(image)) ? Transparency.TRANSLUCENT : Transparency.OPAQUE;
		
		// create a buffered image and space to paint it
		BufferedImage bimage = createBufferedImage(image.getWidth(null), image.getHeight(null), transparencyType);
		Graphics g = bimage.createGraphics();
		
		// paint the image onto the buffered image
		g.drawImage(image, 0, 0, null);
		g.dispose();
		
		return bimage;
	}
}
