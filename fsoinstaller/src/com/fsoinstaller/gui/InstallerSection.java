package com.fsoinstaller.gui;

import java.io.*;
import java.net.*;
import java.util.*;
import java.util.zip.*;
import java.awt.event.*;
import javax.swing.*;

import com.fsoinstaller.main.FreeSpaceOpenInstaller;
import com.fsoinstaller.util.InternetUtil;
import com.fsoinstaller.util.THFileIO;

/**
 * @author Thomas
 *
 */
public class InstallerSection implements ActionListener {
	private static final int FILTER = 128;
	private static THFileIO versiontrack;
	private static JProgressBar progressbar;
	private THFileIO modtextfile;
	private ArrayList<UsableURL> url;
	private String mainpath, folderpath, name, desc, filename, currentver, oldver, note;
	private boolean errored = false, hasinstallnote = false, noauto = false;
	private JCheckBox selectbox;
	private JButton help;
	private InstallerSection parent = null;

	public InstallerSection(String path, String file){
		this(path, file, new THFileIO(path + "Installer" + File.separator + file));
	}
	public InstallerSection(String path, String file, THFileIO mod, InstallerSection newparent){
		this(path, file, mod);
		parent = newparent;
	}
	public InstallerSection(String path, String file, THFileIO mod){
		url = new ArrayList<UsableURL>();
		mainpath = path;
		filename = file;
		modtextfile = mod;
		name = modtextfile.readFromFile();
		if (name.equals("NAME"))
				name = modtextfile.readFromFile();
		desc = "";
		String line = modtextfile.readFromFile();
		if (line.equals("NOAUTO"))
		{
			noauto = true;
			line = modtextfile.readFromFile();
		}
		if (line.equals("DESC")){
			line = modtextfile.readFromFile();
				while (line != null && !line.equals("ENDDESC")){
					desc += line + "\n";
					line = modtextfile.readFromFile();
				}
		}
		selectbox = new JCheckBox(name);
		SelectPage selectpage = (SelectPage) FreeSpaceOpenInstaller.GUI().getPage(SelectPage.class);
		selectbox.addActionListener(selectpage);
		selectbox.setActionCommand(name);
		help = new JButton("What's this?");
		help.addActionListener(this);
		help.setActionCommand("GETHELP");
		System.out.println(name);
	}

	public ArrayList<InstallerSection> getSections(boolean auto, boolean installedOnly){
		this.returnToStart();
		ArrayList<InstallerSection> result = new ArrayList<InstallerSection>();
        result.add(this);
//        if (!noauto && auto) {
        if (auto) {
        	selectbox.setSelected(true);
        }
		currentver = "MAYBE";
		oldver = "MAYBE2";
		while (currentver.equals("MAYBE") && modtextfile.hasNext()){
			String line = modtextfile.readFromFile();
			if (line != null && !line.equals("")){
				if (line.equals("NAME"))
					(new InstallerSection(mainpath, filename, modtextfile)).skip();
				else if (line.equals("VERSION"))
					currentver = modtextfile.readFromFile();
			}
		}
		THFileIO oldfile = new THFileIO(mainpath + "Installer" + File.separator + "installedversions.txt");
		while (oldver.equals("MAYBE2") && oldfile.hasNext()){
			String line = oldfile.readFromFile();
			if (line != null && !line.equals("")){
				if (line.equals("NAME") && oldfile.hasNext()){
					line = oldfile.readFromFile();
					if (line.equals(name) && oldfile.hasNext()){
						line = oldfile.readFromFile();
						if (line.equals("VERSION") && oldfile.hasNext())
							oldver = oldfile.readFromFile();
					}
				}
			}
		}
		if (!currentver.equalsIgnoreCase(oldver) && !(installedOnly && oldver.equals("MAYBE2"))){
			this.returnToStart();
		}
		else {
			if (!oldver.equals("MAYBE2")){
				versiontrack.writeToFile("NAME\n" + name + "\nVERSION\n" + oldver);
				this.returnToStart();
				selectbox.setSelected(true);
			}
			else
			{
				selectbox.setSelected(false);				
			}
			selectbox.setEnabled(false);
		}
		boolean done = false, childauto = /*!noauto &&*/ auto;
		InstallerSection newparent = this;
		while (modtextfile.hasNext() && !done){
			String line = modtextfile.readFromFile();
			if (line != null && !line.equals("")){
				if (line.equals("NAME"))
				{
					result.addAll((new InstallerSection(mainpath, filename, modtextfile, newparent)).getSections(childauto, installedOnly));
				} else if (line.equals("END"))
				{
					if (this.parent() == null)
					{
						newparent = null;
						childauto = auto;
					}
					else
						done = true;
				}
			}
		}
		return result;
	}
	
