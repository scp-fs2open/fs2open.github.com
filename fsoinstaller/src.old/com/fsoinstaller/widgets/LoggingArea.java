
package com.fsoinstaller.widgets;

import javax.swing.JTextArea;
import javax.swing.text.BadLocationException;

import org.apache.log4j.Logger;


public class LoggingArea
{
	private static Logger logger = Logger.getLogger(LoggingArea.class);

	protected Logger textLogger;
	protected JTextArea textArea;

	public LoggingArea(String name, int textRows, int textColumns)
	{
		textLogger = Logger.getLogger(name);
		textArea = new JTextArea(textRows, textColumns);

		textArea.setLineWrap(true);
		textArea.setWrapStyleWord(true);
		textArea.setEditable(false);
	}

	public JTextArea getTextArea()
	{
		return textArea;
	}

	public void append(String line)
	{
		textLogger.info(line);
		textArea.append(line);
	}

	public void replaceLastLine(String line)
	{
		textLogger.info(line);
		try
		{
			textArea.replaceRange(null, textArea.getLineStartOffset(textArea.getLineCount() - 1), textArea.getLineEndOffset(textArea.getLineCount() - 1));
			textArea.replaceRange(line, textArea.getLineStartOffset(textArea.getLineCount() - 1), textArea.getLineEndOffset(textArea.getLineCount() - 1));
		}
		catch (BadLocationException ble)
		{
			logger.error(ble);
		}
	}
}
