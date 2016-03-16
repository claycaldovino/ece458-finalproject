/* This file takes in RoW, Bank and Column information 
   and makes a 32-bit address. It also takes in type of 
   CPU request and time issued*/

#include <stdio.h>
#include <stdlib.h>


int askUser()
{
	char confirm;
	
	printf("Do you want to continue (y/n): ");
	scanf("%c",&confirm);
	if(confirm =='y' || confirm =='Y')
		return 1;
	else
	{
		printf("\nExiting... Thank you!!\n");
		return 0;
	}
}
int main()
{
	unsigned row;
	unsigned column;
	unsigned bank;
	unsigned address;
	int response = 1;
	char requestType[6];
	unsigned timeIssued;

	FILE *ofile;		/* Output file pointer */

	if((ofile = fopen("test.tx","w"))== NULL)
		printf("File open not successful. \n");
	else
	{
		do{

		/*Write out values*/
		printf("Enter row position: ");
		scanf("%u", &row);
		getchar();
		printf("Enter bank position: ");
		scanf("%u", &bank);
		getchar();
		printf("Enter column position: ");
		scanf("%u", &column);
		getchar();

		/* Create the address */	
		address = ((row & 0xFF)<<17)|((bank & 0xFF)<<14)|((column & 0xFF)<<3);
		
		printf("Type of request (READ/IFETCH/WRITE): ");
		scanf("%s",requestType);
		getchar();

		printf("CPU request issued: ");
		scanf("%u",&timeIssued);
		getchar();

		/* Write out values */
		fprintf(ofile,"0x%08x \t %s \t %u \n", address,requestType,timeIssued);

		response = askUser();
		printf("\n");
		
		}while(response == 1);

		printf("\nWrote to the file successfully!! \n\n");
		/* Close the file */
		if(fclose(ofile)==EOF)
			printf("File close not successful");
	}

	return 0;
}