	public JPanel getPanel()
	{
		JPanel panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
		for(int i = 0; i < numParents(); i++)
			panel.add(Box.createHorizontalStrut(15));
		panel.add(selectbox);
		panel.add(Box.createHorizontalGlue());
		panel.add(help);
		return panel;
	}
	public int numParents()
	{
		int i = 0;
		for (InstallerSection s = this.parent; s != null; s = s.parent())
			i++;
		return i;
	}
	public void actionPerformed(ActionEvent e)
	{
		if ("GETHELP".equals(e.getActionCommand()))
		{
			JOptionPane.showMessageDialog(null,
				    "Description of " + name + ":\n" + desc,
				    FreeSpaceOpenInstaller.INSTALLER_NAME,
				    JOptionPane.INFORMATION_MESSAGE);
		}
	}
	
	private void returnToStart()
	{
		modtextfile.returnToStart();
		boolean done = false;
		while (!done && modtextfile.hasNext()){
			String line = modtextfile.readFromFile();
			if (line != null && !line.equals("")){
				if (line.equals("NAME"))
					if (modtextfile.readFromFile().equals(name))
						done = true;
			}
		}
		if (noauto)
			modtextfile.readFromFile();

	}
	public void update(){
		boolean quitting = false;
		if (!isTreeSelected() || currentver.equalsIgnoreCase(oldver))
		{
			if (!isTreeSelected() && !oldver.equals("MAYBE2")){
				versiontrack.writeToFile("NAME\n" + name + "\nVERSION\n" + oldver);
			}
			quitting = true;
		}
		this.returnToStart();
		while (!quitting && modtextfile.hasNext()){
			FreeSpaceOpenInstaller.GUI().refresh();
			String line = modtextfile.readFromFile();
			if (line != null && !line.equals("")){
//				System.out.println(line);
				if (line.equals("FOLDER")){
					folderpath = modtextfile.readFromFile() + File.separator;
					if (folderpath.equals("\\" + File.separator))
						folderpath = "";
					(new File(mainpath + folderpath)).mkdir();
				}
				else if (line.equals("SUBFOLDER")){
					folderpath += modtextfile.readFromFile() + File.separator;
					(new File(mainpath + folderpath)).mkdir();
				}
				else if (line.equals("URL")) {
					url.clear();
					String line2 = modtextfile.readFromFile();
					if (line2.equals("LOCAL")) {
						try {
						url.add(new UsableURL((new File(modtextfile.readFromFile()).toURI().toURL().toString()), false));
						} catch (MalformedURLException E) {
							E.printStackTrace();
						}
					}
					else {
						url.add(new UsableURL(line2, false));
					}
				}
				else if (line.equals("MULTIURL")){
					url.clear();
					String line2 = modtextfile.readFromFile();
						while (line2 != null && !line2.equals("ENDMULTI")){
							if (line2.equals("LOCAL")) {
								try {
								url.add(new UsableURL(new File(modtextfile.readFromFile()).toURI().toURL().toString(), false));
								} catch (MalformedURLException E) {
									E.printStackTrace();
								}
							}
							else {
								url.add(new UsableURL(line2, false));
							}
							System.out.println(url.get(url.size() - 1));
							line2 = modtextfile.readFromFile();
						}
				}
				else if (line.equals("NAME"))
					(new InstallerSection(mainpath, filename, modtextfile)).skip();
				else if (line.equals("END"))
					quitting = true;
				else if (line.equals("DESC")){
					desc = "";
					String line2 = modtextfile.readFromFile();
						while (line != null && !line2.equals("ENDDESC")){
							desc += line2 + "\n";
							line2 = modtextfile.readFromFile();
						}
				}
				else if (line.equals("NOTE")){
					if (!hasinstallnote)
					{
						hasinstallnote = true;
						note = "Post-Installation Notes for " + name + "\n";
					}
					String line2 = modtextfile.readFromFile();
						while (line != null && !line2.equals("ENDNOTE")){
							note += line2 + "\n";
							line2 = modtextfile.readFromFile();
						}
				}
				else if (line.equals("VERSION")){
					if (!errored)
					{
						versiontrack.writeToFile("NAME\n" + name + "\nVERSION\n" + currentver);
					}
					else
					{
						if (!hasinstallnote)
						{
							hasinstallnote = true;
							note = "Post-Installation Notes for " + name + "\n";
						}
						note += "IMPORTANT NOTE: There was an error while installing this section." +
								" You may need to run the Installer again in order to ensure that this" +
								" section works correctly.\n";
					}
					modtextfile.readFromFile();
				}
				else if (line.equals("DELETE"))
					(new File(mainpath + folderpath + modtextfile.readFromFile())).delete();
				else if (line.equals("RENAME")){
					File start = new File(mainpath + folderpath + modtextfile.readFromFile());
					File dest = new File(mainpath + folderpath + modtextfile.readFromFile());
					start.renameTo(dest);
				}
				else if (line.equals("UNZIP")) {
					ArrayList<String> files = null;
					String source = modtextfile.readFromFile();
					String line2 = modtextfile.readFromFile();
					while (!line2.equals("ENDUNZIP")) {
						if (files == null) {
							files = new ArrayList<String>();
						}
						files.add(line2);
						line2 = modtextfile.readFromFile();
					}
					downloadZip(source, mainpath + folderpath, files);
				}
				else {
					if (line.substring(line.length() - 3, line.length()).equalsIgnoreCase("ZIP")){
						FreeSpaceOpenInstaller.INSTALL_STATUS.append("Checking: " + line + "\n");
						downloadZip(line, mainpath + folderpath);
						FreeSpaceOpenInstaller.INSTALL_STATUS.append("Finished with Zip File: " + line + "\n");
					}
					else{
						FreeSpaceOpenInstaller.INSTALL_STATUS.append("Checking: " + line + "\n");
						download(line, mainpath + folderpath + line);
						if (line.substring(line.length() - 6, line.length()).equalsIgnoreCase("TAR.GZ")){
							try {
      				        	String cmd = "tar -xvzf " + mainpath + folderpath + line + " -C " + mainpath + folderpath;
								FreeSpaceOpenInstaller.INSTALL_STATUS.append("Untarring using command: " + cmd + "\n");
								Runtime.getRuntime().exec(cmd);
							} catch (Exception E) {}
						}
						if (line.substring(line.length() - 7, line.length()).equalsIgnoreCase("TAR.BZ2")){
							try {
								String cmd = "tar -xvjf " + mainpath + folderpath + line + " -C " + mainpath + folderpath;
								FreeSpaceOpenInstaller.INSTALL_STATUS.append("Untarring using command: " + cmd + "\n");
								Runtime.getRuntime().exec(cmd);
							} catch (Exception E) {}
						}
					}
				}
			}
			THFileIO statusfile = null;
			try {
				statusfile = new THFileIO(mainpath + "installer" + File.separator + "installer_status.log.txt");
			}
			catch (Exception E){
				try {
					(new File(mainpath + "installer" + File.separator + "installer_status.log")).createNewFile();
					System.setErr(new PrintStream(new FileOutputStream(mainpath + "installer" + File.separator + "installer_status.log", false)));
				}
				catch (Exception F){E.printStackTrace(); F.printStackTrace();}
			}
			try {
				statusfile.writeToFile(FreeSpaceOpenInstaller.INSTALL_STATUS.getTextArea().getDocument().getText(0, FreeSpaceOpenInstaller.INSTALL_STATUS.getTextArea().getDocument().getLength()));
			}
			catch (Exception E){E.printStackTrace();}
		}
		FreeSpaceOpenInstaller.INSTALL_STATUS.append("Done with section: " + name + "\n");
		if (hasinstallnote)
		{
			note += "\n\n";
			FreeSpaceOpenInstaller.INSTALL_NOTES.append(note);
		}
	}
	public void skip(){
		boolean quitting = false;
		while (!quitting && modtextfile.hasNext()){
			String line = modtextfile.readFromFile();
			if (line != null && !line.equals("")){
				if (line.equals("NAME"))
					(new InstallerSection(mainpath, filename, modtextfile)).skip();
				else if (line.equals("END"))
					quitting = true;
			}
		}
	}
	public String name(){
		return name;
	}
	public void selectClick()
	{
		selectbox.doClick();
	}
	public void setSelected(boolean bool)
	{
		selectbox.setSelected(bool);
	}
	public boolean isTreeSelected()
	{
		if (parent == null)
			return isSelected();
		return isSelected() && parent().isTreeSelected();
	}
	public boolean isSelected()
	{
		return selectbox.isSelected();		
	}
	public InstallerSection parent() {
		return parent;
	}
	public boolean equals(Object obj) {
		if (obj instanceof InstallerSection)
		{
			InstallerSection other = (InstallerSection) obj;
			if (this.name().equals(other.name()))
				return true;
		}
		return false;
	}
	public static void versiontrack(THFileIO textfile){
		versiontrack = textfile;
	}
	public static THFileIO versiontrack(){
		return versiontrack;
	}
	public static void setProgressBar(JProgressBar newbar){
		progressbar = newbar;
	}
	public static JProgressBar getProgressBar() {
		return progressbar;
	}
	
