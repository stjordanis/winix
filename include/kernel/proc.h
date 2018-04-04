/**
 * 
 * Process Management for WINIX.
 *
 * @author Bruce Tan
 * @email brucetansh@gmail.com
 * 
 * @author Paul Monigatti
 * @email paulmoni@waikato.ac.nz
 * 
 * @create date 2016-09-19
 * 
*/

#ifndef _W_PROC_H_
#define _W_PROC_H_ 1

#include <sys/ucontext.h>
#include <sys/types.h>
#include <signal.h>
#include <winix/type.h>
#include <winix/timer.h>
#include <winix/kwramp.h>
#include <winix/__list.h>

#define INTERRUPT                   -30

// Kernel Process
#define NUM_TASKS                   3
#define SYSTEM                      0
#define INIT                   		1
#define CLOCK                    	2
#define IDLE                       	3

// Number of User procs
#define NUM_PROCS               	8

// Total number of procs
#define NUM_PROCS_AND_TASKS         (NUM_TASKS + NUM_PROCS)

// Scheduling
#define NUM_QUEUES              	5
#define MAX_PRIORITY            	0
#define MIN_PRIORITY            	4

// Max string len for a process name 
//(including NULL terminator)
#define PROC_NAME_LEN           	16

// min bss segment size
#define MIN_BSS_SIZE            	200

// stack
#define STACK_MAGIC             	0x12345678
#define USER_STACK_SIZE         	1024
#define KERNEL_STACK_SIZE       	1024

// heap
#define USER_HEAP_SIZE          	2048

// Signal PCB Context
#define SIGNAL_CTX_LEN          	21

// Process Defaults
#define DEFAULT_FLAGS            	0
#define PTABLE_LEN               	32
#define DEFAULT_CCTRL            	0xff9
#define DEFAULT_STACK_POINTER    	0x00000
#define USER_CCTRL               	0x8 // OKU is set to 0
#define DEFAULT_RBASE            	0x00000
#define DEFAULT_PTABLE           	0x00000
#define DEFAULT_KERNEL_QUANTUM   	64
#define DEFAULT_USER_QUANTUM		8
#define DEFAULT_REG_VALUE        	0xffffffff
#define DEFAULT_MEM_VALUE        	0xffffffff
#define DEFAULT_RETURN_ADDR        	0x00000000
#define DEFAULT_PROGRAM_COUNTER    	0x00000000

// Process Scheduling Flags state, process is runnable when state == 0
#define STATE_RUNNABLE              0x00000000    /* Process is running or in the ready queue */
#define STATE_SENDING               0x00000001    /* process blocked trying to SEND */
#define STATE_RECEIVING             0x00000002    /* process blocked trying to RECEIVE */
#define STATE_WAITING               0x00000004    /* process blocked wait(2) */
#define STATE_PAUSING               0x00000008    /* process blocked by sigsuspend(2) or pause(2) */
#define STATE_VFORKING              0x00000010    /* parent is blocked by vfork(2) */
#define STATE_STOPPED               0x00000020    /* Stopped by SIGSTOP or SIGTSTP */
#define STATE_ZOMBIE                0x80000000    /* Zombie process */

// Process Information flags
#define IN_USE                    	0x0001      /* process slot is in use */
#define BILLABLE                	0x0002      /* Set when user is invoking a system call */
#define DISABLE_FIRST_PAGE          0x0004      /* Set when the first page of the user address space is disabled */

// proc_memctll flags
#define PROC_ACCESS                	1
#define PROC_NO_ACCESS            	0


struct k_context{
    mcontext_t m;
    reg_t *rbase;
    reg_t *ptable;
    reg_t cctrl;                  	// len 19 words
};

struct proc_vm{
    vptr_t* text;
    vptr_t* data;
    vptr_t* bss;
    vptr_t* stack_top;
    vptr_t* heap_break;             // Heap_break is also the physical address of the curr
                                	// Brk, retrived by syscall brk(2)
    vptr_t* heap_bottom;         	// Bottom of the process image
};


struct proc_sched{
    /* Scheduling */
    struct proc *next;            	// Next pointer
    int priority;                	// Priority
    int quantum;                	// Timeslice length
    int ticks_left;                	// Timeslice remaining

    /* Accounting */
    clock_t time_used;            	// CPU time used
    clock_t sys_time_used;        	// system time used while the system is executing on behalf 
                                	// of this proc
};

struct boot_image{
    char name[PROC_NAME_LEN];
    void (*entry)();
    int proc_nr;
    int quantum;
    int priority;
};

/**
 * Process structure for use in the process table.
 *
 * Note:     Do not update this structure without also
 *             updating the definitions in "wramp.s"
 **/
