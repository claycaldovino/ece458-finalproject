#include "request.h"

int prechargePriority(int queueIndex);
int readPriority(int queueIndex);
int writePriority(int queueIndex);
command findNextCommand(int rqIndex);
void updateDimmStatus(command cmd, unsigned bank, unsigned row);
bool isCommandLegal(command cmd, unsigned bank, unsigned row, int index, bool starvedCommand);
int calculateWindow(const command cmd, const unsigned bank, unsigned long long *lower, unsigned long long *upper);
void initializeTimers();
void updateTimers(command cmd, unsigned bank);
void incrementTimers(unsigned long n);
int findStarvation();
void policyManager(FILE *ofile);

void policyManager(FILE *ofile)
{	
	command chosenCommand = WAIT;
	command nextCommand;
	bool isLegal = TRUE;
	int chosenPriority = -2;
	int comparePriority = 0;
	int chosenIndex;
	int queueIndex;
	
	int starveCheck = findStarvation();
	if (starveCheck != -1)
	{
		chosenIndex = starveCheck;
		chosenCommand = findNextCommand(starveCheck);
		chosenPriority = 10;
		
		starvationStatus.isCommandStarving = TRUE;
		starvationStatus.name = chosenCommand;
		starvationStatus.bank = requestQueue[chosenIndex].bank;
		
		calculateWindow(starvationStatus.name, starvationStatus.bank, &starvationStatus.lowerWindow, &starvationStatus.upperWindow);
				
		if (!isCommandLegal(chosenCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row, chosenIndex, TRUE))
		{
			chosenCommand = WAIT;
			chosenPriority = -1;
		}
	}
	else
	{
		starvationStatus.isCommandStarving = FALSE;
	}
	
	for (queueIndex = 0; queueIndex < 16; ++queueIndex)
	{
		if (requestQueue[queueIndex].occupied && 
			!requestQueue[queueIndex].finished && 
			queueIndex != starveCheck)
		{
			nextCommand = findNextCommand(queueIndex);
			
			isLegal = isCommandLegal(nextCommand, 
				requestQueue[queueIndex].bank, 
				requestQueue[queueIndex].row,
				queueIndex, FALSE);
				
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
					comparePriority = 4;
					break;
					
				case WR :
					comparePriority = 4;
					break;
					
				case WAIT:
					comparePriority = -1;
					
				default :
				printf("\nERROR: Unknown Command\n");
			}
			
			if (comparePriority > chosenPriority)
			{
				chosenPriority = comparePriority;
				chosenIndex = queueIndex;
				chosenCommand = nextCommand;
			}
			else if (comparePriority == chosenPriority)
			{
				if (requestQueue[queueIndex].timeIssued < 
					requestQueue[chosenIndex].timeIssued)
				{
					chosenIndex = queueIndex;
					chosenCommand = nextCommand;
				}
			}		
		}
	}
	
	switch (chosenCommand)
	{
		case PRE:
			fprintf(ofile, "%llu\tPRE\t%d\n", currentCPUTick, requestQueue[chosenIndex].bank);
			updateDimmStatus(chosenCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(chosenCommand, requestQueue[chosenIndex].bank);
		break;
		
		case ACT:
			fprintf(ofile, "%llu\tACT\t%d\t%d\n", currentCPUTick, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateDimmStatus(chosenCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(chosenCommand, requestQueue[chosenIndex].bank);
		break;
		
		case RD:
			fprintf(ofile, "%llu\tRD\t%d\t%d\n", currentCPUTick, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].column);
			requestQueue[chosenIndex].finished = TRUE;
			requestQueue[chosenIndex].timeRemaining = tCAS + tBURST;
			updateDimmStatus(chosenCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(chosenCommand, requestQueue[chosenIndex].bank);
		break;
		
		case WR:
			fprintf(ofile, "%llu\tWR\t%d\t%d\n", currentCPUTick, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].column);
			requestQueue[chosenIndex].finished = TRUE;
			requestQueue[chosenIndex].occupied = FALSE;
			countSlotsOccupied -= 1;
			updateDimmStatus(chosenCommand, requestQueue[chosenIndex].bank, requestQueue[chosenIndex].row);
			updateTimers(chosenCommand, requestQueue[chosenIndex].bank);
		break;
			
		default:
			//printf("CPU:%llu ---\n", currentCPUTick);
			incrementTimers(1);
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

int calculateWindow(const command cmd, const unsigned bank, unsigned long long *lower, unsigned long long *upper)
{
	unsigned long long temp1 = 0;
	unsigned long long temp2 = 0;
	unsigned long long temp3 = 0;
	int loopCheck;
	int i;
	
	switch (cmd)
	{
		case PRE:
			return 0;
			
		case ACT:
			for(i=0; i < 8; ++i)
			{
				loopCheck = (tRRD - commandTimers[i][ACT]) * 4;
				if (loopCheck > 0 && loopCheck > temp1)
				{
					temp1 = loopCheck;
				}
			}
			
			temp1 = temp1 + currentCPUTick;			
			
			if (tRP - commandTimers[bank][cmd] > 0)
				temp2 = currentCPUTick + ((tRP - commandTimers[bank][cmd]) * 4);
				
			if (temp1 > temp2)
				*upper = temp1;
			else
				*upper = temp2;
				
			*lower = *upper - ((tRRD - 1) * 4);
			return 0;
		
		case RD :
			for(i=0; i < 8; ++i)
			{
				loopCheck = (tBURST - commandTimers[i][RD]) * 4;
				if (loopCheck > 0 && loopCheck > temp1)
				{
					temp1 = loopCheck;
				}
			}
			
			temp1 = temp1 + currentCPUTick;
			
			if (tRCD - commandTimers[bank][ACT] > 0)
				temp2 = currentCPUTick + ((tRCD - commandTimers[bank][ACT]) * 4);
			
			if (tWTR - commandTimers[bank][WR] > 0)
				temp3 = currentCPUTick + ((tWTR - commandTimers[bank][WR]) * 4);
				
			if (temp1 > temp2)
			{
				if (temp1 > temp3)
					*upper = temp1;
				else
					*upper = temp3;
			}
			else
			{
				if (temp2 > temp3)
					*upper = temp2;
				else
					*upper = temp3;
			}
			
			*lower = *upper - (tBURST * 4);
			
			return 0;
		
		case WR :
			for(i=0; i < 8; ++i)
			{
				loopCheck = (tBURST - commandTimers[i][WR]) * 4;
				if (loopCheck > 0 && loopCheck > temp1)
				{
					temp1 = loopCheck;
				}
			}
			
			temp1 = temp1 + currentCPUTick;
			
			for(i=0; i < 8; ++i)
			{
				loopCheck = ((tCAS - tCWL + tBURST) - commandTimers[i][RD]) * 4;
				if (loopCheck > 0 && loopCheck > temp1)
				{
					temp2 = loopCheck;
				}
			}
			
			temp2 = temp2 + currentCPUTick;
			
			if (temp1 < temp2)
				temp1 = temp2;
			
			
			if (tRCD - commandTimers[bank][ACT] > 0)
				temp2 = currentCPUTick + ((tRCD - commandTimers[bank][ACT]) * 4);
			else
				temp2 = currentCPUTick;
				
			if (temp1 < temp2)
				temp1 = temp2;
					
								
			if (tRTW - commandTimers[bank][RD] > 0)
				temp2 = currentCPUTick + ((tRTW - commandTimers[bank][RD]) * 4);
			else
				temp2 = currentCPUTick;
				
								
			if (temp1 > temp2)
				*upper = temp1;
			else
				*upper = temp2;
			
			*lower = *upper - ((tCAS - tCWL + tBURST) * 4);
			return 0;
			
		default :
			printf("calculateWindow error: invalid input command\n");
			return -1;
	
	}

}

int findStarvation()
{
	int i;
	int mostStarvedRequest = -1;  // default to no starving commands 
	unsigned long long starvationTemp = 0, starvationAmount = 0; 
	
	for (i = 0; i < TOTAL_BANKS; ++i)
	{
		if (requestQueue[i].occupied && !requestQueue[i].finished) // &&
			//isCommandLegal(findNextCommand(i), requestQueue[i].bank, requestQueue[i].row, i))
		{
			starvationTemp = currentCPUTick - requestQueue[i].timeIssued;
			
			if (starvationTemp > STARVE_LIMIT 
			    && starvationTemp > starvationAmount)
			{
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

bool isCommandLegal(command cmd, unsigned bank, unsigned row, int index, bool starvedCommand)
{
	bool check = FALSE;
	int i;
	
	if (starvationStatus.isCommandStarving && starvedCommand == FALSE)
		if (bank == starvationStatus.bank)
			return FALSE;
	
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
						return check;
					}
				}
			}
			
			if (starvationStatus.isCommandStarving == TRUE && starvedCommand == FALSE)
			{
				if (starvationStatus.name == ACT)
				{
					if (currentCPUTick >= starvationStatus.lowerWindow &&
						currentCPUTick <= starvationStatus.upperWindow)
					{
						check = FALSE;
						return check;
					}
					else
						check = TRUE;
				}
			}
						
			break;
			
		case RD:
			if (commandTimers[bank][ACT] >= tRCD &&
				commandTimers[bank][WR] >= tWTR)
			{
				for (i = 0; i < 16; ++i)
				{
					if (requestQueue[i].occupied & !requestQueue[i].finished)
					{
						if (!strcmp(requestQueue[i].name, "WRITE") &&
							requestQueue[i].bank == bank &&
							requestQueue[i].row == row &&
							requestQueue[i].column == requestQueue[index].column)	
						{
							if (requestQueue[i].timeIssued < requestQueue[index].timeIssued)
							{	
								check = FALSE;
								return check;
							}
							else
							{
								check = TRUE;
							}
						}
					}
				}
				
				for(i = 0; i < 8; ++i)
				{
					if (commandTimers[i][RD] >= tBURST)
						check = TRUE;
					else
					{
						check = FALSE;
						return check;
					}
				}
			
				if (starvationStatus.isCommandStarving == TRUE && starvedCommand == FALSE)
				{
					if (starvationStatus.name == RD)
					{
						if (currentCPUTick >= starvationStatus.lowerWindow &&
							currentCPUTick <= starvationStatus.upperWindow)
						{
							check = FALSE;
							return check;
						}
						else
							check = TRUE;
					}
					
					else if (starvationStatus.name == WR)
					{
						if (currentCPUTick >= starvationStatus.lowerWindow &&
							currentCPUTick <= starvationStatus.upperWindow)
						{
							check = FALSE;
							return check;
						}
						else
							check = TRUE;
					}
				}
			}
			
			break;
			
		case WR:
			if (commandTimers[bank][ACT] >= tRCD &&
				commandTimers[bank][RD] >= tRTW)
			{
				for (i = 0; i < 16; ++i)
				{
					if (requestQueue[i].occupied & !requestQueue[i].finished)
					{
						if ((!strcmp(requestQueue[i].name, "READ") ||
							!strcmp(requestQueue[i].name, "IFETCH")) &&
							requestQueue[i].bank == bank &&
							requestQueue[i].row == row &&
							requestQueue[i].column == requestQueue[index].column)	
						{
							if (requestQueue[i].timeIssued < requestQueue[index].timeIssued)
							{	
								check = FALSE;
								return check;
							}
							else
							{
								check = TRUE;
							}
						}
					}
				}
				
				for(i = 0; i < 8; ++i)
				{
					if (commandTimers[i][RD] >= tCAS - tCWL + tBURST)
						check = TRUE;
					else
					{
						check = FALSE;
						return check;
					}
				}
				
				for(i = 0; i < 8; ++i)
				{
					if (commandTimers[i][WR] >= tBURST)
						check = TRUE;
					else
					{
						check = FALSE;
						return check;
					}
				}
				
				if (starvationStatus.isCommandStarving == TRUE && starvedCommand == FALSE)
				{
					if (starvationStatus.name == WR)
					{
						if (currentCPUTick >= (starvationStatus.upperWindow - (tBURST * 4)) &&
							currentCPUTick <= starvationStatus.upperWindow)
						{
							check = FALSE;
							return check;
						}
						else
							check = TRUE;
					}
				}	
				
			}
			break;
			
		default:
			printf("Command ERROR!\n");
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
			commandTimers[i][j] = TIMER_LIMIT;	
	
}

void updateTimers(command cmd, unsigned bank)
{
	int i, j;
	
	commandTimers[bank][cmd] = 0;
	
	for (i = 0; i < TOTAL_BANKS; ++i)
		for (j = 0; j < 4; ++j)
			if (commandTimers[i][j] < TIMER_LIMIT)
				commandTimers[i][j] += 1;

}

void incrementTimers(unsigned long n)
{
	int i, j;
	
	if (n > TIMER_LIMIT)
	{
		n = TIMER_LIMIT;
	}
	
	for (i = 0; i < TOTAL_BANKS; ++i)
		for (j = 0; j < 4; ++j)
			if (commandTimers[i][j] < TIMER_LIMIT)
				commandTimers[i][j] += n;
}

/*===============================================================================*/
/* fill an array slot if empty */
bool enqueue(int *countSlotsOccupied, int addMax)
{
	int loopVar;
	int addCount = 0;

	///* When array is full, update the time of inputbuffer */
	//if(*countSlotsOccupied == ARRAY_SIZE-1) /* Array is filled */
	//{
			//inputBuffer.timeEnqueued = currentCPUTick;
	//}
	
	for(loopVar = 0; loopVar < ARRAY_SIZE; ++loopVar)
	{	/* Find an open spot to enqueue */
		
		 if(requestQueue[loopVar].occupied == FALSE)
		{
			addCount += 1;
			
			if (addCount == addMax)
				requestQueue[loopVar].timeIssued = currentCPUTick;
			else
				requestQueue[loopVar].timeIssued = inputBuffer.timeIssued;
							
			/*Open slot found. Copy the contents of the temporary buffer */
		 	strcpy(requestQueue[loopVar].name,inputBuffer.name);
			requestQueue[loopVar].fullAddress 	= inputBuffer.fullAddress;
			requestQueue[loopVar].row		 	= inputBuffer.row;
			requestQueue[loopVar].column 		= inputBuffer.column;
			requestQueue[loopVar].bank			= inputBuffer.bank;
			requestQueue[loopVar].occupied 		= YES;	/* The slot is filled now */
			/* Initialize other struct members */
			requestQueue[loopVar].finished 		= NO;
			requestQueue[loopVar].timeRemaining = NO;
			*countSlotsOccupied  = *countSlotsOccupied +1;	/*Increment counter */
									
			return FALSE;  /* futureRequest= FALSE. Reload the buffer with CPU request next time */
		}
	}
	
	return TRUE;      /* Enqueu operation failed. so futureRequest = TRUE. */
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

			/* If the last CPU issue time was less than the current issue time,
			    there is a error. So, set the time error flag.*/
			/*==================================================================*/
			if(lastIssueTime > inputBuffer.timeIssued && inputBuffer.timeIssued !=0)
			{
				/* CPU issued time is out of order */
				issueTimeErrorFlag = YES;
				printf("ERROR! CPU requests are not sequentially ordered\n");
			}		
				/* Two CPU Requests at the same time error */
			else if (lastIssueTime == inputBuffer.timeIssued && countSlotsOccupied !=0)
			{
				/* CPU issed two requests at the same time */
				issueTimeErrorFlag = YES;
				printf("Error!!! CPU  cannot issue more than 1 request at the same time\n");
			}
			else
				lastIssueTime = inputBuffer.timeIssued;
			/*====================================================================*/
			inputBuffer.timeIssued = (unsigned long long) strtoll(token, NULL, 10);
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
	for(loopVar=0; loopVar < ARRAY_SIZE ; ++loopVar)
	{
		if(requestQueue[loopVar].occupied && requestQueue[loopVar].finished)
		{
			requestQueue[loopVar].timeRemaining = requestQueue[loopVar].timeRemaining - 1;
			
			if(requestQueue[loopVar].timeRemaining == 0)
			{
				requestQueue[loopVar].occupied = FALSE;      /* The slot is open for enqueue */
				*countSlotsOccupied = *countSlotsOccupied - 1;	/* Decreament the queue counter */
			}
		}
	}
}
/*===============================================================================*/
int main(int argc, char **argv)
{
	FILE *ofile;
	FILE *fp;								    	/* File handler */
	bool endOfFile;							   		/* End of file variable */
	bool futureRequest  	 = FALSE;				/* The CPU request is for future time. It also makes "inputBuffer hold-On" call */
	currentCPUTick     	  	 = 0;					/* Start of the simulation */
	int addMax = 0;
	
	int i;
	
	for (i = 0; i < 16; ++i)
	{
		requestQueue[i].occupied = FALSE;
		requestQueue[i].finished = FALSE;
	}
	
	initializeTimers();

	/*================================================================================*/
	/* Open the file */
     if (argc != 3)
     {
      	printf("You must enter a testfile to use, and output file name.\n");
        return -1;
      }
 		
	 if ((fp = fopen(argv[1],"r")) == NULL)
     {
         printf("Could not open file: %s\n", argv[1]);
         return -1;
     }
     
    if((ofile = fopen(argv[2],"w"))== NULL)
    {
		printf("File open not successful. \n");
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
		futureRequest = YES;
	else
		printf("File is empty!\n");
	
	while (!endOfFile || (countSlotsOccupied !=0))
	{
		/* Call a function to see if any request has been completed */
		
		
		addMax = ARRAY_SIZE - countSlotsOccupied + 1;
		examineQueueForCompletion(&countSlotsOccupied);
		
		/*====================================================================*/
		while (!endOfFile && countSlotsOccupied != ARRAY_SIZE && !issueTimeErrorFlag)   
		{
			/* Check this section again */
			/*==============================================*/
			if(futureRequest == YES)
			{
				/* CASE 1: Queue is empty */
				if(countSlotsOccupied == 0)
				{
					if(inputBuffer.timeIssued >currentCPUTick)
					{
						incrementTimers((inputBuffer.timeIssued + (4 - inputBuffer.timeIssued % 4)) - currentCPUTick);
						currentCPUTick = inputBuffer.timeIssued + (4 - inputBuffer.timeIssued % 4);
					}
				}
				
				if(inputBuffer.timeIssued > currentCPUTick)
					break;  		/* It's not yet time to enqueue. So break the loop */
				
				futureRequest = enqueue(&countSlotsOccupied, addMax);	/* Queue in the first open slot found */
			}
			
			/* Load another image from */

			else if(!endOfFile)
				{
					/* Load another request from the file */
					endOfFile=loadInputBuffer(fp);     /* Fill the temporary buffer */
					if(!endOfFile)
						futureRequest = YES;
					
				}
			/*========================================================*/
								
		}

		/* Error in CPU request time sequence or two requests at the same time */
		if(issueTimeErrorFlag)
			break;
		
		
		/* Call a function to see if any request has been completed */
			
	/* Do the DRAM service */

		policyManager(ofile);
		
	/*=========================================================================*/
	/* Service the DRAM */
	
		currentCPUTick += 4;

	}  
	/*========================================================================*/
	
return 0;
}
				
		
	
			
		
