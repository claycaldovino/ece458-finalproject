#include "request.h"

int main(int argc, char **argv)
{
	FILE *fp;		/* File handler */
	
	/*================================================================================*/
		/* Open the file */
        if (argc != 2)
        {
                printf("You must enter a testfile to use, 'simulate testfile.txt'\n");
                return -1;
        }
 		
		if ((fp = fopen(argv[1],"r")) == NULL)
        {
                printf("Could not open file: %s\n", argv[1]);
                return -1;
        }
	