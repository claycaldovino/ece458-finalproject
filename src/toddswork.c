#include "request.h"
#include <string.h>

const unsigned long long STARVATION_LIMIT = 50;

// Precharge, Activate, Read, Write commands
// used for column indices of the commandTimers array
typedef enum {PRE, ACT, RD, WR} command;

unsigned commandTimers[TOTAL_BANKS][4];

typedef struct
{
    bool isPrecharged;
	bool isActivated;
	unsigned activeRow;
} bankStatus;

bankStatus dimmStatus[TOTAL_BANKS];

int prechargePriority(int queueIndex);
int readPriority(int queueIndex);
int writePriority(int queueIndex);
command findNextCommand(int rqIndex);

void policyManager()
{	
	command nextCommand;
	int lastPriority = -3;
	int comparePriority = 0;
	int chosenIndex;
	
	for (int queueIndex = 0; queueIndex < 16; ++queueIndex)
	{
		if (requestQueue[queueIndex].occupied & 
			!requestQueue[queueIndex].finished)
		{
			nextCommand = findNextCommand(queueIndex);
			switch(nextCommand)
			{
				case PRE :
					comparePriority = prechargePriority(queueIndex);
					break;
					
				case ACT :
					comparePriority = 2;
					break;
				
				case RD :
					comparePriority = readPriority(queueIndex);
					break;
					
				case WR :
					comparePriority = writePriority(queueIndex);
					break;
					
				default :
				printf("\nThe FUCK Todd!?!?\n");
			}
			
			if (comparePriority > lastPriority)
			{
				lastPriority = comparePriority;
				chosenIndex = queueIndex;
			}
			else if (comparePriority == lastPriority)
			{
				if (requestQueue[queueIndex].timeIssued > 
					requestQueue[chosenIndex].timeIssued)
				{
					chosenIndex = queueIndex;
				}
			}
		
		}
	}
	
	printf("The chosen command: %s \nThe index number: %d \n :The priority level: %d\n", requestQueue[chosenIndex].name, chosenIndex, lastPriority);
}

int prechargePriority(int queueIndex)
{
	int bank = requestQueue[queueIndex].bank;
	int row = dimmStatus[bank].activeRow;
	int priority = 3;
	
	for (int i = 0; i < 16; ++i)
	{
		if (requestQueue[i].occupied && !requestQueue[i].finished)
		{
			if ((requestQueue[i].bank == bank) && (requestQueue[i].row == row))
			{
				priority = 0;
				return priority;
			}
			else
			{
				priority = 3;
			}
		}
	}
	
	return priority;
}

int readPriority(int queueIndex)
{
	int bank = requestQueue[queueIndex].bank;
	int row = requestQueue[queueIndex].row;
	int col = requestQueue[queueIndex].column;
	int timestamp = requestQueue[queueIndex].timeIssued;
	int priority = 4;
	
	for (int i = 0; i < 16; ++i)
	{
		if (requestQueue[i].occupied & !requestQueue[i].finished)
		{
			if (!strcmp(requestQueue[i].name, "WRITE") &&
				requestQueue[i].bank == bank &&
				requestQueue[i].row == row &&
				requestQueue[i].column == col)	
			{
				if (requestQueue[i].timeIssued < timestamp)
				{
					priority = -2;
					return priority;
				}
				else
				{
					priority = 4;
				}
			}
		}
	}
	
	return priority;
}

int writePriority(int queueIndex)
{
	int bank = requestQueue[queueIndex].bank;
	int row = requestQueue[queueIndex].row;
	int col = requestQueue[queueIndex].column;
	int timestamp = requestQueue[queueIndex].timeIssued;
	int priority = 4;
	
	for (int i = 0; i < 16; ++i)
	{
		if (requestQueue[i].occupied & !requestQueue[i].finished)
		{
			if ((!strcmp(requestQueue[i].name, "READ") ||
				!strcmp(requestQueue[i].name, "IFETCH")) &&
				requestQueue[i].bank == bank &&
				requestQueue[i].row == row &&
				requestQueue[i].column == col)	
			{
				if (requestQueue[i].timeIssued < timestamp)
				{
					priority = -2;
					return priority;
				}
				else
				{
					priority = 4;
				}
			}
		}
	}
	
	return priority;
}

int findStarvation()
{
	int i;
	int mostStarvedRequest = -1;  // default to no starving commands 
	unsigned long long starvationTemp = 0, starvationAmount = 0; 
	
	for (i = 0; i < TOTAL_BANKS; ++i)
	{
		if (requestQueue[i].occupied && !requestQueue[i].finished)
		{
			starvationTemp = currentCPUTick - requestQueue[i].timeIssued;
			if (starvationTemp > STARVATION_LIMIT 
			    && starvationTemp > starvationAmount)
			    starvationAmount = starvationTemp;
			    mostStarvedRequest = i;
		}
	}
	return mostStarvedRequest;
}

command findNextCommand(int rqIndex)
{
	// looks at a request in the requestQueue and determines which 
	// command needs to be issued next to fufill the request
	
	unsigned dsIndex = requestQueue[rqIndex].bank;
	unsigned reqRow = requestQueue[rqIndex].row;
	
	// always precharge if bank is neither precharged nor activated
	if (!dimmStatus[dsIndex].isPrecharged  
	    && !dimmStatus[dsIndex].isActivated)
	    return PRE;
	// also precharge if different row is currently active
	else if (dimmStatus[dsIndex].isActivated
	         && reqRow != dimmStatus[dsIndex].activeRow)
	    return PRE;
	// activate if the bank is precharged
	else if (dimmStatus[dsIndex].isPrecharged
	         && !dimmStatus[dsIndex].isActivated)
	    return ACT;
	// read or write if the desired row is already activated
	else
	{
		if (!strcmp(requestQueue[rqIndex].name, "READ")
			|| !strcmp(requestQueue[rqIndex].name, "IFETCH"))
			return RD;
	    else
	        return WR;
	}
}

void updateDimmStatus(command cmd, unsigned bank, unsigned row)
{
	if (cmd == PRE)
	{
		dimmStatus[bank].isPrecharged = TRUE;
		dimmStatus[bank].isActivated  = FALSE;
	}
	else if (cmd == ACT)
	{
		dimmStatus[bank].isPrecharged = FALSE;
		dimmStatus[bank].isActivated = TRUE;
		dimmStatus[bank].activeRow = row;
	}
} 


int main()
{
	// Bank 0 start-up condition
	dimmStatus[0].isPrecharged = FALSE;
	dimmStatus[0].isActivated  = FALSE;
	dimmStatus[0].activeRow    = 0x0;
	
	// Request for read from adress 0x00000000
	strcpy(requestQueue[0].name, "READ");
	requestQueue[0].bank = 0x0;
	requestQueue[0].row  = 0x0;
	
	command cmdBuf;
	
	cmdBuf = findNextCommand(0x0);
	printf("Should be a PRE (0)\n");
	printf("Actually is %i\n\n", cmdBuf);
	
	// Bank 0 is precharged
	dimmStatus[0].isPrecharged = TRUE;
	
	return 0;
}

