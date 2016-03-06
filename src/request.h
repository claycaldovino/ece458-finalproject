

typedef int bool;
#define TRUE 1
#define FALSE 0



typedef struct
{
	char name[32];
	unsigned fullAddress;
	unsigned row;
	unsigned bank;
	unsigned column;
	unsigned long timeIssued;
	bool occupied;
	bool finished;
	unsigned timeRemaining;	
} request;

request inputBuffer;
request requestQueue[16]; 
