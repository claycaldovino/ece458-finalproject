#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DRAM timing constraints

#define tRC 50
#define tRAS 36
#define tRRD 6
#define tRP 14
#define tRFC 172
#define tCWL 10 // (tCWD)
#define tCAS 14 // (CL)
#define tRCD 14
#define tWR 16
#define tRTP 8
#define tBURST 4
#define tWTR 8

struct instruction
{
	char name[32];
	char address[32];
	int age;
};

struct instruction * store_command(char *string);


int main(int argc, char **argv)
{	
	char commandFile[64];
	char buff[128];
	char * trimmedCommand;
	struct instruction *commandQueue[15] = {NULL};
	int openSlot = 0;
	int i;

	
	strcpy(commandFile, argv[1]);
		
	FILE *fp;
	
	fp = fopen(commandFile, "r");
	while (fgets(buff, 128, (FILE*)fp) != NULL)
	{
		printf("Line: %s", buff);
		commandQueue[openSlot] = store_command(buff);
		openSlot++;
	}
	
	for (i = 0; i < openSlot; i++)
	{
		printf ("Data from thing found: %s\n", commandQueue[i]->address);
	}
	
	for (i = openSlot-1; i >= 0; i--)
	{
		free(commandQueue[i]);
	}
	
	fclose(fp);
	
	return 0;
}

struct instruction * store_command (char *string)
{
	struct instruction * commandPTR = malloc(sizeof(struct instruction));

	string += 2;
	
	strncpy (commandPTR->address, string, 8);
	
	return commandPTR;
}

