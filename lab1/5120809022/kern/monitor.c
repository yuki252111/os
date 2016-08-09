// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line

int time_kerninfo(int argc, char **argv, struct Trapframe *tf);
int time_help(int argc, char **argv, struct Trapframe *tf);
int time_backtrace(int argc, char **argv, struct Trapframe *tf);

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace","stack backtrace",mon_backtrace },
	{ "time help","Display this list of commands",time_help },
	{ "time kerninfo","Display information about the kernel",time_kerninfo },
	{ "time backtrace","stack backtrace",time_backtrace }
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

unsigned read_eip();

/***** Implementations of basic kernel monitor commands *****/
int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		(end-entry+1023)/1024);
	return 0;
}

// Lab1 only
// read the pointer to the retaddr on the stack
static uint32_t
read_pretaddr() {
    uint32_t pretaddr;
    __asm __volatile("leal 4(%%ebp), %0" : "=r" (pretaddr)); 
    return pretaddr;
}

void
do_overflow(void)
{
    cprintf("Overflow success\n");
}

void
start_overflow(void)
{
	// You should use a techique similar to buffer overflow
	// to invoke the do_overflow function and
	// the procedure must return normally.

    // And you must use the "cprintf" function with %n specifier
    // you augmented in the "Exercise 9" to do this job.

    // hint: You can use the read_pretaddr function to retrieve 
    //       the pointer to the function call return address;

    char str[256] = {};
    int nstr = 0;

    for(nstr;nstr<256;nstr++)
	str[nstr]='0';
    
    unsigned char* old_pos=(unsigned char*)read_pretaddr();
    unsigned char* new_pos=old_pos+4;
    uint32_t old_val=*(uint32_t*)old_pos;
    uint32_t new_val=(uint32_t)do_overflow;
    //*new_pos=*old_pos;
    //*old_pos=(uint32_t)do_overflow;
    
    cprintf("%.*s%n",(old_val>>0)&0xff,str,new_pos);
    cprintf("%.*s%n",(old_val>>8)&0xff,str,new_pos+1);
    cprintf("%.*s%n",(old_val>>16)&0xff,str,new_pos+2);
    cprintf("%.*s%n",(old_val>>24)&0xff,str,new_pos+3);

    cprintf("%.*s%n",(new_val>>0)&0xff,str,old_pos);
    cprintf("%.*s%n",(new_val>>8)&0xff,str,old_pos+1);
    cprintf("%.*s%n",(new_val>>16)&0xff,str,old_pos+2);
    cprintf("%.*s%n",(new_val>>24)&0xff,str,old_pos+3);
}

void
overflow_me(void)
{
	volatile int i=0;
        start_overflow();
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
        uint32_t bp,ip;
	uint32_t args[5];
	struct Eipdebuginfo info;
	bp=read_ebp();
	ip=*((uint32_t*)bp+1);
	int i=0;
	for(i=0;i<5;i++)
	{
		args[i]=*((uint32_t*)bp+i+2);
	}
	cprintf("Stack backtrace:\n");
	while(bp!=0)
	{
		debuginfo_eip(ip,&info);
		cprintf("eip %x ebp %x args %08x %08x %08x %08x %08x",ip,bp,args[0],args[1],
		args[2],args[3],args[4]);
		cprintf("%s:%d: %.*s+%d\n",info.eip_file,info.eip_line,info.eip_fn_namelen,
		info.eip_fn_name,ip-info.eip_fn_addr);
		bp=*(uint32_t*)bp;
		if(bp!=0)
		{
			ip=*((uint32_t*)bp+1);
			for(i=0;i<5;i++)
			{
				args[i]=*((uint32_t*)bp+i+2);
			}
		}
	}
	overflow_me();
	cprintf("%s\n","Backtrace success");
	return 0;
}
static __inline__ unsigned long long rdtsc(void)
{
	unsigned long long x;
	__asm__ volatile (".byte 0x0f,0x31":"=A"(x));
	return x;
}
int time_help(int argc, char **argv, struct Trapframe *tf)
{
	unsigned long long begin,end,interval;
	begin=rdtsc();
	mon_help(argc,argv,tf);
	end=rdtsc();
	interval=end-begin;
	cprintf("Help cycles: %ld\n",interval);
	return 0;
}
int time_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	unsigned long long begin,end,interval;
	begin=rdtsc();
	mon_kerninfo(argc,argv,tf);
	end=rdtsc();
	interval=end-begin;
	cprintf("Kerninfo cycles: %ld\n",interval);
	return 0;
}
int time_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	unsigned long long begin,end,interval;
	begin=rdtsc();
	mon_backtrace(argc,argv,tf);
	end=rdtsc();
	interval=end-begin;
	cprintf("Backtrace cycles: %ld\n",interval);
	return 0;
}
/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	if(strcmp(argv[0],"time")==0)
	{
		if (strcmp(argv[1],"help")==0)
			return commands[3].func(argc, argv, tf);
		if (strcmp(argv[1],"kerninfo")==0)
			return commands[4].func(argc, argv, tf);
		if (strcmp(argv[1],"backtrace")==0)
			return commands[5].func(argc, argv, tf);
	}
	else
	{
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}

// return EIP of caller.
// does not work if inlined.
// putting at the end of the file seems to prevent inlining.
unsigned
read_eip()
{
	uint32_t callerpc;
	__asm __volatile("movl 4(%%ebp), %0" : "=r" (callerpc));
	return callerpc;
}
