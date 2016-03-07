#include "request.h"

bool queueFull;
/*==========================================================================*/
/* Check if a request is serviced. If yes, set it to done*/
void removeFinishedRequest(bool * occupied, const bool finished,unsigned * timeRemaining)
{
	if(*occupied ==TRUE)
	{
		if(finished ==TRUE) 
		{
			timeRemaining = timeRemaining -1;
			if(timeRemaining==FALSE)
			{
				occupied ==FALSE;      /* The slot is open for enqueue */
			}
		}
	}

}
/* fill an array slot if empty */
bool fillOpenSlot(void)
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
			queueFull = TRUE;	/* Queue is full */
			return TRUE;   /* Queue is full. futureRequest = TRUE. Request next time */
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
			
			return FALSE;  /* futureRequest= FALSE. Don't request next time */
			
		}
		else
			++loopVar;
	}
	
	
}

/*====================================================================*/
int main(int argc, char **argv)
{
	FILE *fp;								/* File handler */
	char *filePointer;						/* Temporary file pointer */
	bool endOfFile 		= FALSE;				/* End of file */
	bool queueEmpty = FALSE;					/* Queue is empty */
	bool futureRequest = FALSE;				/* Future request */
	currentCPUTick  = 0;					/* Start of the simulation */
	int loopVar 	= 0;					/* Temporary loop variable */
	char buf[128];							/* Temporary buffer */
	char *token;							/* Token for file read */
	int i;
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
	/* Initialize the array element as clean */
		i = 0;
		while(i < ARRAY_SIZE)
		{
			requestQueue[i].occupied = FALSE;  		/* All the array elements are unused */
			requestQueue[i].finished = FALSE;  		/* Nothing is done */
			requestQueue[i].timeRemaining = FALSE; 	/* No time remaining */
			++i;
		}
	
	/*==========================================================================*/
	/* Keep doing the task, until all done. */
	while (!endOfFile || !queueEmpty)
	{
		/*====================================================================*/
		/* Take the requests from the CPU. Enqueue the requests.*/
						
		/* Remove all the Requests  that are completed*/
		for(loopVar; loopVar <ARRAY_SIZE; ++loopVar)
			removeFinishedRequest(&requestQueue[loopVar].occupied,requestQueue[loopVar].finished,&requestQueue[loopVar].timeRemaining);
	
			
		/*  if there is pending request in the buffer,enqueue it first*/
		/* CASE 1: Pending request to queue in buffer */	
		do
		{	
		if(futureRequest==TRUE) 
			futureRequest = fillOpenSlot();	/* Fill the first open slot found */
				
		/* CASE 2:Nothing in the pending buffer, and it is not endOfFile */		
		else if(!endOfFile)
		{		/* Get the next request */
				filePointer = fgets(buf,128,fp);  /* Fill the buffer */
			
				if(filePointer==NULL)
					endOfFile = TRUE;    /* File is empty */
 			
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
			/*--------------------------------------------------------*/
					futureRequest = fillOpenSlot();		/* Call Function to fill the array */
			    } 
		}
		}(while inputBuffer.timeIssued <=currentCPUTick);
		
		/*CASE 3: Queue is not empty, and it is end of file */
	}  
	/*========================================================================*/
	
return 0;
}
				
		
	
			
		