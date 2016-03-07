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

