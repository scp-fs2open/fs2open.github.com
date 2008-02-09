#ifdef _WIN32
#include <windows.h>
#endif

#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>

#include "dbugfile.h"

bool dbugfile_init_var = false;

char dbugfile_filename[512];

void dbugfile_output(char *buffer)
{
	if(dbugfile_init_var == false)
	{
		return;
	}


	FILE *fp = fopen(dbugfile_filename, "a");
	
	if(fp == NULL)
	{
		return;
	}

	fwrite(buffer, sizeof(char) * strlen(buffer), 1, fp);
	fclose(fp);
}

void dbugfile_init()
{
	char big_buffer[1000];
	char temp_buff[512];

	strcpy(dbugfile_filename, "DBUG-");
	strcpy(big_buffer, "DBUGFILE Active: ");

#ifdef _WIN32
	{
		unsigned long len = 512;
		GetUserName(temp_buff, &len);
		
		strcat(dbugfile_filename, temp_buff);
		strcat(big_buffer, temp_buff);
		
		strcat(dbugfile_filename, "-");
		strcat(big_buffer, " ");
	}
#endif

    _tzset();

    // Display operating system-style date and time.
    _strtime( temp_buff);
	strcat(dbugfile_filename, "(");
	strcat(dbugfile_filename,temp_buff);
	strcat(big_buffer,temp_buff);

	strcat(dbugfile_filename, ")-(");
	strcat(big_buffer, " ");
    _strdate( temp_buff);
	strcat(dbugfile_filename,temp_buff);
	strcat(big_buffer,temp_buff);

	strcat(dbugfile_filename, ").txt");
	strcat(big_buffer, "\n");

	// Remove invalid slash chars
	int len = strlen(dbugfile_filename);
	while(len >= 0)
	{
		if(dbugfile_filename[len] == '/' || dbugfile_filename[len] == ':')
		{
			dbugfile_filename[len] = '-';
		}
		len--;
	}

	// open file
	FILE *fp = fopen(dbugfile_filename, "w");

	if(fp == NULL)
	{
		return;
	}

	fwrite(big_buffer, sizeof(char) * strlen(big_buffer), 1, fp);

#ifdef _WIN32

	// detect OS type

	OSVERSIONINFO version_info;
	version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if(GetVersionEx(&version_info) != false)
	{
		char *error = "Error in detecting OS version";
		char *version = error;

		switch(version_info.dwMajorVersion)
		{
			case 4:
			{
				if(version_info.dwMinorVersion == 0)
				{
					version = "Windows 95"; 
				}
				else if(version_info.dwMinorVersion == 10)
				{
					version = "Windows 98";  
				}
				else if(version_info.dwMinorVersion == 90)
				{
					version = "Windows Me";
				}
				break;
			}
			case 5:
			{
				if(version_info.dwMinorVersion == 0)
				{
					version = "Windows 2000"; 
				}
				else if(version_info.dwMinorVersion == 1)
				{
					version = "Windows XP";
				}
				break;
			}
		}

		char *release_type = NULL;
#ifdef _DEBUG 
		release_type = "Debug";
#else
		release_type = "Release";
#endif
		sprintf(big_buffer, "OS: %s %s\n", version, release_type);
	}				  

	fwrite(big_buffer, sizeof(char) * strlen(big_buffer), 1, fp);

#endif

	fclose(fp);

	dbugfile_init_var = true;
}

void dbugfile_deinit()
{
	dbugfile_output("Normal end");
	dbugfile_init_var = false;
}

void dbugfile_sprintf(int line, char *file, const char *format, ...)
{
	if(dbugfile_init_var == false)
	{
		return;
	}

	// find last slash (for PC) to crop file path
	char *ptr = strrchr(file, '\\');

	if(ptr == NULL)
	{
		ptr = file;
	}
	else
	{
		ptr++;
	}

	char buffer[1000];
	int i =  sprintf(buffer, "[%s,%4d] ", ptr, line);

	va_list ap;
	char *p, *sval;
	long ival;
	double dval;

	// Add each extra parameter to string
	va_start(ap, format);

	for(p = (char *) format; *p; p++)
	{
		if(*p != '%')
		{
			*(buffer + i) = *p;
			i++;
			continue;
		}

		p++;

		switch(*p)
		{
			case 'd':
			{
				ival = va_arg(ap, int);
				i += sprintf(buffer+i,"%d", ival);
				break;
			}
			case 'c':
			{
				ival = va_arg(ap, char);
				buffer[i] = (char) ival;
				i++;
				break;
			}
			case 'x':
			{
				ival = va_arg(ap, int);
				i += sprintf(buffer+i,"%x", ival);
				break;
			}

			case 'f':
			{
				dval = va_arg(ap, double);
				i += sprintf(buffer+i,"%f", dval);
				break;
			}
			case 's':
			{
				for(sval = va_arg(ap, char *); *sval; sval++)
				{
					*(buffer + i++) = *sval;
				}
				break;
			}
			default:
			{
				i += sprintf(buffer+i,"N/A: %%%c", *p);
				break;
			}
		}
	}

	va_end(ap);

	i += sprintf(buffer + i, "\n");

	dbugfile_output(buffer);
}


