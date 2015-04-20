#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "tzvisor.h"

#define PAGE_SIZE 4096

int main(int argc, char **argv)
{
	// e.g., ./user pal
	if(argc < 2)
	{
		printf("Usage: ./user file_path\n");
		return 0;
	}

	char *path = argv[1];
	struct stat st;
	stat(path, &st);
	int fsize =  st.st_size;  // size of file

	int fd_sec = 0;
	// alloc buffer to read PAL ELF file
	char *buf = malloc(fsize);

	fd_sec = open(path, O_RDONLY);
	if(fd_sec < 0)
	{
		printf("Failed to open the file!\n");
		return -1;
	}

	char *tmp = buf;
	int len;
	while(len=read(fd_sec, tmp, PAGE_SIZE) > 0)
	{
		tmp += len;
	}

	close(fd_sec);

	int fd = 0;

	fd = open("/dev/tzvisor-vm", O_RDWR);
	if(fd < 0)
	{
		printf("Failed to open the device\n");
		return -1;
	}

	printf("user data: %s\n", buf);

	// communicate with tzvisor driver
	struct vm_tzv_file tfi;
	tfi.file_len = fsize; 
	tfi.file_cont = buf;
	if(ioctl(fd, TZV_IOREG, &tfi) < 0)
	{
		printf("Failed to use ioctl\n");
		return -1;
	}
	
	free(buf);
	close(fd);

	return 0;
}
