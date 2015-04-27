#include <stdio.h>

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("usage error\n");
		return 1;
	}

	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL)
	{
		printf("Failed to open the file\n");
		return 0;
	}

	unsigned int tmp;

	while(fread(&tmp, sizeof(unsigned int), 1, fp) > 0)
	{
		printf("0x%x\n", tmp);
	}

	fclose(fp);

	return 0;
}
