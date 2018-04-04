/**
 * 
 * Process management for WINIX
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

#include <kernel/kernel.h>
#include <winix/mm.h>
#include <winix/srec.h>

// Linked lists are defined by a head and tail pointer.

// Process table
// PRIVATE struct proc _proc_table[NUM_PROCS + NUM_TASKS];

// // Pointer to proc table, public system wise
// PUBLIC struct proc *proc_table;

PUBLIC struct proc _task_table[NUM_TASKS];

// Scheduling queues
PUBLIC struct proc *ready_q[NUM_QUEUES][2];

// The currently-running process
PUBLIC struct proc *current_proc;

PUBLIC struct proc *system_task;


/**
 * How is the process image aligned?
 *
 * Each process image is aligned as shown below
 *
 * Text segment
 * Data segment
 * Bss segment
 * Stack
 * Heap
 *
 * In the struct proc, rbase points to the beggining of 
 * the process image.
 *
 * stack_top is the physical pointer that points to the 
 * physical address of the beginning of the stack. Stack
 * cannot be extended
 *
 * heap_break points to the physical address of the current
 * user heap break
 *
 * heap_bottom, points to the end of the
 * process image. Heap can be extended by extending heap_bottom.
 *
 * Note that all those segments are continous, so whenever a fork
 * is called, we can simply compute the number of pages this process
 * is occuping by doing heap_bottom + 1 - rbase. NB heap_bottom points
 * to the end of the page.
 * 
 */

/**
 * Report all the running procs to serial port 1
 * 
 *
**/
void kreport_all_procs() {
    struct proc *curr;
    kprintf("NAME    PID PPID RBASE      PC         STACK      HEAP       PROTECTION   FLAGS\n");

    foreach_ktask(curr){
        kreport_proc(curr);
    }
    foreach_proc(curr){
        kreport_proc(curr);
    }
}

/**
 * Report the proc's info
**/
void kreport_proc(struct proc* curr) {
    int ptable_idx = PADDR_TO_PAGED(curr->ctx.rbase)/32;
    kprintf("%-08s %-03d %-04d 0x%08x 0x%08x 0x%08x 0x%08x %d 0x%08x %d\n",
            curr->proc_nr,
            curr->name,
            curr->tgid,
            get_proc(curr->parent)->tgid,
            curr->ctx.rbase,
            get_physical_addr(get_pc_ptr(curr),curr),
            get_physical_addr(curr->ctx.m.sp,curr),
            curr->heap_break,
            ptable_idx,
            curr->ctx.ptable[ptable_idx],
            curr->state);
}

/**
 * get next free pid
**/
pid_t get_next_pid(){
    static pid_t pid = 0;
    return pid++;
}

/**
 * Gets a pointer to a process by pid
 * if multiple process has the same pid, the first
 * one is returned
 *
 * Parameters:
 *   pid               The process's pid'.
 *
 * Returns:            The relevant process, or NULL if it can't be found
 **/
// struct proc* get_proc_by_tgid(pid_t pid){
//     struct proc* curr;
//     if(pid == 0)
//         return get_proc(SYSTEM);
//     foreach_proc(curr){
//         if(curr->tgid == pid){
//             return curr;
//         }
//     }
//     return NULL;
// }

/**
 * Gets a pointer to a process.
 *
 * Parameters:
 *   proc_nr        The process to retrieve.
 *
 * Returns:            The relevant process, or NULL if it does not exist.
 **/
struct proc *get_proc(pid_t pid) {
    struct proc* curr;

    foreach_proc(curr){
        if(curr->pid == pid)
            return curr;
    }
    return NULL;
    
    // struct proc* who;
    // struct proc* table = proc_nr < NUM_TASKS ? task_table : proc_table;
    // if (IS_PROCN_OK(proc_nr))
    //     if(IS_INUSE(who = table + proc_nr))
    //         return who;
    // return NULL;
}

/**
 * similar to get_proc(), but this one makes sure the 
 * returning proc is runnable
 * @param  proc_nr 
 * @return         
 */
struct proc *get_runnable_proc(int proc_nr){
    struct proc *p = get_proc(proc_nr);
    if(p && IS_RUNNABLE(p))
        return p;
    return NULL;
}

/**
 * Adds a proc to the tail of a list.
 *
 * Parameters:
 *   q        An array containing a head and tail pointer of a linked list.
 *   proc    The proc struct to add to the list.
 **/
void enqueue_tail(struct proc **q, struct proc *proc) {
    if (q[HEAD] == NULL) {
        q[HEAD] = q[TAIL] = proc;
    }
    else {
        q[TAIL]->next = proc;
        q[TAIL] = proc;
    }
    proc->next = NULL;
}

/**
 * Adds a proc to the head of a list.
 *
 * Parameters:
 *   q        An array containing a head and tail pointer of a linked list.
 *   proc    The proc struct to add to the list.
 **/
void enqueue_head(struct proc **q, struct proc *proc) {
    struct proc *curr = NULL;
    if (q[HEAD] == NULL) {
        proc->next = NULL;
        q[HEAD] = q[TAIL] = proc;
    }
    else {
        proc->next = q[HEAD];
        q[HEAD] = proc;
    }
}

