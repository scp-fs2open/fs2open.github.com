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

package com.fsoinstaller.wizard;

import java.awt.Color;
import java.awt.Dimension;
import java.util.List;

import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JTextField;

import org.apache.log4j.Logger;
import org.ciscavate.cjwizard.PageFactory;
import org.ciscavate.cjwizard.WizardPage;
import org.ciscavate.cjwizard.WizardSettings;


public class InstallerPageFactory implements PageFactory
{
	private static final Logger logger = Logger.getLogger(InstallerPageFactory.class);
	
	public InstallerPageFactory()
	{
	}

    // To keep things simple, we'll just create an array of wizard pages:
    private final WizardPage[] pages = {
          new WizardPage("One", "First Page"){
             // this is an instance initializer -- it's a constructor for
             // an anonymous class.  WizardPages don't need to be anonymous,
             // of course.  It just makes the demo fit in one file if we do it
             // this way:
             {
                JTextField field = new JTextField();
                // set a name on any component that you want to collect values
                // from.  Be sure to do this *before* adding the component to
                // the WizardPage.
                field.setName("testField");
                field.setPreferredSize(new Dimension(50, 20));
                add(new JLabel("One!"));
                add(field);
             }
          },
          new WizardPage("Two", "Second Page"){
             {
                JCheckBox box = new JCheckBox("testBox");
                box.setName("box");
                add(new JLabel("Two!"));
                add(box);
             }

             /* (non-Javadoc)
              * @see org.ciscavate.cjwizard.WizardPage#updateSettings(org.ciscavate.cjwizard.WizardSettings)
              */
             @Override
             public void updateSettings(WizardSettings settings) {
                super.updateSettings(settings);
                
                // This is called when the user clicks next, so we could do
                // some longer-running processing here if we wanted to, and
                // pop up progress bars, etc.  Once this method returns, the
                // wizard will continue.  Beware though, this runs in the
                // event dispatch thread (EDT), and may render the UI
                // unresponsive if you don't issue a new thread for any long
                // running ops.  Future versions will offer a better way of
                // doing this.
             }
             
          },
          new WizardPage("Three", "Third Page"){
             {
                add(new JLabel("Three!"));
                setBackground(Color.green);
             }

             /**
              * This is the last page in the wizard, so we will enable the finish
              * button and disable the "Next >" button just before the page is
              * displayed:
              */
             public void rendering(List<WizardPage> path, WizardSettings settings) {
                super.rendering(path, settings);
                setFinishEnabled(true);
                setNextEnabled(false);
             }
          }
    };
    
    
    /* (non-Javadoc)
     * @see org.ciscavate.cjwizard.PageFactory#createPage(java.util.List, org.ciscavate.cjwizard.WizardSettings)
     */
    @Override
    public WizardPage createPage(List<WizardPage> path,
          WizardSettings settings) {
       logger.debug("creating page "+path.size());
       
       // Get the next page to display.  The path is the list of all wizard
       // pages that the user has proceeded through from the start of the
       // wizard, so we can easily see which step the user is on by taking
       // the length of the path.  This makes it trivial to return the next
       // WizardPage:
       WizardPage page = pages[path.size()];
       
       // if we wanted to, we could use the WizardSettings object like a
       // Map<String, Object> to change the flow of the wizard pages.
       // In fact, we can do arbitrarily complex computation to determine
       // the next wizard page.
       
       logger.debug("Returning page: "+page);
       return page;
    }

}
