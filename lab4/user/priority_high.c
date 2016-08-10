#include <inc/x86.h>
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	uint32_t i=0;
	for(i=0;i<3;i++)
	cprintf("[%8x] Priority High is Running!\n",sys_getenvid());
	return;
}
