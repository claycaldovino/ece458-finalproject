#include <stdlib.h>
#include <stdio.h>

int main()
{
	char str[30] = "0x11 Baby";
	char *ptr;
	unsigned ret;
	
	ret = (unsigned int) strtol(str, &ptr, 16);
	printf("The number is %u\n", ret);
	printf("String part is |%s|\n", ptr);
	
	return 0;
}
