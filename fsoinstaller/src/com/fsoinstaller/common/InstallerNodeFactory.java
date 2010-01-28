
package com.fsoinstaller.common;

import java.io.IOException;
import java.io.Reader;
import java.io.Writer;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;


/**
 * Mostly for parsing and printing.
 */
public class InstallerNodeFactory
{
	private static final String EOL = System.getProperty("line.separator");

	public static InstallerNode readNode(Reader reader) throws InstallerNodeParseException, IOException
	{
		// skip strings until NAME is reached
		Object object;
		while (true)
		{
			object = readStringOrToken(reader);
			if (object == null)
				throw new InstallerNodeParseException("End of reader reached before '" + InstallerNodeToken.NAME + "' found!");
			else if (object == InstallerNodeToken.NAME)
				break;
			else if (object instanceof InstallerNodeToken)
				throw new InstallerNodeParseException("Unexpected token '" + object + "' found while looking for '" + InstallerNodeToken.NAME + "'!");
		}

		return readNodeSub(reader);
	}

	private static InstallerNode readNodeSub(Reader reader) throws InstallerNodeParseException, IOException
	{
		// create the node with its name
		InstallerNode node = new InstallerNode(readString(reader));

		// populate this node
		Object object;
		while (true)
		{
			object = readStringOrToken(reader);

			if (object == null)
			{
				throw new InstallerNodeParseException("End of reader reached before parsing completed!");
			}
			// child node
			else if (object == InstallerNodeToken.NAME)
			{
				node.addChild(readNodeSub(reader));
			}
			// done with the node
			else if (object == InstallerNodeToken.END)
			{
				break;
			}
			// handle a token
			else if (object instanceof InstallerNodeToken)
			{
				handleToken(reader, (InstallerNodeToken) object, node);
			}
			// an unannotated string means something we install
			else
			{
				node.addInstall((String) object);
			}
		}

		return node;
	}

	private static void handleToken(Reader reader, InstallerNodeToken token, InstallerNode node) throws InstallerNodeParseException, IOException
	{
		switch (token)
		{
			case DELETE:
				String delete = readString(reader);
				node.addDelete(delete);
				break;

			case DESC:
				String desc = readStringUntilEndToken(reader, InstallerNodeToken.ENDDESC);
				node.setDescription(desc);
				break;

			case FOLDER:
				String folder = readString(reader);
				node.setFolder(folder);
				break;

			case MULTIURL:
				List<String> strings = readStringsUntilEndToken(reader, InstallerNodeToken.ENDMULTI);
				String curURL = null;
				try
				{
					Iterator<String> ii = strings.iterator();
					while (ii.hasNext())
					{
						curURL = ii.next();
						URI urlToAdd = new URI(curURL);
						if (!validateURL(urlToAdd))
							throw new InstallerNodeParseException("The URL '" + curURL + "' must use the HTTP protocol and must point to a folder, not a file!");
						node.addRootURL(urlToAdd);
					}
				}
				catch (URISyntaxException urise)
				{
					throw new InstallerNodeParseException("Syntax error in URL '" + curURL + "'!", urise);
				}
				break;

			case NOTE:
				String note = readStringUntilEndToken(reader, InstallerNodeToken.ENDNOTE);
				node.setNote(note);
				break;

			case RENAME:
				String renameFrom = readString(reader);
				String renameTo = readString(reader);
				node.addRenamePair(new InstallerNode.RenamePair(renameFrom, renameTo));
				break;

			case URL:
				String singleURL = readString(reader);
				try
				{
					URI urlToAdd = new URI(singleURL);
					if (!validateURL(urlToAdd))
						throw new InstallerNodeParseException("The URL '" + singleURL + "' must use the HTTP protocol and must point to a folder, not a file!");
					node.addRootURL(urlToAdd);
				}
				catch (URISyntaxException urise)
				{
					throw new InstallerNodeParseException("Syntax error in URL '" + singleURL + "'!", urise);
				}
				break;

			case VERSION:
				String version = readString(reader);
				node.setVersion(version);
				break;

			case ENDDESC:
			case ENDMULTI:
			case ENDNOTE:
				throw new InstallerNodeParseException("Unexpected token '" + token + "' found!");

			case NAME:
			case END:
				throw new Error("The token '" + token + "' should have been handled already!");

			default:
				throw new Error("Unhandled token '" + token + "'!");
		}
	}

	/**
	 * Ensure that we use HTTP and that we are pointing to a folder, not a file.
	 */
	public static boolean validateURL(URI theURL)
	{
		// TODO
	}

	private static String readString(Reader reader) throws InstallerNodeParseException, IOException
	{
		Object object = readStringOrToken(reader);
		if (object == null)
			throw new InstallerNodeParseException("End of reader reached before parsing completed!");
		else if (object instanceof InstallerNodeToken)
			throw new InstallerNodeParseException("Expected a plain string; found the token '" + object + "'!");

		return (String) object;
	}

