/* This file takes in RoW, Bank and Column, type of request
   and time issued information and makes a 32-bit address. 
   It also takes in type of CPU request and time issued*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define RUNTIME 15000

typedef enum {READ, WRITE, IFETCH} command;

unsigned rand_lim(unsigned limit) {
/* return a random number between 0 and limit inclusive. */

    unsigned divisor = RAND_MAX/(limit+1);
    unsigned retval;

    do { 
        retval = rand() / divisor;
    } while (retval > limit);

    return retval;
}

int main()
{
	srand(time(NULL));
	int i;
	unsigned address_lsb;
	unsigned address_msb;	
	command cmd;

	FILE *ofile;		/* Output file pointer */

	if((ofile = fopen("rng_test.txt","w"))== NULL)
		printf("File open not successful. \n");
	else
	{
		for(i = 0; i < RUNTIME; ++i)
		{
			cmd = rand_lim(2);
			address_msb = rand_lim(65535);
			address_lsb = rand_lim(65535);
			
			switch (cmd)
			{
				case READ:
					fprintf(ofile, "0x%04x%04x\tREAD\t%u\n", address_msb, address_lsb, i);
					break;
					
				case WRITE:
					fprintf(ofile, "0x%04x%04x\tWRITE\t%u\n", address_msb, address_lsb, i);
					break;
				
				case IFETCH:
					fprintf(ofile, "0x%04x%04x\tIFETCH\t%u\n", address_msb, address_lsb, i);
					break;
				}
		
		}
	}

	return 0;
}
