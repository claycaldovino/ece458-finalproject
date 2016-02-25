// Team TTSC
// ECE 485

// Structs!

// Command Struct

int CPU_Time;

struct Command {

	// Address Info
	int Full_Adress;
	int Row;
	int Bank;
	int Column;
	
	char Name[32];
	int Time_Issued;
	
	int Full;
};

struct DIMM_Status{
	
	int Last_Row;
	int Last_Bank;
	int Last_Column;
	
	char Last_Command[32];	
};

