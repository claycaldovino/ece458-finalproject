#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef int bool;
#define TRUE 1
#define FALSE 0

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

#define ARRAY_SIZE 16
#define TOTAL_BANKS 8



typedef struct
{
	char name[32];
	unsigned fullAddress;
	unsigned row;
	unsigned bank;
	unsigned column;
	unsigned long long timeIssued;
	bool occupied;
	bool finished;
	unsigned timeRemaining;	
} request;

request inputBuffer;
request requestQueue[ARRAY_SIZE]; 

unsigned long long currentCPUTick;  /* Current CPU tick */
