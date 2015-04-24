//
// Board specific information for the VersatilePB board
//
#ifndef VERSATILEPB
#define VERSATILEPB

#include "types.h"

// the VerstatilePB board can support up to 256MB memory.
// but we assume it has 128MB instead. During boot, the lower
// 64MB memory is mapped to the flash, needs to be remapped
// the the SDRAM. We skip this for QEMU
#define PHYSTOP         (0x08000000 + PHY_START)
#define BSP_MEMREMAP    0x04000000

#define DEVBASE1        0x1c000000
#define DEVBASE2        0x2c000000
#define DEV_MEM_SZ      0x01000000
#define VEC_TBL         0xFFFF0000


#define STACK_FILL      0xdeadbeef

#define UART0           0x1c090000
#define UART_CLK        24000000    // Clock rate for UART

#define TIMER0          0x1c110000
#define TIMER1          0x1c120000
#define CLK_HZ          1000000     // the clock is 1MHZ

// HCLIN reference vexpress.c:
// sysbus_create_simple("pl011", map[VE_UART0], pic[5]);
//
// a GIC is registered in vexpress.c
//     * 0x2c000000 A15MPCore private memory region (GIC) *
//    init_cpus(cpu_model, "a15mpcore_priv", 0x2c000000, pic);
// in init_cpus, the GIC accept 64 gpio input and one output to ARM_IRQ
// 
// 			dev // GIC device
//			busdev = SYS_BUS_DEVICE(dev);
//		    for (n = 0; n < 64; n++) {
//              pic[n] = qdev_get_gpio_in(dev, n);
//    		}
//			/* Connect the CPUs to the GIC */
//    		for (n = 0; n < smp_cpus; n++) {
//        		DeviceState *cpudev = DEVICE(qemu_get_cpu(n));
//        		sysbus_connect_irq(busdev, n, qdev_get_gpio_in(cpudev, ARM_CPU_IRQ));
//    		}
#define VIC_BASE        0x2c000000
#define PIC_TIMER01     2
#define PIC_TIMER23     3
#define PIC_UART0       5
#define PIC_GRAPHIC     19

#define GICD_CTLR       0x000
#define GICD_TYPER      0x004
#define GICD_IIDR       0x008

#define GICD_IGROUP     0x080
#define GICD_ISENABLE       0x100
#define GICD_ICENABLE       0x180
#define GICD_ISPEND     0x200
#define GICD_ICPEND     0x280
#define GICD_ISACTIVE       0x300
#define GICD_ICACTIVE       0x380
#define GICD_IPRIORITY      0x400
#define GICD_ITARGET        0x800
#define GICD_ICFG       0xC00

#define GICC_CTLR       0x000
#define GICC_PMR        0x004
#define GICC_BPR        0x008
#define GICC_IAR        0x00C
#define GICC_EOIR       0x010
#define GICC_RRR        0x014
#define GICC_HPPIR      0x018

#define GICC_ABPR       0x01C
#define GICC_AIAR       0x020
#define GICC_AEOIR      0x024
#define GICC_AHPPIR     0x028

#define GICC_APR        0x0D0
#define GICC_NSAPR      0x0E0
#define GICC_IIDR       0x0FC
#define GICC_DIR        0x1000

static volatile uint* gic_base;

#define GICD_REG(o)     (*(uint *)(((uint) gic_base) + 0x1000 + o))
#define GICC_REG(o)     (*(uint *)(((uint) gic_base) + 0x2000 + o))

// A SP804 has two timers, we only use the first one, and as perodic timer

// define registers (in units of 4-bytes)
#define TIMER_LOAD     0    // load register, for perodic timer
#define TIMER_CURVAL   1    // current value of the counter
#define TIMER_CONTROL  2    // control register
#define TIMER_INTCLR   3    // clear (ack) the interrupt (any write clear it)
#define TIMER_MIS      5    // masked interrupt status

// control register bit definitions
#define TIMER_ONESHOT  0x01 // wrap or one shot
#define TIMER_32BIT    0x02 // 16-bit/32-bit counter
#define TIMER_INTEN    0x20 // enable/disable interrupt
#define TIMER_PERIODIC 0x40 // enable periodic mode
#define TIMER_EN       0x80 // enable the timer

#define SGI_TYPE        1
#define PPI_TYPE        2
#define SPI_TYPE        3

#endif
