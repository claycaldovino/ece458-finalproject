#include "request.h"


/*===============================================================================*/
/* fill an array slot if empty */
bool enqueue(int *countSlotsOccupied)
{
	int loopVar;
	for(loopVar = 0; loopVar <ARRAY_SIZE; ++loopVar)
	{	/* Find an open spot to enqueue */
		
		 if(requestQueue[loopVar].occupied == NO)
		{
			/*Open slot found. Copy the contents of the temporary buffer */
		 	strcpy(requestQueue[loopVar].name,inputBuffer.name);
			requestQueue[loopVar].fullAddress 	= inputBuffer.fullAddress;
			requestQueue[loopVar].row		 	= inputBuffer.row;
			requestQueue[loopVar].column 		= inputBuffer.column;
			requestQueue[loopVar].bank			= inputBuffer.bank;
			requestQueue[loopVar].timeIssued 	= inputBuffer.timeIssued;
			requestQueue[loopVar].timeEnqueued 	= inputBuffer.timeEnqueued;
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
			return TRUE;   							/* File is empty */
				 
		/* Load the next CPU request */
		else   
		{
			/* Fill the temporary buffer with the content of CPU */
			token = strtok(buf,"\t");
			inputBuffer.fullAddress = (unsigned int) strtol(token,NULL,16);
			/*Split the address into row, bank and column */
			inputBuffer.row = (inputBuffer.fullAddress & 0xFFFE0000)>>17;
			inputBuffer.bank = (inputBuffer.fullAddress & 0x1C000)>>14 ;
			inputBuffer.column = (inputBuffer.fullAddress & 0x3FF8)>>3;
			/* Fill the request name*/
			token = strtok(NULL,"\t");
			strcpy(inputBuffer.name,token);
			/* Fill the CPU request time */
			token = strtok(NULL,"\n");
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
					if(requestQueue[loopVar].timeRemaining==FALSE)
					{
						requestQueue[loopVar].occupied =FALSE;      /* The slot is open for enqueue */
						countSlotsOccupied = countSlotsOccupied -1;	/* Decreament the queue counter */
					}
					else
						requestQueue[loopVar].timeRemaining = requestQueue[loopVar].timeRemaining -1;
			}
		}
}
/*====================================================================*/
int main(int argc, char **argv)
{
	FILE *fp;								    	/* File handler */
	bool endOfFile;							   		/* End of file variable */
	int countSlotsOccupied	 = 0;					/* Counts the number of requests on the queue*/
	bool futureRequest  	 = FALSE;				/* The CPU request is for future time. It also makes "inputBuffer hold-On" call */
	currentCPUTick     	  	 = 0;					/* Start of the simulation */
	int loopVar 	         = 0;					/* Temporary loop variable */
	

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

	
	while (!endOfFile || (countSlotsOccupied !=0))
	{
		/*====================================================================*/
		while (!endOfFile && countSlotsOccupied !=(ARRAY_SIZE-1))   
		{
			
			/* Check this section again */
			/*==============================================*/
			if(futureRequest == YES)
			{
				/* CASE 1: Queue is empty */
				if(countSlotsOccupied == 0)
				{
					if(inputBuffer.timeIssued >currentCPUTick)
						currentCPUTick =inputBuffer.timeIssued;
				}
				
				if(inputBuffer.timeIssued > currentCPUTick)
					break;  		/* It's not yet time to enqueue. So break the loop */
				
				futureRequest = enqueue(&countSlotsOccupied);	/* Queue in the first open slot found */
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
		
		/* Call a function to see if any request has been completed */
		
		examineQueueForCompletion(&countSlotsOccupied);
	
	/* Do the DRAM service */

		
	/*=========================================================================*/
	/* Service the DRAM */

	}  
	/*========================================================================*/
	
return 0;
}
				
		
	
			
		
