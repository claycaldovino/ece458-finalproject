#include <stdio.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

// DRAM timing constraints

#define tRC 50
#define tRAS 36
#define tRRD 6
#define tRP 14
#define tRFC 172
#define tCWL 10 // (tCWD)
#define tCAS 14 // (CL)
#define tRCD 14
#define tWR 16
#define tRTP 8
#define tBURST 4
#define tWTR 8

#define ARRAY_SIZE 16
clock_t global_tick; /*CPU clock tick since main got executed*/

/* Struct to hold the CPU command temporarily */
typedef struct
{
	unsigned int Address;
	char CPU_OP[32];
	unsigned long int CPU_clock_cycle;
} temporary;

/* Struct to enqueue cpu command */
typedef struct
{		
	char Operation[32];
    unsigned int full_address;
    unsigned int row;
	unsigned int bank;
	unsigned int column;
    unsigned long int time_issued;
} instruction;

/* Struct to include in each queue-- it contains instruction and extra variables*/
typedef struct
{
	instruction to_add;
	int dirty; /*Initially each array is clean */
} Queue;
	
/*========================================================================================*/
int main(int argc, char **argv)
{
		/*===============================================================================*/
		/* Declare the variables*/
		/* clock_t gives the CPU clock ticks since the start of the process */
		global_tick = clock();   	/* Start of the clock cycle*/
		Queue array[ARRAY_SIZE];  	/*Arrays of Structs */
		temporary temp_buf;			/* This is a temporary buffer to hold next command to enqueue */
		FILE * fp;					/* File handler */
        char buf[128];				/* Temporary buffer */
        char commandFile[64];
        char *token;
       	double cpu_ticks_used;
        int i;						/* Loop variable */
		int total_queue = 0;
	
		int head = 0;				/*Head index : The next to execute */
		int tail = 0;				/*Tail index : This also refers to open slot*/

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
	
		/* Initialize the array element as clean */
		i = 0;
		while(i < ARRAY_SIZE)
		{
			array[i].dirty = 0;  /* All the array elements are unused */
			++i;
		}
		
        /*==================================================================================*/
	    while (fgets(buf,128,fp)!=NULL)
        {	
		
				/*-------------------------------------------------------*/
				/* Fill the temporary buffer with the content of CPU */
				token = strtok(buf,"\t");
				temp_buf.Address = (unsigned int) strtol(token,NULL,16);
				token = strtok(NULL,"\t");
				strcpy(temp_buf.CPU_OP,token);
				token = strtok(NULL,"\n");
				temp_buf.CPU_clock_cycle = atoi(token);
				/*--------------------------------------------------------*/
				/* Enqueue the item*/
				/* CASE 1: The Queue is empty */
			
				tail = tail%ARRAY_SIZE;   			/*when tail = 16, next slot is 0 */
			
				if(!array[tail].dirty )   
				{	
					array[tail].to_add.full_address = temp_buf.Address;
					/*Split the address into row, bank and column */
					array[tail].to_add.row = (temp_buf.Address & 0xFFFE0000)>>17;
					array[tail].to_add.bank = (temp_buf.Address & 0x1C000)>>14 ;
					array[tail].to_add.column = (temp_buf.Address & 0x3FF8)>>3;
	                strcpy(array[tail].to_add.Operation,temp_buf.CPU_OP);
                    array[tail].to_add.time_issued = temp_buf.CPU_clock_cycle ;   /* Copy the time issued */
				
					array[tail].dirty = 1;   /*The slot is filled */
								/* Update tail pointer */
					
					/* CASE 1: Queue is not full */
					if (head==tail )
					{
						printf("Queue is Empty.  %d remaining slots\n",ARRAY_SIZE - total_queue-1);
						if((clock()-global_tick)<temp_buf.CPU_clock_cycle)
						{	printf("Speeding the clock\n");
							global_tick = array[tail].to_add.time_issued;    /* Speed up the clock*/
						}
					}				
					
					/*CASE 2: Queue is non-empty and not completely filled*/ 
					else
						printf("Queue is Not full. %d remaining slots.\n",ARRAY_SIZE - total_queue-1);
					
					++tail;	  /* Update head */
								  
				}
												
				/* CASE 3: Queue is filled */
				else
				{
					printf("WARNING!! QUEUE is full. WAIT!! \n");
					break;
				}
				
					++total_queue;   /* Update the queue length. */
		}

	
	
	
        fclose(fp);
/*
        for (i=0; i<total_queue; i++) 
        {
                printf("----------------------------------\n");
                printf("Slot             :%d\n",i);
                printf("Operation        : %s\n", array[i].to_add.Operation);
                printf("Full_Address     : 0x%x\n",array[i].to_add.full_address);
			    printf("Row              : 0x%x\n",array[i].to_add.row);
				printf("Bank             : 0x%x\n",array[i].to_add.bank);
				printf("Column           : 0x%x\n",array[i].to_add.column);
                printf("Time_Issued      : %lu\n",array[i].to_add.time_issued);
                printf("-----------------------------------\n");
        }
*/	
		//rrent_tick = clock();
		//u_ticks_used = (double)(current_tick-global_tick);
		printf("Total clock ticks used: %f \n",(double)(clock())/CLOCKS_PER_SEC);
        return 0;

}