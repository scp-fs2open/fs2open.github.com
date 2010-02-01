
package com.fsoinstaller.common;

public class InvalidBaseURLException extends Exception
{
	public InvalidBaseURLException()
	{
		super();
	}

	public InvalidBaseURLException(String message, Throwable cause)
	{
		super(message, cause);
	}

	public InvalidBaseURLException(String message)
	{
		super(message);
	}

	public InvalidBaseURLException(Throwable cause)
	{
		super(cause);
	}
}
