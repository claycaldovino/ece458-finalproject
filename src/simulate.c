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
	int dirty = 0; /*Initially each array is clean */
}Queue;
	
/*========================================================================================*/
int main(int argc, char **argv)
{
		/*===============================================================================*/
		/* Declare the variables*/
		/* clock_t gives the CPU clock ticks since the start of the process */
		global_tick = clock();   	/* Start of the clock cycle*/
		Queue array[ARRAY_SIZE]  			/*Arrays of Structs */
		temporary temp_buf;			/* This is a temporary buffer to hold next command to enqueue */
		FILE * fp;					/* File handler */
        char buf[128];				/* Temporary buffer */
        char commandFile[64];
        char *token;
       	double cpu_ticks_used;
        int i;						/* Loop variable */
	
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
			
				if( head==tail && !array[tail].dirty)   
				{					               
	         		array[tail].full_address = temp_buf.Address;
					/*Split the address into row, bank and column */
					array[tail].row = (temp_buf.Address & 0xFFFE0000)>>17;
					array[tail].bank = (temp_buf.Address & 0x1C000)>>14 ;
					array[tail].column = (temp_buf.Address & 0x3FF8)>>3;
	                strcpy(array[tail].Operation,temp_buf.CPU_OP);
                    array[tail].time_issued = temp_buf.CPU_clock_cycle ;   /* Copy the time issued */
				
					array[tail].dirty = 1;   /*The slot is filled */
					
					++tail;					/* Update tail pointer */
				}
			
				/*CASE 2: Queue is non empty and not completely filled*/
				else if( head != tail && !array[tail].dirty)   
				{					               
	         		array[tail].full_address = temp_buf.Address;
					/*Split the address into row, bank and column */
					array[tail].row = (temp_buf.Address & 0xFFFE0000)>>17;
					array[tail].bank = (temp_buf.Address & 0x1C000)>>14 ;
					array[tail].column = (temp_buf.Address & 0x3FF8)>>3;
	                strcpy(array[tail].Operation,temp_buf.CPU_OP);
                    array[tail].time_issued = temp_buf.CPU_clock_cycle ;   /* Copy the time issued */
				
					array[tail].dirty = 1;   /*The slot is filled */
					
					++tail;					/* Update tail pointer */
				}
			
				/* CASE 3: Queue is filled */
				else if (head == tail && array[tail].dirty)
				{
					printf("WARNING!! QUEUE is filled.. WAIT!! \n");
					break;
				}
					             
		}

        fclose(fp);

        for (i=0; i<=tail; i++) 
        {
                printf("----------------------------------\n");
                printf("Slot             :%d\n",i);
                printf("Operation        : %s\n", array[i].Operation);
                printf("Full_Address     : 0x%x\n",array[i].full_address);
			    printf("Row              : 0x%x\n",array[i].row);
				printf("Bank             : 0x%x\n",array[i].bank);
				printf("Column           : 0x%x\n",array[i].column);
                printf("Time_Issued      : %lu\n",array[i].time_issued);
                printf("-----------------------------------\n");
        }
	
		//rrent_tick = clock();
		//u_ticks_used = (double)(current_tick-global_tick);
		printf("Total clock ticks used: %f \n",(double)(clock())/CLOCKS_PER_SEC);
        return 0;

}