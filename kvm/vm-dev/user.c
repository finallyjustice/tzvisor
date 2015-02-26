#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sched.h>

int main(int argc, char **argv)
{
	int fp;
	fp = open("/dev/temp", O_RDWR, S_IRUSR);
	if(fp < 0)
	{
		printf("Failed to open device /dev/temp!\n");
		return -1;
	}

	char buf[1024];

	while(1)
	{
		read(fp, buf, 0);
		//sched_yield();
		//sleep(1);
		//usleep(100);
	}

	close(fp);

	return 0;
}
