#include "user.h"

int main(int argc, char **argv)
{
	sys_test8();
	
	sys_disp('A');
	sys_disp('B');
	sys_disp('C');
	sys_disp('\n');
	sys_disp('\r');

	sys_test8();
	
	sys_test9("hello test");
	//cprintf("This is number!: %d\n", 1000);
	sys_test7(100);
	sys_exit();
	return 0;
}
