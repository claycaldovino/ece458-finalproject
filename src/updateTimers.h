void initializeTimers()
{
	int i,j;
	
	for (i = 0; i < TOTAL_BANKS; ++i)
		for (j = 0; j < 4; ++j)
			commandTimers[i][j] = 200;	
	
}

void updateTimers(command cmd, unsigned bank)
{
	int i, j;
	
	commandTimers[bank][cmd] = 0;
	
	for (i = 0; i < TOTAL_BANKS; ++i)
		for (j = 0; j < 4; ++j)
			if (commandTimers[i][j] <= 200)
				commandTimers[i][j] += 1;

}
