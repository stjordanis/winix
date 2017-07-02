#include "winix.h"


proc_t *get_idle(){
	static proc_t *idle;
	kprintf("get idle\n");
	if(idle == NULL){
		idle = new_proc(idle_main, IDLE_PRIORITY, "IDLE");
	}
	return idle;
}


/**
 * Chooses a process to run.
 *
 * Returns:
 *   The process that is runnable with the highest priority.
 *   NULL if no processes are runnable (should never happen).
 *
 * Side Effects:
 *   A proc is removed from a ready_q.
 **/
proc_t *pick_proc() {
	int i;

	//Find the highest-priority non-empty queue
	for (i = 0; i < NUM_QUEUES; i++) {
		if (ready_q[i][HEAD] != NULL) {
			return dequeue(ready_q[i]);
		}
	}

	return get_idle();
}


/**
 * The Scheduler.
 *
 * Notes:
 *   Context of current_proc must already be saved.
 *   If successful, this function does not return.
 *
 * Side Effects:
 *   current_proc has its accounting fields updated, and is reinserted to ready_q.
 *   current_proc is updated to point to the next runnable process.
 *   Context of the new proc is loaded.
 **/
void sched() {

	proc_t *curr = ready_q[3][HEAD];
	int nextpick = 0;
	static int count = 0;

	if (current_proc != NULL && !current_proc->flags) {
		//Accounting
		current_proc->time_used++;

		if (--current_proc->ticks_left) {
			enqueue_head(ready_q[current_proc->priority], current_proc);
		}
		else { //Re-insert process at the tail of its priority queue
			enqueue_tail(ready_q[current_proc->priority], current_proc);
		}
	}

	current_proc = pick_proc();
	if(DEBUG){
		kprintf("pick %d t %d f %d |",current_proc->proc_index, current_proc->ticks_left, current_proc->flags);
		DEBUG--;
	}
		
	
	//Reset quantum if needed
	if (current_proc->ticks_left <= 0) {
		current_proc->ticks_left = current_proc->quantum;
	}

	//Load context and run
	wramp_load_context();
}
