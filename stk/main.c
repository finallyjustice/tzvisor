#include "uart.h"
#include "versatile_pb.h"
#include "types.h"

void kmain(void)
{
	__REG(UART0) = 'A';
	uart_send('W');
	cprintf("This is new world!\n");
	while(1);
}
