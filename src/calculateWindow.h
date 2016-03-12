"request.h"

int calculateWindow(const command cmd, const unsigned bank, unsigned long long &lower, unsigned long long &upper)
{
	int i;
	unsigned minTimer = ~0;
	
	switch (cmd)
	{
		case PRE:
			return 0;
			
		case ACT:		
			upper = currentCPUTick + max(0, tRP - commandTimers[bank][cmd]);
			lower = upper - tRRD;
			return 0;
		
		case RD :			
			return 0;
		
		case WR :
			upper = currentCPUTick + max(0, tRCD - commandTimers[bank][ACT]);
			upper = max(upper, currentCPUTick + max(0, tRTW - commandTimers[bank][RD]));
			lower = upper - (tCAS - tCWL + tBURST);
			return 0;
			
		default :
			printf("calculateWindow error: invalid input command\n";
			return -1;
	
	}

}
