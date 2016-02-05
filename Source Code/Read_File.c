#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	struct instruction *commandQueue[15];
	int openSlot = 0;
	
	
	for (int i = 0; i < 15; i++)
	{
		commandQueue[i] = NULL;
	}
	
	strcpy(commandFile, argv[1]);
		
	FILE *fp;
	
	fp = fopen(commandFile, "r");
	while (fgets(buff, 128, (FILE*)fp) != NULL)
	{
		printf("Line: %s", buff);
		commandQueue[openSlot] = store_command(buff);
		openSlot++;
	}
	
	for (int i = 0; i < openSlot; i++)
	{
		printf ("Data from thing found: %s\n", commandQueue[i]->address);
	}
	
	for (int i = openSlot-1; i >= 0; i--)
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

