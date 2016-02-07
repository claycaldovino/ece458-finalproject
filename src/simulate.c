#include <stdio.h>
#include <string.h>

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

int main(int argc, char **argv)
{
 
        typedef struct
        {
        char name[32];
        char address[32];
        int age;
        } instruction;

        instruction commandQueue[15];
        FILE * fp;

        char buf[128];
        char commandFile[64];
        char *token;
        int openSlot = 0;
        int i;

        if (argc != 2)
        {
                printf("You must enter a testfile to use, 'simulate testfile.txt'\n");
                return 1;
        }

        if ((fp = fopen(argv[1],"r")) == NULL)
        {
                printf("Could not open file: %s\n", argv[1]);
                return 1;
        }
        

        while (fgets(buf,128,fp))
        {

                token = strtok(buf,"\t");
                strcpy(commandQueue[openSlot].address,token);

                token = strtok(NULL,"\t");
                strcpy(commandQueue[openSlot].name,token);

                token = strtok(NULL,"\n");
                commandQueue[openSlot].age = atoi(token);
                openSlot++;
        }

        fclose(fp);

        for (i=0; i<openSlot; i++) 
        {
                printf("slot #%d\n",i);
                printf("name: %s\n",commandQueue[i].name);
                printf("address: %s\n",commandQueue[i].address);
                printf("age: %d\n",commandQueue[i].age);
        }
        return 0;

}