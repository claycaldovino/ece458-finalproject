// Policy Stuff

typedef enum {PRE, ACT, RD, WR} command;

int prechargePriority(int queueIndex);
int readPriority(int queueIndex);
int writePriority(int queueIndex);

void policyManager()
{	
	command nextCommand;
	int lastPriority = -3;
	int comparePriority = 0;
	int chosenIndex;
	
	for (int queueIndex = 0; queueIndex < 16; ++queueIndex)
	{
		if (requestQueue[queueIndex].occupied & 
			!requestQueue[queueIndex].finished)
		{
			nextCommand = findNextCommand(queueIndex);
			switch(nextCommand)
			{
				case PRE :
					comparePriority = prechargePriority(queueIndex);
					break;
					
				case ACT :
					comparePriority = 2;
					break;
				
				case RD :
					comparePriority = readPriority(queueIndex);
					break;
					
				case WR :
					comparePriority = writePriority(queueIndex);
					break;
					
				default :
				printf("\nThe FUCK Todd!?!?\n");
			}
			
			if (comparePriority > lastPriority)
			{
				lastPriority = comparePriority;
				chosenIndex = queueIndex;
			}
			else if (comparePriority == lastPriority)
			{
				if (requestQueue[queueIndex].timeIssued > 
					requestQueue[chosenIndex].timeIssued)
				{
					chosenIndex = queueIndex;
				}
			}
		
		}
	}
	
	
}

int prechargePriority(int queueIndex)
{
	int bank = requestQueue[queueIndex].bank;
	unsigned row = dimmStatus[bank].activeRow;
	priority = 3;
	
	for (int i = 0; i < 16; ++i)
	{
		if (requestQueue[i].occupied & !requestQueue[i].finished)
		{
			if (requestQueue[i].bank = bank & requestQueue[i] = row)
			{
				priority 0;
				return priority;
			}
			else
			{
				priority = 3;
			}
		}
	}
	
	return priority;
}

int readPriority(int queueIndex)
{
	int bank = requestQueue[queueIndex].bank;
	int row = requestQueue[queueIndex].row;
	int col = requestQueue[queueIndex].column;
	int timestamp = requestQueue{queueIndex].timeIssued;
	int priority = 4;
	
	for (int i = 0; i < 16; ++i)
	{
		if (requestQueue[i].occupied & !requestQueue[i].finished)
		{
			if (requestQueue[i].name == "WRITE" &
				requestQueue[i].bank == bank &
				requestQueue[i].row == row &
				requestQueue[i].column == col)	
			{
				if (requestQueue[i].timeIssued < timestamp)
				{
					priority = -2;
					return priority;
				}
				else
				{
					priority = 4;
				}
			}
		}
	}
	
	return priority;
}

int writePriority(int queueIndex)
{
	int bank = requestQueue[queueIndex].bank;
	int row = requestQueue[queueIndex].row;
	int col = requestQueue[queueIndex].column;
	int timestamp = requestQueue{queueIndex].timeIssued;
	int priority = 4;
	
	for (int i = 0; i < 16; ++i)
	{
		if (requestQueue[i].occupied & !requestQueue[i].finished)
		{
			if (requestQueue[i].name == "READ" &
				requestQueue[i].bank == bank &
				requestQueue[i].row == row &
				requestQueue[i].column == col)	
			{
				if (requestQueue[i].timeIssued < timestamp)
				{
					priority = -2;
					return priority;
				}
				else
				{
					priority = 4;
				}
			}
		}
	}
	
	return priority;
}
