#include "request.h"

int prechargePriority(int queueIndex);
int readPriority(int queueIndex);
int writePriority(int queueIndex);
command findNextCommand(int rqIndex);
void updateDimmStatus(command cmd, unsigned bank, unsigned row);
bool isCommandLegal(command cmd, unsigned bank, unsigned row);
void initializeTimers();
void updateTimers(command cmd, unsigned bank);
void incrementTimers();
int findStarvation();

void policyManager()
{	
	int lastCommand = 10;
	command nextCommand;
	bool isLegal = TRUE;
	int lastPriority = -2;
	int comparePriority = 0;
	int chosenIndex;
	int queueIndex;
	
	int starveCheck = findStarvation();
	if (starveCheck != -1)
	{
		printf("Found a starving something\n");
		chosenIndex = starveCheck;
		lastCommand = findNextCommand(starveCheck);
	}
	else
	{
		for (queueIndex = 0; queueIndex < 16; ++queueIndex)
		{
			if (requestQueue[queueIndex].occupied & 
				!requestQueue[queueIndex].finished)
			{
				nextCommand = findNextCommand(queueIndex);
				
				isLegal = isCommandLegal(nextCommand, 
					requestQueue[queueIndex].bank, 
					requestQueue[queueIndex].row);
					
				if (!isLegal)
					continue;
					
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
			printf("CPU:%llu PRE %d\n", currentCPUTick, requestQueue[chosenIndex].bank);
			updateDimmStatus(lastCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(lastCommand, requestQueue[chosenIndex].bank);
		break;
		
		case ACT:
			printf("CPU:%llu ACT %d %d\n", currentCPUTick, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateDimmStatus(lastCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(lastCommand, requestQueue[chosenIndex].bank);
		break;
		
		case RD:
			printf("\nFinished read issued: %llu\n", requestQueue[chosenIndex].timeIssued);
			printf("CPU:%llu RD %d %d\n", currentCPUTick, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].column);
			requestQueue[chosenIndex].finished = TRUE;
			requestQueue[chosenIndex].timeRemaining = tCAS + tBURST;
			updateDimmStatus(lastCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(lastCommand, requestQueue[chosenIndex].bank);
		break;
		
		case WR:
			printf("\nFinished write issued: %llu\n", requestQueue[chosenIndex].timeIssued);		
			printf("CPU:%llu WR %d %d\n", currentCPUTick, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].column);
			requestQueue[chosenIndex].finished = TRUE;
			requestQueue[chosenIndex].occupied = FALSE;
			countSlotsOccupied -= 1;
			printf("\n---Current queue size: %d---\n", countSlotsOccupied);
			updateDimmStatus(lastCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(lastCommand, requestQueue[chosenIndex].bank);
		break;
			
		default:
			printf("CPU:%llu ---\n", currentCPUTick);
			incrementTimers();
		break;
	}
	
}

int prechargePriority(int queueIndex)
{
	int bank = requestQueue[queueIndex].bank;
	int row = dimmStatus[bank].activeRow;
	int priority = 0;
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
				priority = 3;
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
					priority = -10;
					return priority;
				}
				else
					priority = 4;
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
					priority = -10;
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
		if (requestQueue[i].occupied && !requestQueue[i].finished &&
			isCommandLegal(findNextCommand(i), requestQueue[i].bank, requestQueue[i].row))
		{
			starvationTemp = currentCPUTick - requestQueue[i].timeIssued;
			
			if (starvationTemp > STARVATION_LIMIT 
			    && starvationTemp > starvationAmount)
			{
			    printf("Starved %llu cycles\n", starvationTemp);
			    starvationAmount = starvationTemp;
			    mostStarvedRequest = i;
			}
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
					{
						check = FALSE;
						break;
					}
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
					{
						check = FALSE;
						break;
					}
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
					{
						check = FALSE;
						break;
					}
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
			if (commandTimers[i][j] < 200)
				commandTimers[i][j] += 1;

}

void incrementTimers()
{
	int i, j;
	
	for (i = 0; i < TOTAL_BANKS; ++i)
		for (j = 0; j < 4; ++j)
			if (commandTimers[i][j] < 200)
				commandTimers[i][j] += 1;
}

/*===============================================================================*/
/* fill an array slot if empty */
bool enqueue(int *countSlotsOccupied, queue_state *lastQueueStatus)
{
	int loopVar;

	
	for(loopVar = 0; loopVar <ARRAY_SIZE; ++loopVar)
	{	/* Find an open spot to enqueue */
		
		 if(requestQueue[loopVar].occupied == NO)
		{

			/* Check if last queue status was full. If yes, the time  */
			if(lastQueueStatus==FULL) /* Array was full */
			{
				inputBuffer.timeIssued = currentCPUTick;   /* The current enqueue time is updated */
				lastQueueStatus =NORMAL;    /* Update the flag to normal */
			}
			/*Open slot found. Copy the contents of the temporary buffer */
		 	strcpy(requestQueue[loopVar].name,inputBuffer.name);
			requestQueue[loopVar].fullAddress 	= inputBuffer.fullAddress;
			requestQueue[loopVar].row		 	= inputBuffer.row;
			requestQueue[loopVar].column 		= inputBuffer.column;
			requestQueue[loopVar].bank			= inputBuffer.bank;
			requestQueue[loopVar].timeIssued 	= inputBuffer.timeIssued;
			requestQueue[loopVar].occupied 		= YES;	/* The slot is filled now */
			/* Initialize other struct members */
			requestQueue[loopVar].finished 		= NO;
			requestQueue[loopVar].timeRemaining = NO;
			*countSlotsOccupied  = *countSlotsOccupied +1;	/*Increment counter */
			
			if(*countSlotsOccupied == ARRAY_SIZE-1)
				lastQueueStatus = FULL;
			printf("\n+++Current queue size: %d+++\n", *countSlotsOccupied);
						
			return FALSE;  /* bufferFutureRequest= FALSE. Reload the buffer with CPU request next time */
		}
	}
	
	return TRUE;      /* Enqueu operation failed. so bufferFutureRequest = TRUE. */
}
/*===================================================================================*/
/*  Load the input buffer from the file*/
bool loadInputBuffer(FILE *fp)
{
		char buf[128];							    /* Temporary buffer */
		char *token;							    /* Token for file read */
		if(fgets(buf,128,fp)==NULL)
		{
			return TRUE;   							/* File is empty */
		}
				 
		/* Load the next CPU request */
		else   
		{
			/* Fill the temporary buffer with the content of CPU */
			token = strtok(buf,"\t\n ");
			inputBuffer.fullAddress = (unsigned int) strtol(token,NULL,16);
			/*Split the address into row, bank and column */
			inputBuffer.row = (inputBuffer.fullAddress & 0xFFFE0000)>>17;
			inputBuffer.bank = (inputBuffer.fullAddress & 0x1C000)>>14 ;
			inputBuffer.column = (inputBuffer.fullAddress & 0x3FF8)>>3;
			/* Fill the request name*/
			token = strtok(NULL,"\t\n ");
			strcpy(inputBuffer.name,token);
			/* Fill the CPU request time */
			token = strtok(NULL,"\t\n ");
			inputBuffer.timeIssued = atoi(token);
			return FALSE;								/* Not the end of the file */
		}
}

/*=======================================================================*/
/* This function checks for a request in the queue that is complete.If the
	request is completed, it decrements the slots occupied counter, and clears
	the slot occupied variable */

void examineQueueForCompletion(int * countSlotsOccupied)
{	
		int loopVar	= 0;					/* Temporary loop variable */

	/* Remove all the requests  that are completed*/
		for(loopVar=0; loopVar <ARRAY_SIZE ; ++loopVar)
		{
			if(requestQueue[loopVar].occupied && requestQueue[loopVar].finished)
			{
					requestQueue[loopVar].timeRemaining = requestQueue[loopVar].timeRemaining -1;
					
					if(requestQueue[loopVar].timeRemaining==0)
					{
						requestQueue[loopVar].occupied = FALSE;      /* The slot is open for enqueue */
						*countSlotsOccupied = *countSlotsOccupied - 1;	/* Decreament the queue counter */
						printf("\n---Current queue size: %d---\n", *countSlotsOccupied);

					}
			}
		}
}
/*====================================================================*/
int main(int argc, char **argv)
{
	FILE *fp;								    	/* File handler */
	bool endOfFile;							   		/* End of file variable */
	bool bufferFutureRequest  	 = FALSE;				/* The CPU request is for future time. Viz."inputBuffer hold-On" */
	currentCPUTick     	  	 = 0;					/* Start of the simulation */
	queue_state lastQueueStatus;						/* Enumerated queue status variable */
	int i;
	
	lastQueueStatus = EMPTY;							/* Initialize current queue status as empty */
	for (i = 0; i < 16; ++i)

	{
		requestQueue[i].occupied = NO;
		requestQueue[i].finished = NO;
	}
	
	initializeTimers();

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
	/*====================================================================================*/
	/* This is a big while loop that is exited only when there is no more CPU requests (in other
	 * words end-of-file reached) and the queue is completely empty */
	/*===================================================================================*/
	
	/*Examine if there is anything in the file. If nothing is in the file, no more program
	 * execution needed */
	endOfFile = loadInputBuffer(fp);

	if(!endOfFile)
		bufferFutureRequest = YES;

	
	while (!endOfFile || (lastQueueStatus != EMPTY))
	{
		/*====================================================================*/
		while (!endOfFile && (countSlotsOccupied !=(ARRAY_SIZE-1)))  
		{
			
			/*==============================================*/
			if(bufferFutureRequest == YES)
			{
				/* CASE 1: Queue was empty */
				if(lastQueueStatus==EMPTY)
				{
					// This will move us off DRAM cycle ticks -- 
					// needs to be next valid DRAM cycle instead.
					if(inputBuffer.timeIssued >currentCPUTick)
						currentCPUTick =inputBuffer.timeIssued;
				}
				
				if(inputBuffer.timeIssued > currentCPUTick)
					break;  		/* It's not yet time to enqueue. So break the loop */
				
				bufferFutureRequest = enqueue(&countSlotsOccupied, &lastQueueStatus);	/* Queue in the first open slot found */
			}
			
			/* Load another image from */

			else if(!endOfFile)
				{
					/* Load another request from the file */
					endOfFile=loadInputBuffer(fp);     /* Fill the temporary buffer */
					if(!endOfFile)
						bufferFutureRequest = YES;
					
				}
			/*========================================================*/
								
		}
		
		/* Call a function to see if any request has been completed */
		
		examineQueueForCompletion(&countSlotsOccupied);
		
		if(countSlotsOccupied==0)
			lastQueueStatus ==EMPTY;
	/* Do the DRAM service */

	policyManager();
		
	/*=========================================================================*/
	/* Service the DRAM */
	
	currentCPUTick += 4;

	}  
	/*========================================================================*/
	
return 0;
}
				
		
	
			
		
