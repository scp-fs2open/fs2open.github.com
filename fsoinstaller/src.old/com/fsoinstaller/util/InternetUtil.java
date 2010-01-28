package com.fsoinstaller.util;

import java.io.IOException;
import java.net.URL;
import java.net.URLConnection;

import org.apache.log4j.Logger;

public class InternetUtil
{
    private static final Logger logger = Logger.getLogger(InternetUtil.class);
    
    // Used to identify the windows platform.
    private static final String WIN_ID = "Windows";
    // The default system browser under windows.
    private static final String WIN_PATH = "rundll32";
    // The flag to display a url.
    private static final String WIN_FLAG = "url.dll,FileProtocolHandler";
    // The default browser under unix.
    private static final String UNIX_PATH = "firefox";
    // The flag to display a url.
    private static final String UNIX_FLAG = "";
	//Password if we're using an HTTP proxy
	private static String proxypass = null;
	
    /**
     * Display a file in the system browser.  If you want to display a
     * file, you must include the absolute path name.
     *
     * @param url the file's url (the url must start with either "http://"
     * or
     * "file://").
     */
    public static void browseToURL(String url) {
    	logger.info("Browsing to URL: " + url);
    	
        boolean windows = isWindowsPlatform();
        String cmd = null;
        try {
            if (windows) {
            	logger.debug("Launching browser in Windows OS");
            	
                // cmd = 'rundll32 url.dll,FileProtocolHandler http://...'
                cmd = WIN_PATH + " " + WIN_FLAG + " " + url;
                Runtime.getRuntime().exec(cmd);
            } else {
            	logger.debug("Launching browser in non-Windows OS");
            	
                // Under Unix, Netscape has to be running for the "-remote"
                // command to work.  So, we try sending the command and
                // check for an exit value.  If the exit command is 0,
                // it worked, otherwise we need to start the browser.
                // cmd = 'netscape -remote openURL(http://www.java-tips.org)'
                cmd = UNIX_PATH + " " + UNIX_FLAG + url;
                Process p = Runtime.getRuntime().exec(cmd);
                try {
                	logger.debug("Waiting for exit code");
                	
                    // wait for exit code -- if it's 0, command worked,
                    // otherwise we need to start the browser up.
                    int exitCode = p.waitFor();
                    if (exitCode != 0) {
                    	logger.debug("Command failed; starting the browser explicitly");
                    	
                        // Command failed, start up the browser
                        // cmd = 'netscape http://www.java-tips.org'
                        cmd = UNIX_PATH + " "  + url;
                        p = Runtime.getRuntime().exec(cmd);
                    }
                } catch(InterruptedException ie) {
                	logger.error("Error bringing up browser; cmd='" + cmd + "'", ie);
                }
            }
        } catch(IOException ioe) {
            // couldn't exec browser
        	logger.error("Could not invoke browser; cmd='" + cmd + "'", ioe);
        }
    }
    /**
     * Try to determine whether this application is running under Windows
     * or some other platform by examing the "os.name" property.
     *
     * @return true if this application is running under a Windows OS
     */
    private static boolean isWindowsPlatform() {
        String os = System.getProperty("os.name");
        if ( os != null && os.startsWith(WIN_ID))
            return true;
        return false;   
    }
    
    public static void setProxyPassword(String plainText)
    {
    	proxypass = Codecs.base64Encode(plainText);
    }
    
	public static URLConnection getConnection(URL url)
	{
		logger.debug("Opening connection to URL: " + url);
		
		URLConnection con = null;
		try {
			con = url.openConnection();
			if (proxypass != null)
			{
				con.setRequestProperty("Proxy-Authorization", proxypass);
			}
		}
		catch( IOException ioe) {
			logger.error("Failed to open connection!", ioe);
		}
		return con;
	}
}