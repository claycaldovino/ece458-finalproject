#include "request.h"
#include <string.h>

unsigned long cpuTime = 0;

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
void updateDimmStatus(command cmd, unsigned bank, unsigned row);
bool isCommandLegal(command cmd, unsigned bank, unsigned row);
void initializeTimers();
void updateTimers(command cmd, unsigned bank);
void incrementTimers();

void policyManager()
{	
	int lastCommand = 10;
	command nextCommand;
	bool isLegal = TRUE;
	int lastPriority = -3;
	int comparePriority = 0;
	int chosenIndex;
	int queueIndex;
	
	for (queueIndex = 0; queueIndex < 16; ++queueIndex)
	{
		if (requestQueue[queueIndex].occupied & 
			!requestQueue[queueIndex].finished)
		{
			nextCommand = findNextCommand(queueIndex);
			
			isLegal = isCommandLegal(nextCommand, 
				requestQueue[queueIndex].bank, 
				requestQueue[queueIndex].row);
				
			if (isLegal)
			{	
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
					lastCommand = nextCommand;
				}
				else if (comparePriority == lastPriority)
				{
					if (requestQueue[queueIndex].timeIssued < 
						requestQueue[chosenIndex].timeIssued)
					{
						chosenIndex = queueIndex;
						lastCommand = nextCommand;
					}
				}
			}
		
		}
	}

	switch (lastCommand)
	{
		case PRE:
			printf("CPU:XX PRE %d\n", requestQueue[chosenIndex].bank);
			updateDimmStatus(lastCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			
		break;
		
		case ACT:
			printf("CPU:XX ACT %d %d\n", requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateDimmStatus(lastCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(lastCommand, requestQueue[chosenIndex].bank);
		break;
		
		case RD:
			printf("CPU:XX RD %d %d\n", requestQueue[chosenIndex].bank, requestQueue[chosenIndex].column);
			requestQueue[chosenIndex].finished = TRUE;
			updateDimmStatus(lastCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(lastCommand, requestQueue[chosenIndex].bank);
		break;
		
		case WR:
			printf("CPU:XX WR %d %d\n", requestQueue[chosenIndex].bank, requestQueue[chosenIndex].column);
			requestQueue[chosenIndex].finished = TRUE;
			updateDimmStatus(lastCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(lastCommand, requestQueue[chosenIndex].bank);
		break;
			
		default:
			printf("Skips this clock cycle -- no legal command.\n");
			incrementTimers();
		break;
	}
	
	cpuTime += 4;
}

int prechargePriority(int queueIndex)
{
	int bank = requestQueue[queueIndex].bank;
	int row = dimmStatus[bank].activeRow;
	int priority = 3;
	int i;
	
	for (i = 0; i < 16; ++i)
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
	int i;
	
	for (i = 0; i < 16; ++i)
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
	int i;
	
	for (i = 0; i < 16; ++i)
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

bool isCommandLegal(command cmd, unsigned bank, unsigned row)
{
	bool check = FALSE;
	int i;
	
	switch (cmd)
	{
		case PRE:
			if (commandTimers[bank][ACT] >= tRAS &&
				commandTimers[bank][RD] >= tRTP &&
				commandTimers[bank][WR] >= (tWR + tCWL + tBURST))
				check = TRUE;
			else
				check = FALSE;
			break;
			
		case ACT:
			if (commandTimers[bank][PRE] >= tRP)
			{			
				for(i = 0; i < 8; ++i)
				{
					if (commandTimers[i][ACT] >= tRRD)
						check = TRUE;
					else
						check = FALSE;
						break;
				}
			}
			
			break;
			
		case RD:
			if (commandTimers[bank][ACT] >= tRCD)
			{
				for(i = 0; i < 8; ++i)
				{
					if (commandTimers[i][RD] >= tCCD &&
						commandTimers[i][WR] >= (tWTR + tCWL + tBURST))
						check = TRUE;
					else
						check = FALSE;
				}
			}
			
			break;
			
		case WR:
			if (commandTimers[bank][ACT] >= tRCD)
			{
				for(i = 0; i < 8; ++i)
				{
					if (commandTimers[i][WR] >= tCCD &&
						commandTimers[i][RD] >= 12) //PALCEHOLDER
						check = TRUE;
					else
						check = FALSE;
				}
			}
	}
	
	return check;
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

void initializeTimers()
{
	int i,j;
	
	for (i = 0; i < TOTAL_BANKS; ++i)
		for (j = 0; j < 4; ++j)
			commandTimers[i][j] = 200;	
	
}

void updateTimers(command cmd, unsigned bank)
{
	int i, j;
	
	commandTimers[bank][cmd] = 0;
	
	for (i = 0; i < TOTAL_BANKS; ++i)
		for (j = 0; j < 4; ++j)
			if (commandTimers[i][j] <= 200)
				commandTimers[i][j] += 1;

}

void incrementTimers()
{
	int i, j;
	
	for (i = 0; i < TOTAL_BANKS; ++i)
		for (j = 0; j < 4; ++j)
			if (commandTimers[i][j] <= 200)
				commandTimers[i][j] += 1;
}

int main()
{
	int i;
	
	for (i = 0; i < 16; ++i)
	{
		requestQueue[i].occupied = FALSE;
		requestQueue[i].finished = FALSE;
	}
	
	initializeTimers();
	
	strcpy(requestQueue[2].name, "READ");
	requestQueue[2].row = 113;
	requestQueue[2].bank = 1;
	requestQueue[2].column = 224;
	requestQueue[2].timeIssued = 512;
	requestQueue[2].occupied = TRUE;
	requestQueue[2].finished = FALSE;
	requestQueue[2].timeRemaining = 0;
	
	strcpy(requestQueue[3].name, "READ");
	requestQueue[3].row = 156;
	requestQueue[3].bank = 1;
	requestQueue[3].column = 111;
	requestQueue[3].timeIssued = 514;
	requestQueue[3].occupied = TRUE;
	requestQueue[3].finished = FALSE;
	requestQueue[3].timeRemaining = 0;
	
	strcpy(requestQueue[4].name, "READ");
	requestQueue[4].row = 113;
	requestQueue[4].bank = 1;
	requestQueue[4].column = 696969;
	requestQueue[4].timeIssued = 515;
	requestQueue[4].occupied = TRUE;
	requestQueue[4].finished = FALSE;
	requestQueue[4].timeRemaining = 0;
	
	dimmStatus[1].isPrecharged = FALSE;
	dimmStatus[1].isActivated = FALSE;
	dimmStatus[1].activeRow = 0;
	
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();
	policyManager();

	
	return 0;
}

