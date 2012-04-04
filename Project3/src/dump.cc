/* a simple dump program
   prints the offset, hexvalue, and decimal value for each byte in a
   file, for all files mentioned on the command line */

#include <io_types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

static const String PROGRAM_NAME = "dump";

int main(int argc, char **args)
{
  int fd;
  if(argc < 2)
  {
  	fprintf(stderr,"%s%s%s%s\n", PROGRAM_NAME , ": usage: ./" ,
	PROGRAM_NAME , " input-files");
  }

  for (int i = 1 ; i < argc; i++)
    {
      fd = open (args[i], O_RDONLY);

      // while we are able to read bytes from it
      unsigned char c ;
      for ( int j = 0 ; (read(fd, &c, 1) ) >0 ; j ++ )
        {
          if( c > 0 )
          {
            fprintf(stdout, "%d%s%x%s%d", j, " ", c,  " ", (int)c ) ;
            if( c >= 32 && c < 127 )
              fprintf(stdout, "%s%c", " ", (char)c ) ;
	    fprintf (stdout, "\n");
          }
        }
        close(fd) ;
    }
}