/**
 * Removes the head of a list.
 *
 * Parameters:
 *   q        An array containing a head and tail pointer of a linked list.
 *
 * Returns:
 *   The proc struct that was removed from the head of the list
 *   NULL if the list is empty.
 **/
struct proc *dequeue(struct proc **q) {
    struct proc *p = q[HEAD];

    if (p == NULL)
        return NULL;

    if (q[HEAD] == q[TAIL]) { // Last item
        q[HEAD] = q[TAIL] = NULL;
    }
    else { // At least one remaining item
        q[HEAD] = p->next;
    }
    p->next = NULL;
    return p;
}

/**
 * remove the process from the scheduling queue
 * @param  h process to be removed
 * @return   0 on success, -1 if not
 */
int dequeue_schedule( struct proc *h) {
    struct proc *curr;
    struct proc *prev = NULL;
    struct proc ** q = ready_q[h->priority];

    curr = q[HEAD];

    while (curr != h && curr != NULL) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL)
        return ERR;

    if (prev == NULL) {
        q[HEAD] = curr->next;
        if(curr->next == NULL){
            q[TAIL] = NULL;
        }
    } else {
        prev->next = curr->next;
    }
    return OK;
}

/**
 * enqueue the process to the scheduling queue
 * according to its priority
 * @param p 
 */
void enqueue_schedule(struct proc* p) {
    enqueue_tail(ready_q[p->priority], p);
}

struct proc* new_proc(){
    struct proc* who = (struct proc*)kmalloc(sizeof(struct proc));
    if(!who)
        return NULL;
    proc_set_default(who);
    who->pid = who->tgid = get_next_pid();
    who->flags |= IN_USE;
    __list_add(&task->proc_list, system_task->proc_list.prev, \
                    &system_task->proc_list);
    return who;
}

/**
 * zombify the process, the process will be removed from the
 * ready_q, and memory will be released by the system
 * @param p 
 */
void zombify(struct proc *p){
    p->state |= STATE_ZOMBIE;
    dequeue_schedule(p);
}

/**
 * add this struct to the free_proc list. This method should
 * only be called when a zombie process is released
 * @param p 
 */
void release_zombie(struct proc *p){
    if(p->state & STATE_ZOMBIE){
        __list_del_entry(&p->proc_list);
        kfree(p);
    }
}

/**
 * get a free struct proc from the system proc table
 * @return pointer to the free slot, or NULL
 */
// struct proc *get_free_proc_slot() {
//     int i;
//     struct proc *who;
//     for(i = INIT; i <= NUM_PROCS; i++){
//         who = &proc_table[i];
//         if(!IS_INUSE(who)){
//             proc_set_default(who);
//             who->state = STATE_RUNNABLE;
//             who->flags = IN_USE;
//             who->tgid = get_next_pid();
//             return who;
//         }
//     }
//     return NULL;
// }

/**
 * set the process struct to default values
 * @param p 
 */
void proc_set_default(struct proc *p) {
    int pnr_bak = p->proc_nr;
    memset(p, 0, sizeof(struct proc));
    p->proc_nr = pnr_bak;
    p->state = -1;

    memset(p->ctx.m.regs, -1, NUM_REGS);
    p->ctx.cctrl = DEFAULT_CCTRL;

    p->quantum = DEFAULT_USER_QUANTUM;
    p->ctx.ptable = p->protection_table;
    p->alarm.proc_nr = p->proc_nr;
    p->sig_table[SIGCHLD].sa_handler = SIG_IGN;
}

/**
 * allocate stack for kernel processes or kernel thread
 * stack size is defined by KERNEL_STACK_SIZE
 * this method can be used for creating kernel process 
 * or kernel threads'  stack
 * @param       who 
 * @return      virtual address of the stack
 */
reg_t* alloc_kstack(struct proc *who){
    int page_size;
    int index;
    ptr_t *addr, *stack_top;

    stack_top = user_get_free_pages(who, KERNEL_STACK_SIZE, GFP_HIGH);
    
    addr = stack_top + KERNEL_STACK_SIZE - 1;
    *stack_top = STACK_MAGIC;
    who->stack_top = stack_top;
    return get_virtual_addr(addr, who);
}

/**
 * set corressponding fields of struct pro
 */
void set_proc(struct proc *p, void (*entry)(), const char *name) {
    p->ctx.m.pc = entry;
    strncpy(p->name, name, PROC_NAME_LEN - 1);
}

/**
 * The first page in the kernel is set as unaccessible
 * Remember that NULL is just a macro of a pointer pointing at 0
 *  #define NULL    (void *)0
 * Thus when NULL pointer is dereferenced (is written), 
 * program attemps to write value to address 0, we would set
 * the first page unaccessible, 
 * so that GFP is raised when null pointer is dereferenced
 * 
 */
 void kset_ptable(struct proc* who){
    if(IS_KERNEL_PROC(who)){
        bitmap_fill(who->ctx.ptable, PTABLE_LEN);
        bitmap_clear_bit(who->ctx.ptable, PTABLE_LEN, 0);
    }
}



