package com.fsoinstaller.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Scanner;

import org.apache.log4j.Logger;

/**
 * THFileIO.java - Handles reading from and writing to text files.
 * @author Thomas
 */
public class THFileIO
{
	private static final Logger logger = Logger.getLogger(THFileIO.class);
	
	private PrintWriter output;
	private FileInputStream file;
	//Scanner is also a buffered stream reader, so HA!
	private Scanner input;
	private String filename;
	private boolean append = false;
	/**
	 * Constructor for THFileIO.
	 * @param the file name to interact with.
	 */
	public THFileIO(String name){
		filename = name;
		try {
			logger.debug("Opening file: " + filename);
			file = new FileInputStream(filename);
			input = new Scanner(file);
			returnToStart();
		}
		catch (FileNotFoundException fnfe){
			try{
				logger.debug("File not found; attempting to create it");
				(new File(filename)).createNewFile();
				file = new FileInputStream(filename);
				input = new Scanner(file);
				returnToStart();
			} catch(IOException ioe) {
				logger.error("Could not open " + filename + " for writing!", ioe);
			}
		}
	}
	public THFileIO(String name, boolean app){
		this(name);
		append = app;
	}
	/**
	 * Returns the reader to the beginning of the file.
	 */
	public void returnToStart(){
		logger.trace("Returning to beginning of file");
		
		try {file = new FileInputStream(filename);}
		catch (FileNotFoundException fnfe){
			logger.warn(fnfe);
		}
		
		try{input = new Scanner(file);}
		catch (NullPointerException npe){
			logger.warn(npe);
		}
	}
	/**
	 * Reads the next line from the file.
	 * @return The line read. Returns null if no more lines.
	 */
	public String readFromFile(){
		logger.trace("Reading the next line from file");
		
		String result = null;
		if (input.hasNext()){
			result = input.nextLine().trim();
			while(result.length() > 0 && result.charAt(0) == '\t'){
				if (result.equals("\t"))
					result = "";
				else
					result = result.substring(1, result.length() - 1);
			}
		}
		return result;
	}
	/**
	 * Gets a line off of the file and returns it.
	 * @param line number of the list to read.
	 * @return String that contains that line of the file.
	 */
	public String readLine(int line){
		logger.trace("Reading line number " + line + " from file");
		
		String result;
		returnToStart();
		
		//Get to the line we want to read.
		for (int count = 0; count < line - 1; count++)
			if (input.hasNext())
				result = input.nextLine();
		// Put the default in result.
		result = null;
		//If there is something at the line given, read it.
		if (input.hasNext())
			result = input.nextLine();		
		return result;
	}
	/**
	 * Writes a line to the end of the file. When you are completely
	 * done writing to the file, make sure to call finishWriting()
	 * @param String that contains the text to write.
	 * @return void
	 */
	public void writeToFile(String outputstring){
		logger.trace("Writing line to file");
		
		if (output == null){
			logger.debug("Output writer is null; creating it");
			try {output = new PrintWriter(new FileWriter(filename, append), true);}
			catch (IOException ioe){
				logger.warn(ioe);
			}
		}
		output.println(outputstring);
	}
	/**
	 * Writes a line to the end of the file. When you are completely
	 * done writing to the file, make sure to call finishWriting()
	 * @param String that contains the text to write.
	 * @return void
	 */
	public void writeToFile(){
		writeToFile("");
	}	
	/**
	 * Checks if there are more lines to read in the file.
	 * @return true if there are more lines to read.
	 */
	public boolean hasNext(){
		return input.hasNext();
	}
	/**
	 * "Saves" the written file. If this is not called, the
	 * data written to the file will not be saved after the
	 * program exits.
	 */
	public void finishWriting(){
		logger.debug("Finished writing to this file");
		if (output != null)
			output.close();
		System.gc();
	}
	public void restartWriting(){
		logger.debug("Restarting file writing");
		try {output = new PrintWriter(new FileWriter(filename, append), true);}
		catch (IOException ioe){
			logger.warn(ioe);
		}
	}
}
