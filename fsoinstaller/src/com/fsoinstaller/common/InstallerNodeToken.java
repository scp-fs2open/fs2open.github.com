
package com.fsoinstaller.common;

public enum InstallerNodeToken
{
	NAME("NAME"),
	DESC("DESC"),
	ENDDESC("ENDDESC"),
	FOLDER("FOLDER"),
	DELETE("DELETE"),
	RENAME("RENAME"),
	URL("URL"),
	MULTIURL("MULTIURL"),
	ENDMULTI("ENDMULTI"),
	VERSION("VERSION"),
	NOTE("NOTE"),
	ENDNOTE("ENDNOTE"),
	END("END");

	private final String token;

	private InstallerNodeToken(String token)
	{
		this.token = token;
	}

	public String getToken()
	{
		return token;
	}

	@Override
	public String toString()
	{
		return token;
	}
}
