#include "dbugfile.h"
#include "globalincs/pstypes.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>


int  dbugfile_counters[MAX_COUNTERS];
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
	// Clear the counters
	memset(dbugfile_counters, 0, sizeof(int) * MAX_COUNTERS);

	char big_buffer[1000];
	char temp_buff[512];

	dbugfile_filename[0] = '\0';

#ifdef _WIN32

	char path[MAX_PATH];

	GetCurrentDirectory(MAX_PATH, path);

	int path_len = strlen(path);
	if(path[path_len - 1] != '\\')
	{
	 	path[path_len] = '\\';
		path[path_len + 1] = '\0';
	}

 	strcpy_s(dbugfile_filename, path);

#endif

	strcat_s(dbugfile_filename, "DBUG-");
	strcpy_s(big_buffer, "DBUGFILE Active: ");

#ifdef _WIN32
	{
		unsigned long len = 512;
		GetUserName(temp_buff, &len);
		
		strcat_s(dbugfile_filename, temp_buff);
		strcat_s(big_buffer, temp_buff);
		
		strcat_s(dbugfile_filename, "-");
		strcat_s(big_buffer, " ");
	}
#endif

    _tzset();

    // Display operating system-style date and time.
	_strdate( temp_buff);
	strcat_s(dbugfile_filename, "D(");
	strcat_s(dbugfile_filename,temp_buff);
	strcat_s(big_buffer,temp_buff);

	strcat_s(dbugfile_filename, ") T(");
	strcat_s(big_buffer, " ");

    _strtime( temp_buff);
	strcat_s(dbugfile_filename,temp_buff);
	strcat_s(big_buffer,temp_buff);

	strcat_s(dbugfile_filename, ").txt");
	strcat_s(big_buffer, "\n");

	// Remove invalid slash chars
	int len = strlen(dbugfile_filename);
	while(len >= 2)
	{
		if(dbugfile_filename[len] == '/' || dbugfile_filename[len] == ':')
		{
			dbugfile_filename[len] = '-';
		}
		len--;
	}

	// open file
	// Uncomment this to see where the file is going
  	//MessageBox(NULL, dbugfile_filename, "das", MB_OK); 
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
		sprintf(big_buffer, "OS: %s %s, ", version, release_type);
	}				  

	fwrite(big_buffer, sizeof(char) * strlen(big_buffer), 1, fp);

	char exe_name[MAX_PATH];

	GetModuleFileName(NULL, exe_name, MAX_PATH);
	strcat_s(exe_name, "\n");
	fwrite(exe_name, sizeof(char) * strlen(exe_name), 1, fp);


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
		if (!*p)
			break;	// stupid edge case

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
			case '%':
			{
				buffer[i] = '%';	// escaped %
				i++;
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

void dbugfile_print_matrix_4x4(int line, char *file, float *matrix, char *text)
{
	if(dbugfile_init_var == false)
	{
		return;
	}

	if(matrix == NULL)
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

	if(text == NULL)
	{
		text = "";
	}

	char buffer[1000];

	sprintf(buffer,"[%s,%4d] Matrix %s:\n"
			"%f %f %f %f\n"
		    "%f %f %f %f\n"
		    "%f %f %f %f\n"
		    "%f %f %f %f\n",
			 ptr,line, text,
			 matrix[0],
			 matrix[1],
			 matrix[2],
			 matrix[3],
			 matrix[4],
			 matrix[5],
			 matrix[6],
			 matrix[7],
			 matrix[8],
			 matrix[9],
			 matrix[10],
			 matrix[11],
			 matrix[12],
			 matrix[13],
			 matrix[14],
			 matrix[15]);

 	dbugfile_output(buffer);
}

void dbugfile_print_matrix_3x3(int line, char *file, float *matrix, char *text)
{
	if(dbugfile_init_var == false)
	{
		return;
	}

	if(matrix == NULL)
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

	if(text == NULL)
	{
		text = "";
	}

	char buffer[1000];

	sprintf(buffer,"[%s,%4d] Matrix %s:\n"
			"%f %f %f\n"
		    "%f %f %f\n"
		    "%f %f %f\n",
			 ptr,line, text,
			 matrix[0],
			 matrix[1],
			 matrix[2],
			 matrix[3],
			 matrix[4],
			 matrix[5],
			 matrix[6],
			 matrix[7],
			 matrix[8]);

 	dbugfile_output(buffer);
}

void dbugfile_print_counter(int line, char *file, int counter_num, char *string)
{
	char buffer[1000];
	strcpy_s(buffer, string); 
	strcat_s(buffer, " %d"); 

	dbugfile_sprintf(line, file, buffer, dbugfile_counters[counter_num]);
}



