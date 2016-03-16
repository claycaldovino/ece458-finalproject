typedef int bool;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define YES 1
#define NO 0

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
#define tCCD 4
#define tBURST 4
#define tWTR 8
#define tRTW (tCAS + tCCD + 2 - tCWL)

#define ARRAY_SIZE 16
#define TOTAL_BANKS 8

#define STARVE_LIMIT 200

typedef enum {PRE, ACT, RD, WR, WAIT} command;

typedef struct
{
	char name[32];
	unsigned fullAddress;
	unsigned row;
	unsigned bank;
	unsigned column;
	bool occupied;
	bool finished;
	unsigned timeRemaining;	
	unsigned long long timeIssued;
} request;

typedef struct
{
    bool isPrecharged;
	bool isActivated;
	unsigned activeRow;
} bankStatus;

typedef struct
{
	bool isCommandStarving;
	unsigned long long lowerWindow;
	unsigned long long upperWindow;
	command name;
	int bank;
} starveStruct;

request inputBuffer;
request requestQueue[ARRAY_SIZE];
bankStatus dimmStatus[TOTAL_BANKS];
starveStruct starvationStatus; 

unsigned long long currentCPUTick = 0;  /* Current CPU tick */
int countSlotsOccupied	 = 0;

int commandTimers[TOTAL_BANKS][4];
unsigned lastIssueTime;
bool issueTimeErrorFlag;
