/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Cfilearchiver/CfileArchiver.cpp $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:21 $
 * $Author: penguin $
 *
 * Program to create an archive file for use with cfile stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/23/98 6:15p Dave
 *
 * $NoKeywords: $
 */

#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <string.h>
#include <conio.h>

unsigned int Total_size=16; // Start with size of header
unsigned int Num_files =0;
FILE * fp_out = NULL;
FILE * fp_out_hdr = NULL;

typedef struct vp_header {
	char id[4];
	int version;
	int index_offset;
	int num_files;
} vp_header;

//vp_header Vp_header;

char archive_dat[1024];
char archive_hdr[1024];

#define BLOCK_SIZE (1024*1024)
#define VERSION_NUMBER 2;

char tmp_data[BLOCK_SIZE];		// 1 MB

void write_header()
{
	fseek(fp_out, 0, SEEK_SET);
	fwrite("VPVP", 1, 4, fp_out);
	int ver = VERSION_NUMBER;
	fwrite(&ver, 1, 4, fp_out);
	fwrite(&Total_size, 1, 4, fp_out);
	fwrite(&Num_files, 1, 4, fp_out);
}

int write_index(char *hf, char *df)
{
	FILE *h = fopen(hf, "rb");
	if (!h) return 0;
	FILE *d = fopen(df, "a+b");
	if (!d) return 0;
	for (unsigned int i=0;i<Num_files;i++) {
		fread(tmp_data, 32+4+4+4, 1, h);
		fwrite(tmp_data, 32+4+4+4, 1, d);
	}
	fclose(h);
	fclose(d);
	return 1;
}

void pack_file( char *filespec, char *filename, int filesize, time_t time_write )
{
	char path[1024];

	if ( strstr( filename, ".vp" ))	{
		// Don't pack yourself!!
		return;
	}

	if ( strstr( filename, ".hdr" ))	{
		// Don't pack yourself!!
		return;
	}

	if ( filesize == 0 ) {
		// Don't pack 0 length files, screws up directory structure!
		return;
	}

	memset( path, 0, sizeof(path));
	strcpy( path, filename );
	if ( strlen(filename)>31 )	{
		printf( "Filename '%s' too long\n", filename );
		exit(1);
	}
	fwrite( &Total_size, 1, 4, fp_out_hdr );
	fwrite( &filesize, 1, 4, fp_out_hdr );
	fwrite( &path, 1, 32, fp_out_hdr );
	fwrite( &time_write, 1, sizeof(time_t), fp_out_hdr);

	Total_size += filesize;
	Num_files++;
	printf( "Packing %s\\%s...", filespec, filename );

	sprintf( path, "%s\\%s", filespec, filename );

	FILE *fp = fopen( path, "rb" );
	
	if ( fp == NULL )	{
		printf( "Error opening '%s'\n", path );
		exit(1);
	}

	int nbytes, nbytes_read=0;

	do	{
		nbytes = fread( tmp_data, 1, BLOCK_SIZE, fp );
		if ( nbytes > 0 )	{
			fwrite( tmp_data, 1, nbytes, fp_out );
			nbytes_read += nbytes;

		}
	} while( nbytes > 0 );

	fclose(fp);

	printf( " %d bytes\n", nbytes_read );
}

// This function adds a directory marker to the header file
void add_directory( char * dirname)
{
	char path[256];
	char *pathptr = path;
	char *tmpptr;

	strcpy(path, dirname);
	fwrite(&Total_size, 1, 4, fp_out_hdr);
	int i = 0;
	fwrite(&i, 1, 4, fp_out_hdr);
	// strip out any directories that this dir is a subdir of
	while ((tmpptr = strchr(pathptr, '\\')) != NULL) {
		pathptr = tmpptr+1;
	}
	fwrite(pathptr, 1, 32, fp_out_hdr);
	fwrite(&i, 1, 4, fp_out_hdr); // timestamp = 0
	Num_files++;
}

void pack_directory( char * filespec)
{
	int find_handle;
	_finddata_t find;
	char tmp[512];
	char tmp1[512];

/*
	char dir_name[512];
	char *last_slash;

	last_slash = strrchr(filespec, '\\');
	if ( last_slash ) {
		strcpy(dir_name, last_slash+1);
	} else {
		strcpy(dir_name, filespec);
	}

	if ( !stricmp(dir_name, "voice") ) {
		return;
	}
*/

	strcpy( tmp1, filespec );
	add_directory(filespec);
	strcat( tmp1, "\\*.*" );
	
	printf( "In dir '%s'\n", tmp1 );

	find_handle = _findfirst( tmp1, &find );
	if( find_handle != -1 )	{
		if ( find.attrib & _A_SUBDIR )	{
			if (strcmp( "..", find.name) && strcmp( ".", find.name))	{
				strcpy( tmp, filespec );
				strcat( tmp, "\\" );
				strcat( tmp, find.name );
				pack_directory(tmp);
			}
		} else {
			pack_file( filespec, find.name, find.size, find.time_write );
		}

		while( !_findnext( find_handle, &find ) )	{
			if ( find.attrib & _A_SUBDIR )	{
				if (strcmp( "..", find.name) && strcmp( ".", find.name))	{
					strcpy( tmp, filespec );
					strcat( tmp, "\\" );
					strcat( tmp, find.name );
					pack_directory(tmp);

				}
			} else {
				pack_file( filespec, find.name, find.size, find.time_write );
			}
		}
	}
	add_directory("..");
}



int main(int argc, char *argv[] )
{
	char archive[1024];
	char *p;

	if ( argc < 3 )	{
		printf( "Usage: %s archive_name src_dir\n", argv[0] );
		printf( "Example: %s freespace c:\\freespace\\data\n", argv[0] );
		printf( "Creates an archive named freespace out of the\nfreespace data tree\n" );
		printf( "Press any key to exit...\n" );
		getch();
		return 1;
	}

	strcpy( archive, argv[1] );
	p = strchr( archive, '.' );
	if (p) *p = 0;		// remove extension	

	strcpy( archive_dat, archive );
	strcat( archive_dat, ".vp" );

	strcpy( archive_hdr, archive );
	strcat( archive_hdr, ".hdr" );

	fp_out = fopen( archive_dat, "wb" );
	if ( !fp_out )	{
		printf( "Couldn't open '%s'!\n", archive_dat );
		printf( "Press any key to exit...\n" );
		getch();
		return 1;
	}

	fp_out_hdr = fopen( archive_hdr, "wb" );
	if ( !fp_out_hdr )	{
		printf( "Couldn't open '%s'!\n", archive_hdr );
		printf( "Press any key to exit...\n" );
		getch();
		return 1;
	}

	write_header();

	pack_directory( argv[2] );

	write_header();

	fclose(fp_out);
	fclose(fp_out_hdr);

	printf( "Data files written, appending index...\n" );

	if (!write_index(archive_hdr, archive_dat)) {
		printf("Error appending index!\n");
		printf("Press any key to exit...\n");
		getch();
		return 1;
	}
	
	printf( "%d total KB.\n", Total_size/1024 );
	return 0;
}
