#include <stdio.h>

int main(int argc, char **argv)
{
	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL)
	{
		printf("Failed to open the file\n");
		return 1;
	}

	// read the number of pal funcs
	int tot;
	fseek(fp, -4, SEEK_END);
	fread(&tot, sizeof(int), 1, fp);
	printf("TOT: %d\n", tot);

	// read each pal func
	unsigned int tmp;
	int i;
	fseek(fp, -(sizeof(unsigned int)*tot+sizeof(int)), SEEK_END);
	for(i=0; i<tot; i++)
	{
		fread(&tmp, sizeof(unsigned int), 1, fp);
		printf("0x%08x\n", tmp);
	}

	fclose(fp);

	return 0;
}
