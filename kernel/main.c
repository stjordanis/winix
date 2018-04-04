/**
 * 
 * Main Entry point for Winix
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
#include <kernel/table.h>
#include <winix/bitmap.h>
#include <init.c>
#include <shell.c>

void init_kernel_tasks();
void start_init();
void start_bins();

/**
 * Entry point for WINIX.
 **/
void main() {
    struct proc* p;
    init_bitmap();
    init_mem_table();
    init_proc();
    init_holes();
    init_sched();
    init_syscall_table();

    start_system_init();
    init_kernel_tasks();
    // start_bins();
    p = get_proc(SYSTEM);
    kprintf("%s\n", p->name);
    p = get_proc(CLOCK);
    kprintf("%s\n", p->name);

    kreport_all_procs();

    // init_exceptions();
    // sched();
}

void start_system_init(){
    int ret;
    struct proc* p;
    struct boot_image image = {"SYSTEM", system_main, SYSTEM, 64, MAX_PRIORITY,};
    p = start_kernel_proc(image);
    ASSERT(p != NULL);
    p = exec_proc(NULL,init_code,init_code_length,init_pc,init_offset,"init");
    ASSERT(p != NULL);
    add_free_mem(init_code, init_code_length);
}

void init_kernel_tasks(){
    int i;
    struct proc* p;
    for(i = 0; i < ARRAY_SIZE(boot_table); i++){
        struct boot_image* task = &boot_table[i];
        p = start_kernel_proc(task);
        ASSERT(p != NULL);
    }
    add_free_mem(boot_table, sizeof(boot_table));
}

void start_bins(){
    struct proc* p;
    p = exec_proc(NULL, shell_code,shell_code_length, shell_pc, shell_offset,"shell");
    p->parent = INIT;// hack 
    add_free_mem(shell_code,shell_code_length);
}
