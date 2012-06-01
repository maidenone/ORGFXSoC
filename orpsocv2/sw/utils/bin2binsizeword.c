/*$$HEADER*/
/******************************************************************************/
/*                                                                            */
/*                    H E A D E R   I N F O R M A T I O N                     */
/*                                                                            */
/******************************************************************************/

// Project Name                   : 
// File Name                      : bin2binsizeword.c
// Prepared By                    : 
// Project Start                  : 

/*$$COPYRIGHT NOTICE*/
/******************************************************************************/
/*                                                                            */
/*                      C O P Y R I G H T   N O T I C E                       */
/*                                                                            */
/******************************************************************************/
/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; 
  version 2.1 of the License, a copy of which is available from
  http://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*$$DESCRIPTION*/
/******************************************************************************/
/*                                                                            */
/*                           D E S C R I P T I O N                            */
/*                                                                            */
/******************************************************************************/
//
// Generates a binary file, but with the first word replaced as the size of the 
// image in big endian format
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Number of bytes before line is broken
   For example if target flash is 8 bits wide,
   define BREAK as 1. If it is 16 bits wide,
   define it as 2 etc.
*/
#define BREAK 1
 
int main(int argc, char **argv)
{

  FILE  *fd, *dest_fd;
  int i = 0;
  int n;
  int filename_index=1;
  int output_filename_index=2;
  unsigned int image_size;
	
  if(argc < 2) 
    {
      fprintf(stderr,"Usage: bin2binsizeword INFILE OUTFILE\n");
      fprintf(stderr,"Copy contents of INFILE to OUTFILE, replacing the first word of INFILE with the\n");
      fprintf(stderr,"size of the file in bytes.\n\n");
      fprintf(stderr,"\tInsufficient options.\n");
      fprintf(stderr,"\tPlease specify an input and then an output name.\n");
      fprintf(stderr,"\n");
      return 1;
    }
  
  // Open input file
  fd = fopen( argv[filename_index], "r" );
  if (fd == NULL) 
    {
      fprintf(stderr,"failed to open input file: %s\n",argv[filename_index]);
      return 1;
    }
  
  // Open the output file
  dest_fd = fopen( argv[output_filename_index], "w");
  if ( dest_fd == NULL )
    {
      fprintf(stderr,"failed to open output file: %s\n",argv[output_filename_index]);
      return 1;
    }
  
  // the very first word in flash is used. Determine the length of this file
  fseek(fd, 0, SEEK_END);
  image_size = ftell(fd);
  fseek(fd,0,SEEK_SET);
	
  // This bit ensures the size word is a word multiple, but that shouldn't be 
  // required.
  //image_size+=3;
  //image_size &= 0xfffffffc;
  
  // Sanity check on image size
  if (image_size < 8){ 
    fprintf(stderr, "Bad binary image. Size too small\n");
    return 1;
  }
  
  // Now write out the image size, BIG ENDIAN
  i=0;
  unsigned char data_byte;
  data_byte = (image_size >> 24) & 0xff;
  fwrite(&data_byte, 1, 1, dest_fd);
  data_byte = (image_size >> 16) & 0xff;
  fwrite(&data_byte, 1, 1, dest_fd);
  data_byte = (image_size >> 8) & 0xff;
  fwrite(&data_byte, 1, 1, dest_fd);
  data_byte = (image_size >> 0) & 0xff;
  fwrite(&data_byte, 1, 1, dest_fd);

  // Now copy the binary file into RAM

  // Fix for the current bootloader software! Skip the first 4 bytes of 
  // application data. Hopefully it's not important. 030509 -- jb
  for(i=0;i<4;i++)
    n = fread(&data_byte,1,1,fd);
  
  while(i<image_size)
    {
      n = fread(&data_byte,1,1,fd);
      if (!n)
	{
	  fprintf(stderr,"error reading input file\n");
	  fclose(fd);
	  fclose(dest_fd);
	  return 1;
	}
      fwrite(&data_byte, 1, 1, dest_fd);
      i++;
    }
  
  fclose(fd);
  fclose(dest_fd);
  
  return 0;
}	