typedef struct proc {
    struct k_context ctx;

    /* IPC messages */
    int state;                      // schedling flags
    struct message* message;    	// Message Buffer
                                	// len 21 words
                                	// DO NOT MODIFY or CHANGE the order of the above
                                    // fields unless you know what you are doing
    

    /* Heap and Stack*/
    ptr_t* stack_top;             	// Stack_top is the physical address
    ptr_t* heap_break;             	// Heap_break is also the physical address of the curr
                                	// Brk, retrived by syscall brk(2)
    ptr_t* heap_bottom;         	// Bottom of the process image

    /* Protection */
    reg_t protection_table[PTABLE_LEN];

    /* IPC queue */
    struct proc *sender_q;        	// Head of process queue waiting to send to this process
    struct proc *next_sender;     	// Link to next sender in the queue

    /* Pending messages, used by winix_notify */
    unsigned int notify_pending;	// bitmap for masking list of pending messages by system proc

    /* Scheduling */
    struct proc *next;            	// Next pointer
    int priority;                	// Priority
    int quantum;                	// Timeslice length
    int ticks_left;                	// Timeslice remaining

    /* Accounting */
    clock_t time_used;            	// CPU time used
    clock_t sys_time_used;        	// system time used while the system is executing on behalf 
                                	// of this proc

    /* Metadata */
    char name[PROC_NAME_LEN];    	// Process name
    int exit_status;            	// Storage for status when process exits
    int sig_status;                	// Storage for siginal status when process exits
    pid_t pid;                    	// Thread id
    pid_t tgid;                     // Thread Group id
    pid_t procgrp;                	// Pid of the process group (used for signals)
    pid_t wpid;                    	// pid this process is waiting for
    int woptions;                   // waiting options
    int parent;                    	// proc_index of parent
    int flags;                	// information flags

    /* Process Table Index */
    int proc_nr;                	// Index in the process table

    /* Signal Information */
    sigset_t sig_pending;
    sigset_t sig_mask;
    sigset_t sig_mask2;
    struct sigaction sig_table[_NSIG];
    sighandler_t sa_restorer;

    /* Alarm */
    struct timer alarm;

    struct list_head proc_list;
    struct list_head child_list;
    
} proc_t;

/**
* Pointer to the current process.
**/
// extern struct proc *current_proc;

// extern struct proc *proc_table;
extern struct proc *ready_q[NUM_QUEUES][2];
extern struct proc *block_q[2];
// extern struct proc *task_table;
extern struct proc *system_task;

#define IS_PROCN_OK(i)                  ((i)> -NUM_TASKS && (i) <= NUM_PROCS)
#define IS_PRIORITY_OK(priority)        (0 <= (priority) && (priority) < NUM_QUEUES)
#define IS_KERNEL_PROC(p)               ((p)->ctx.rbase == NULL)
#define IS_KERNELN(n)                   ((n)<= 0 && (n)> -NUM_TASKS)
#define IS_USER_PROC(p)                 ((p)->ctx.rbase != NULL)

#define IS_IDLE(p)                      ((p)->proc_nr == IDLE)
#define IS_SYSTEM(p)                    ((p)->proc_nr == SYSTEM)

#define IS_INUSE(p)                     ((p)->flags & IN_USE)
#define IS_RUNNABLE(p)                  ((p)->state == STATE_RUNNABLE)
#define IS_ZOMBIE(p)                    (IS_INUSE(p) && (p)->state & STATE_ZOMBIE)
#define IS_BLOCKED(p)                   (IS_INUSE(p) && (p)->state > 0)

#define CHECK_STACK(p)                  (*((p)->stack_top) == STACK_MAGIC)
#define GET_DEF_STACK_SIZE(who)         (IS_USER_PROC(who) ? USER_STACK_SIZE : KERNEL_STACK_SIZE)
#define GET_HEAP_TOP(who)               ((who)->stack_top + GET_DEF_STACK_SIZE(who))

#define TASK_NR_TO_SID(tnr)             (tnr <= 0 ? -tnr + 1 : tnr)
#define SID_TO_TASK_NR(sid)             (-sid + 1)


#define next_task(task)\
    list_next_entry(struct proc, task, proc_list)

#define foreach_proc(curr)\
for(curr = proc_table + INIT; curr <= proc_table + NUM_PROCS ; curr++)\
    if(IS_INUSE(curr))

#define foreach_ktask(curr)\
    for(curr = task_table ; curr < task_table + NUM_TASKS ; curr++)\

#define foreach_proc_and_task(curr)\
for(curr = proc_table - NUM_TASKS + 1; curr <= proc_table + NUM_PROCS; curr++)\
    if(IS_INUSE(curr))

#define foreach_child(curr, parent_proc)\
list_for_each_entry(struct proc_list, curr, &parent_proc->child_list, list)


void* get_pc_ptr(struct proc* who);
void enqueue_tail(struct proc **q, struct proc *proc);
void enqueue_head(struct proc **q, struct proc *proc);
struct proc *dequeue(struct proc **q);
void init_proc();
void proc_set_default(struct proc *p);
reg_t* alloc_stack(struct proc *who);
void set_proc(struct proc *p, void (*entry)(), const char *name);
struct proc *start_kernel_proc(struct boot_image* task);
struct proc *start_user_proc(size_t *lines, size_t length, size_t entry, int priority, const char *name);
struct proc *get_free_proc_slot();
int alloc_proc_mem(struct proc *who, int tdb_length, int stack_size, int heap_size);
void enqueue_schedule(struct proc* p);
reg_t* alloc_kstack(struct proc *who);
int proc_memctl(struct proc* who ,vptr_t* page_addr, int flags);
pid_t get_next_pid();
struct proc* get_proc_by_tgid(pid_t pid);
struct proc *get_proc(int proc_nr);
struct proc *get_runnable_proc(int proc_nr);
void kreport_all_procs();
void kreport_proc(struct proc* curr);
struct proc *pick_proc();
void zombify(struct proc *p);
void release_zombie(struct proc*p);
int copyto_user_stack(struct proc *who, void *src, size_t len);
vptr_t* copyto_user_heap(struct proc* who, void *src, size_t len);
int build_initial_stack(struct proc* who, int argc, char** argv, char** env, struct proc* srcproc);

#define release_proc_slot(p)    release_zombie(p)

#endif