	private int findFile(List<String> files, String query)
	{
		if (files == null) {
			return -1;
		}
		query = query.replace('/', '\\');
		for (int i = 0; i < files.size(); i++) {
			files.set(i, files.get(i).replace('/', '\\'));
			if (files.get(i).lastIndexOf(query) != -1 || query.lastIndexOf(files.get(i)) != -1) {
				return i;
			}
		}
		return -2;
	}
	private class UsableURL {
		private String url;
		private boolean used;
		
		UsableURL(String newurl, boolean newused)
		{
			url = newurl;
			used = newused;
		}
		public boolean isUsed()
		{
			return used;
		}
		public void setUsed(boolean newused)
		{
			used = newused;
		}
		public String getURL()
		{
			return url;
		}
		public String toString()
		{
			return getURL();
		}
	}
	
	
	
	
	private void download(String remoteFileName, String localFileName) {
		for (int i = 0; i < url.size(); i++) {
			url.get(i).setUsed(false);
		}
		download(remoteFileName, localFileName, new Random().nextInt(url.size()));
	}
	private void download(String remoteFileName, String localFileName, int numOfMirror) {
		String address = url.get(numOfMirror) + remoteFileName;
		url.get(numOfMirror).setUsed(true);
		OutputStream out = null;
		URLConnection conn = null;
		InputStream  in = null;
		try {
			URL url2 = new URL(address);
			conn = InternetUtil.getConnection(url2);
			FreeSpaceOpenInstaller.INSTALL_STATUS.append("Current File Size: " + (new File(localFileName)).length() + "\n");
			FreeSpaceOpenInstaller.INSTALL_STATUS.append("New File Size: " + conn.getContentLength() + "\n");
			if ( (new File(localFileName)).exists() && conn.getContentLength() > 0 && (new File(localFileName)).length() == conn.getContentLength() ){
				FreeSpaceOpenInstaller.INSTALL_STATUS.append("The File " + remoteFileName + " is up to date.\n");
				return;
			}
			//Only print this once.
			if (numOfMirror == 1) {
				FreeSpaceOpenInstaller.INSTALL_STATUS.append("The File " + remoteFileName + " is out of date.\n");
			}
			in = conn.getInputStream();
			try{
			out = new BufferedOutputStream(
				new FileOutputStream(localFileName));
			}catch(FileNotFoundException e){
				(new File(localFileName)).createNewFile();
				out = new BufferedOutputStream(
						new FileOutputStream(localFileName));
			}
			byte[] buffer = new byte[FILTER];
			int numRead, count = 0;
			InstallerSection.progressbar.setIndeterminate(false);
			InstallerSection.progressbar.setMinimum(0);
			InstallerSection.progressbar.setMaximum(100);
			InstallerSection.progressbar.setValue(0);
			long numWritten = 0;
			while ((numRead = in.read(buffer)) != -1) {
				out.write(buffer, 0, numRead);
				numWritten += numRead;
				if (count == FILTER){
					count = 0;
					InstallerSection.progressbar.setValue((int)(((double)numWritten/conn.getContentLength())*100));
					InstallerSection.progressbar.setString("Downloading: " + localFileName + " " + (int)(((double)numWritten/conn.getContentLength())*100) + "% ( " + numWritten/FILTER + " KB) Done.");
					FreeSpaceOpenInstaller.GUI().refresh();
				} else count++;
			}
			FreeSpaceOpenInstaller.INSTALL_STATUS.append("Downloaded: " + localFileName + "\t\t" + ((double)numWritten)/FILTER + " KB\n");
			InstallerSection.progressbar.setValue(0);
			InstallerSection.progressbar.setIndeterminate(true);
			InstallerSection.progressbar.setString("Working...");
			FreeSpaceOpenInstaller.GUI().refresh();
		} catch (Exception exception) {
			FreeSpaceOpenInstaller.INSTALL_STATUS.append("Download Mirror " + (numOfMirror + 1) + " (" + url.get(numOfMirror) + ") has failed.\n");
			numOfMirror = -1;
			for (int i = 0; i < url.size(); i++) {
				if (!url.get(i).isUsed()) {
					do {
						numOfMirror = new Random().nextInt(url.size());
						numOfMirror = (url.get(numOfMirror).isUsed()) ? -1 : numOfMirror;
					} while (numOfMirror == -1);
					break;
				}
			}
			if (numOfMirror != -1) {
				FreeSpaceOpenInstaller.INSTALL_STATUS.append("Trying Download Mirror " + (numOfMirror + 1) + "...\n");
				download(remoteFileName, localFileName, numOfMirror);
			}
			else {
				FreeSpaceOpenInstaller.INSTALL_STATUS.append("All Download Mirrors have failed.\n");
				errored = true;
				exception.printStackTrace();
			}
		} finally {
			try {
				if (in != null) {
					in.close();
				}
				if (out != null) {
					out.close();
				}
			} catch (IOException ioe) {
			}
		}
	}
	private void downloadZip(String remoteZipName, String localDestination) {
		for (int i = 0; i < url.size(); i++) {
			url.get(i).setUsed(false);
		}
		downloadZip(remoteZipName, localDestination, new Random().nextInt(url.size()), null);
	}
	private void downloadZip(String remoteZipName, String localDestination, ArrayList<String> files) {
		for (int i = 0; i < url.size(); i++) {
			url.get(i).setUsed(false);
		}
		downloadZip(remoteZipName, localDestination, new Random().nextInt(url.size()), files);
	}

}