#ifndef __TZVISOR_H__
#define __TZVISOR_H__

#include <linux/ioctl.h>

// param for ioctl call (user-->kernel)
struct vm_tzv_file
{
	int file_len;
	char *file_cont;
};

// param for hypercall (vm-->kvm)
struct kvm_tzv_file
{
	int file_len;
	unsigned int file_cont_gpa;
};

// hypercall command
#define TZV_COMM_REGISTRATION  0x11   // register a PAL binary
#define TZV_COMM_UNREGISTER    0x12   // call a PAL function
#define TZV_COMM_PALCALL       0x13   // unregister a PAL

// ioctl call command
#define TZ_IO_MAGIC 0xAF
#define TZV_IOREG   _IOWR(TZ_IO_MAGIC, 0xE1, struct vm_tzv_file)  // register a PAL binary
#define TZV_IOCALL  _IOWR(TZ_IO_MAGIC, 0xE2, int)                 // call a PAL function
#define TZV_IOUNREG _IOWR(TZ_IO_MAGIC, 0xE3, int)                 // unregister a PAL


#endif
