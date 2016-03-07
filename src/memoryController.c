#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool queueFull;

/* fill an array slot if empty */
bool enqueue(void)
{
	int loopVar;
	/* Check Queue for open slot */
	loopVar = 0;
	
	if(inputBuffer.timeIssued > currentCPUTick)
		return TRUE;			/* Not ready to enqueue yet */
	
	while(loopVar<=ARRAY_SIZE)
	{
		if(loopVar==ARRAY_SIZE) /* Loop variable should be 0-15 */
		{	
			printf("Queue is full now ! \n");
			queueFull = TRUE;	/* Queue is full */
			return TRUE;   /* Queue is full. Do future request for this input buffer.*/
		}
		else if(requestQueue[loopVar].occupied == FALSE)
		{
			 /*The last time queue was full. So, set timeIssued to current CPU tick */
			if(queueFull)
			{
				
				requestQueue[loopVar].timeIssued = currentCPUTick; 
				queueFull = FALSE;
			}
			/* None empty slot found. Copy the contents of the temporary buffer */
		 	strcpy(requestQueue[loopVar].name,inputBuffer.name);
			requestQueue[loopVar].fullAddress = inputBuffer.fullAddress;
			requestQueue[loopVar].row = inputBuffer.row;
			requestQueue[loopVar].column = inputBuffer.column;
			requestQueue[loopVar].bank= inputBuffer.bank;
			requestQueue[loopVar].timeIssued = inputBuffer.timeIssued;
			requestQueue[loopVar].occupied = TRUE;	/* The slot is filled now */
			printf("LoopVar: %d \n",loopVar);
			
			return FALSE;  /* futureRequest= FALSE. Don't request next time */
			
		}
		else
			++loopVar;
	}
	
	
}
/*===================================================================================*/
/* Fill inputBuffer */
bool fillInputBuffer(FILE *fp)
{
		char buf[128];							    /* Temporary buffer */
		char *token;							    /* Token for file read */
		if(fgets(buf,128,fp)==NULL)
		{
			printf("File is empty\n");
			return TRUE;    /* File is empty */
		}
				 
				/* Get next CPU request */
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
/*====================================================================*/
int main(int argc, char **argv)
{
	FILE *fp;								    /* File handler */
	bool endOfFile;							    /* End of file */
	bool queueEmpty		 = TRUE;				/* Queue is empty intially*/
	bool futureRequest   = FALSE;				/* Future request */
	currentCPUTick       = 0;					/* Start of the simulation */
	int loopVar 	     = 0;					/* Temporary loop variable */

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
	/* Initialize the array element as not occupied */
		 
		while(loopVar < ARRAY_SIZE)
		{
			requestQueue[loopVar].occupied 		= FALSE;  		/* All the array elements are unused */
			requestQueue[loopVar].finished 		= FALSE;  		/* Nothing is done */
			requestQueue[loopVar].timeRemaining = FALSE; 		/* No time remaining */
			++loopVar;
		}
	
	/*==========================================================================*/
	/* Keep doing the task, until all done. */
	
	/* Case 1: First time access the file.Fill the inputBuffer */
	endOfFile = fillInputBuffer(fp);
	++currentCPUTick;                     /* One clock tick to add to buffer */
	
	while (!endOfFile || !queueEmpty)
	{
		/*====================================================================*/
		/* Take the requests from the CPU. Enqueue the requests.*/
						
		/* Remove all the requests  that are completed*/
		for(loopVar=0; loopVar <ARRAY_SIZE ; ++loopVar)
		{
			if(requestQueue[loopVar].occupied && requestQueue[loopVar].finished)
			{
					if(requestQueue[loopVar].timeRemaining==FALSE)
						requestQueue[loopVar].occupied =FALSE;      /* The slot is open for enqueue */
					else
						requestQueue[loopVar].timeRemaining = requestQueue[loopVar].timeRemaining -1;
			}
		}
			
		/*--------------------------------------------------------------------*/		
	
		/* Set the Current CPU time to the time of request.*/
		if(queueEmpty)
		{
			if(inputBuffer.timeIssued >currentCPUTick)
				currentCPUTick =inputBuffer.timeIssued;
			futureRequest= TRUE;					/*Input Buffer contains data*/
		}
		
		/*-------------------------------------------------------------------*/
		while ((currentCPUTick %4) != 0)
		{
			++currentCPUTick;
			/*  if there is pending request in the buffer,enqueue it first*/
			/* CASE 2: Pending request to queue in buffer */
			if(futureRequest==TRUE) 
			{	
				futureRequest = enqueue();	/* Fill the first open slot found */
										
			}
			/* CASE 3: Not the end of file */
			else if(!endOfFile)
			{
				
				endOfFile=fillInputBuffer(fp);     /* Fill the temporary buffer */
				
				if(!endOfFile)
					futureRequest = enqueue();	  	 /* Try to enqueue */
				;
			}
			
			
					
								
		}
	
	/* Do the DRAM service */
		break;
		
	/*=========================================================================*/
	/* Service the DRAM */

	}  
	/*========================================================================*/
	
return 0;
}
				
		
	
			
		
