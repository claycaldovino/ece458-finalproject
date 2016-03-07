#include "request.h"

/*==========================================================================*/
/* Check if a request is serviced. If yes, set it to done*/
void removeFinishedRequest(bool * occupied, const bool finished, const unsigned timeRemaining)
{
	if(occupied ==TRUE)
	{
		if(finished ==TRUE) 
		{
			if(timeRemaining==FALSE)
			{
				occupied ==FALSE;      /* The slot is open for enqueue */
			}
		}
	}

}

/*====================================================================*/
int main(int argc, char **argv)
{
	FILE *fp;								/* File handler */
	//request inputBuffer;					/* Temporary Buffer */
	//request requestQueue[ARRAY_SIZE]; 		/* Queue */
	bool EOF 		= FALSE;						/* End of file */
	bool queueEmpty = FALSE;				/* Queue is empty */
	currentCPUTick  = 0;					/* Start of the simulation */
	int loopVar 	= 0;					/* Temporary loop variable */
	char buf[128];							/* Temporary buffer */
	char *token;							/* Token for file read */
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
		
	while (!EOF && !queueEmpty)
	{
		/* Remove all the serviced Requests */
		for(loopVar; loopVar <ARRAY_SIZE; ++loopVar)
			removeFinishedRequest(requestQueue[loopVar].occupied,requestQueue[loopVar].finished,requestQueue[loopVar].timeRemaining)
		
		/* Fill the temporary inputBuffer with a CPU request */
		while (fgets(buf,128,fp)!=NULL)   
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
			
			
			
		