	private static List<String> readStringsUntilEndToken(Reader reader, InstallerNodeToken endToken) throws InstallerNodeParseException, IOException
	{
		List<String> strings = new ArrayList<String>();

		while (true)
		{
			Object object = readStringOrToken(reader);
			if (object == null)
				throw new InstallerNodeParseException("End of reader reached before parsing completed!");
			else if (object == endToken)
				break;
			else if (object instanceof InstallerNodeToken)
				throw new InstallerNodeParseException("Unexpected token '" + object + "' found while looking for '" + endToken + "'!");

			strings.add((String) object);
		}

		return strings;
	}

	private static String readStringUntilEndToken(Reader reader, InstallerNodeToken endToken) throws InstallerNodeParseException, IOException
	{
		List<String> strings = readStringsUntilEndToken(reader, endToken);
		Iterator<String> ii = strings.iterator();
		if (!ii.hasNext())
			return "";

		StringBuilder builder = new StringBuilder(ii.next());
		while (ii.hasNext())
		{
			builder.append('\n');
			builder.append(ii.next());
		}
		return builder.toString();
	}

	private static Object readStringOrToken(Reader reader) throws IOException
	{
		String line = readLine(reader);
		if (line == null)
			return null;

		for (InstallerNodeToken token: InstallerNodeToken.values())
		{
			if (token.getToken().equals(line))
				return token;
		}

		return line;
	}

	/**
	 * Returns the next line of input, trimmed of whitespace on either side. Returns
	 * null if the end of the reader has been reached. (Note that this means the last
	 * line of characters will be lost, if there is no newline between it and the end of
	 * the reader.)
	 */
	private static String readLine(Reader reader) throws IOException
	{
		StringBuilder builder = new StringBuilder();
		while (true)
		{
			int ch = reader.read();
			if (ch < 0)
				return null;
			else if (ch == '\n')
				break;
			builder.append((char) ch);
		}
		return builder.toString().trim();
	}

	public static void writeNode(Writer writer, InstallerNode node) throws IOException
	{
		writeNode(0, writer, node);
	}

	public static void writeNode(int indent, Writer writer, InstallerNode node) throws IOException
	{
		writeLine(indent, writer, InstallerNodeToken.NAME, node.getName());

		if (node.getDescription() != null)
			writeLine(indent, writer, InstallerNodeToken.DESC, node.getDescription(), InstallerNodeToken.ENDDESC);

		if (node.getFolder() != null)
			writeLine(indent, writer, InstallerNodeToken.FOLDER, node.getFolder());

		for (String delete: node.getDeleteList())
			writeLine(indent, writer, InstallerNodeToken.DELETE, delete);

		for (InstallerNode.RenamePair pair: node.getRenameList())
			writeLine(indent, writer, InstallerNodeToken.RENAME, pair.getFrom(), pair.getTo());

		if (node.getRootURLList().size() == 1)
			writeLine(indent, writer, InstallerNodeToken.URL, node.getRootURLList().get(0).toString());
		else if (node.getRootURLList().size() > 1)
		{
			writeLine(indent, writer, InstallerNodeToken.MULTIURL);
			for (URI rootURL: node.getRootURLList())
				writeLine(indent, writer, rootURL.toString());
			writeLine(indent, writer, InstallerNodeToken.ENDMULTI);
		}

		for (String install: node.getInstallList())
			writeLine(indent, writer, install);

		if (!node.getChildren().isEmpty())
		{
			writeLine(indent, writer, "");
			for (InstallerNode child: node.getChildren())
				writeNode(indent + 1, writer, child);
		}

		if (node.getVersion() != null)
			writeLine(indent, writer, InstallerNodeToken.VERSION, node.getVersion());

		if (node.getNote() != null)
			writeLine(indent, writer, InstallerNodeToken.NOTE, node.getNote(), InstallerNodeToken.ENDNOTE);

		writeLine(indent, writer, InstallerNodeToken.END);
		writeLine(indent, writer, "");
	}

	private static void writeLine(int indent, Writer writer, Object ... objects) throws IOException
	{
		for (Object object: objects)
		{
			if (object instanceof String)
				writeLine(indent, writer, (String) object);
			else if (object instanceof InstallerNodeToken)
				writeLine(indent, writer, ((InstallerNodeToken) object).getToken());
			else
				writeLine(indent, writer, object.toString());
		}
	}

	private static void writeLine(int indent, Writer writer, String string) throws IOException
	{
		String trimmed = string.trim();
		if (!trimmed.isEmpty())
		{
			for (int i = 0; i < indent; i++)
				writer.write('\t');
			writer.write(trimmed);
		}
		writer.write(EOL);
	}
}