/**
 * Creates a new kernel process and adds it to the runnable queue
 *
 * Parameters:
 *   entry        A pointer to the entry point of the new process.
 *   priority    The scheduling priority of the new process.
 *   name        The name of the process, up to PROC_NAME_LEN characters long.
 *
 * Returns:
 *   The newly-created proc struct.
 *   NULL if the priority is not valid.
 *   NULL if the process table is full.
 *
 * Side Effects:
 *   A proc is removed from the free_proc list, reinitialised, and added to ready_q.
 */

struct proc *start_kernel_proc(struct boot_image* task) {
    struct proc *who = new_proc();
    
    if(who){
        set_proc(who, task->entry, task->name);
        kset_ptable(who);
        who->quantum = task->quantum;
        who->ctx.m.sp = alloc_kstack(who);
        who->priority = task->priority;
        who->state = STATE_RUNNABLE;
        
        enqueue_schedule(who);
    }
    return who;
}

/**
 * process memory control, 
 * @param  who       
 * @param  page_addr the virtual address memory
 * @param  flags     PROC_ACCESS OR PROC_NO_ACCESS
 * @return           
 */
int proc_memctl(struct proc* who ,vptr_t* page_addr, int flags){
    int paged = PADDR_TO_PAGED(get_physical_addr(page_addr, who)); // get page descriptor
    
    if(flags == PROC_ACCESS){
        return bitmap_set_bit(who->ctx.ptable, PTABLE_LEN, paged);
    }else if(flags == PROC_NO_ACCESS){
        return bitmap_clear_bit(who->ctx.ptable, PTABLE_LEN, paged);
    }
    return ERR;
}

/**
 * allocate memory for the given process
 * @param  who        
 * @param  text_data_length total of text and data size
 * @param  stack_size 
 * @param  heap_size  
 * @param  flags      PROC_SET_SP or/and PROC_SET_HEAP    
 * @return            
 */
int alloc_proc_mem(struct proc *who, int text_data_length, int stack_size, int heap_size){
    const int vm_offset = PAGE_LEN;
    int proc_len;
    int td_aligned;
    ptr_t *bss_start;
    int bss_size;

    // make sizes page aligned
    // text_Data_length is the length of text plus data.
    // Since srec file does not include the size of bss segment by
    // default, so we have to manually set it. By default, bss segment
    // extends from the end of the data segment. So the initial bss 
    // size is simply aligned size minus exact size
    td_aligned = align_page(text_data_length);
    bss_size = td_aligned - text_data_length;

    stack_size = align_page(stack_size);
    heap_size = align_page(heap_size);
    
    // give bss an extra page if its not enough. not that if bss size 
    // is not enough, it could extend to the stack segment, which could
    // potentially corrupt the stack 
    if(bss_size < MIN_BSS_SIZE)
        bss_size += PAGE_LEN;

    proc_len = vm_offset + stack_size + text_data_length + bss_size + heap_size;
    who->ctx.rbase = user_get_free_pages(who, proc_len, GFP_NORM);
    if(who->ctx.rbase == NULL)
        return ENOMEM;

    // for information on how process memory are structured, 
    // look at the first line of this file
    who->stack_top = who->ctx.rbase + vm_offset;
    who->ctx.m.sp = get_virtual_addr(who->stack_top + stack_size - 1,who);
    *(who->stack_top) = STACK_MAGIC;

    // set bss segment to 0
    bss_start = who->ctx.rbase + vm_offset + stack_size + text_data_length;
    memset(bss_start, 0, bss_size);

    who->heap_break = who->ctx.rbase + vm_offset + stack_size + text_data_length + bss_size;
    who->heap_bottom = who->heap_break + heap_size - 1;
    
    return OK;
}

/**
 * Copy values onto the user stack, this is very similar to memcpy
 * @param  who 
 * @param  src 
 * @param  len 
 * @return     
 */
int copyto_user_stack(struct proc *who, void *src, size_t len){
    reg_t *sp = get_physical_addr(who->ctx.m.sp,who);
    sp -= len;
    memcpy(sp,src,len);
    who->ctx.m.sp = get_virtual_addr(sp,who);
    return OK;
}

vptr_t* copyto_user_heap(struct proc* who, void *src, size_t len){
    vptr_t* addr = get_virtual_addr(who->heap_break, who);
    memcpy(who->heap_break, src, len);
    who->heap_break += len;
    return addr;
}


/**
 * Initialises the process table
 *
 * Side Effects:
 *   ready_q is initialised to all empty queues.
 *   free_proc queue is initialised and filled with processes.
 *   proc_table is initialised to all DEAD processes.
 *   current_proc is set to NULL.
 **/
void init_proc() {
    int i, procnr_offset;
    struct proc *p;
    int preset_pnr;
    // Initialise queues

    for (i = 0; i < NUM_QUEUES; i++) {
        ready_q[i][HEAD]  = ready_q[i][TAIL] = NULL;
    }

    current_proc = NULL;
}



