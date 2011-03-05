/*
 * File: JarClassLoader.java
 * 
 * Copyright (C) 2008-2011 JDotSoft. All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 * 
 * Visit jdotsoft.com for commercial license.
 * 
 * $Id: JarClassLoader.java,v 1.29 2011/01/27 22:54:09 mg Exp $
 */

package com.jdotsoft.jarloader;

import com.jdotsoft.jarloader.JarClassLoader;


public class JarLauncher
{
	public static void main(String[] args) throws Throwable
	{
		JarClassLoader jcl = new JarClassLoader();
		jcl.invokeMain("com.fsoinstaller.main.FreeSpaceOpenInstaller", args);
	}
}
