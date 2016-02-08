#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct instruction
{
	char name[32];
	char address[32];
	int age;
};

struct node
{
	struct * node next;
}
	
void store_command (struct instruction **commandQueue, struct node *head, FILE *fp);

int main(int argc, char **argv)
{	
	char commandFile[64];
	char buff[128];
	struct instruction *commandQueue[15];
	int full = 0;
	struct node * head = NULL;
	
	for (int i = 0; i < 15; i++)
	{
		commandQueue[i] = NULL;
	}
	
	strcpy(commandFile, argv[1]);
		
	FILE *fp;
	
	fp = fopen(commandFile, "r");
	while (fgets(buff, 128, (FILE*)fp) != NULL && !full)
	{
		printf("Line: %s", buff);
		store_command(buff);
	}
	
	for (int i = 0; i < 15; i++)
	{
		printf ("Data from thing found: %s\n", commandQueue[i]->address);
		free(commandQueue[i]);
	}
	
	fclose(fp);	
	
	return 0;
}

void store_command (struct instruction **commandQueue, struct node *head, FILE *fp)
{
	struct instruction * commandPTR = malloc(sizeof(struct instruction));

	string += 2;
	
	strncpy (commandPTR->address, string, 8);
	
	return void;
}

