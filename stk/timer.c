#include "versatile_pb.h"
#include "types.h"
#include "memlayout.h"
#include "uart.h"
#include "mmu.h"
#include "proc.h"

int gic_getack()
{
	return GICC_REG(GICC_IAR);
}

static int spi2id(int spi)
{
	return spi+32;
}

void gic_eoi(int intn)
{
	GICC_REG(GICC_EOIR) = spi2id(intn);
}

void timer_irq_handler()
{
	int intid, intn;
	volatile uint * timer0 = (uint *)TIMER0;
	intid = gic_getack(); /* iack */
	intn = intid - 32;
	cprintf("IRQ Number : %d\n", intn);
	timer0[TIMER_INTCLR] = 1;
	gic_eoi(intn);
	sti();
}

static void gicd_set_bit(int base, int id, int bval) 
{
	int offset = id/32;
	int bitpos = id%32;
	uint rval = GICD_REG(base+4*offset);
	if(bval)
		rval |= 1 << bitpos;
	else
		rval &= ~(1<< bitpos);
	GICD_REG(base+ 4*offset) = rval;
}

static void gd_spi_setcfg(int spi, int is_edge)
{
	int id=spi2id(spi);
	int offset = id/16;
	int bitpos = (id%16)*2;
	uint rval = GICD_REG(GICD_ICFG+4*offset);
	uint vmask=0x03;
	rval &= ~(vmask << bitpos);
	if (is_edge)
		rval |= 0x02 << bitpos;
	GICD_REG(GICD_ICFG+ 4*offset) = rval;
}

static void gd_spi_enable(int spi)
{
	int id = spi2id(spi);
	gicd_set_bit(GICD_ISENABLE, id, 1);
}

static void gd_spi_group0(int spi)
{
	return;
}

static void gd_spi_target0(int spi)
{
	int id=spi2id(spi);
	int offset = id/4;
	int bitpos = (id%4)*8;
	uint rval = GICD_REG(GICD_ITARGET+4*offset);
	unsigned char tcpu=0x01;
	rval |= tcpu << bitpos;
	GICD_REG(GICD_ITARGET+ 4*offset) = rval;
}

static void gic_dist_configure(int itype, int num)
{
	int spi= num;
	gd_spi_setcfg(spi, 1);
	gd_spi_enable(spi);
	gd_spi_group0(spi);
	gd_spi_target0(spi);
}

static void gic_configure(int itype, int num)
{
	gic_dist_configure(itype, num);
}

static void gic_enable()
{
	GICD_REG(GICD_CTLR) |= 1;
	GICC_REG(GICC_CTLR) |= 1;
}

void gic_init(uint* base)
{
	gic_base = base;
	/* Provides an intrerupt priority filter. Only interrupts with higher
	 *priority than the value in this register are signaled to the processor.
	 */
	GICC_REG(GICC_PMR) = 0x80;

	gic_configure(SPI_TYPE, PIC_TIMER01);
	gic_enable();
}

void timer_init(int hz)
{   
	volatile uint * timer0 = P2V_DEV(TIMER0);

	timer0[TIMER_LOAD] = CLK_HZ / hz;
	timer0[TIMER_CONTROL] = TIMER_EN|TIMER_PERIODIC|TIMER_32BIT|TIMER_INTEN;
}

#define GIC_DIST_BASE 0x2c001000
#define GIC_DIST_SEC  0x80

void set_timer_secure()
{
	uint *base = P2V_DEV(GIC_DIST_BASE+GIC_DIST_SEC);
	base = base + 1;  // timer irq number is 34, on the second secure register
	*base = 0;
}

void set_timer_normal()
{
	uint *base = P2V_DEV(GIC_DIST_BASE+GIC_DIST_SEC);
	base = base + 1;  // timer irq number is 34, on the second secure register
	*base = 0xffffffff;
}
