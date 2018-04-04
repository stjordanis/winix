#ifndef __WINIX_LIST_H_
#define __WINIX_LIST_H_


#define LIST_POISON1  ((void *) 0)
#define LIST_POISON2  ((void *) 1)

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

void INIT_LIST_HEAD(struct list_head *list);
#define   INIT_LIST_HEAD(list)\
do{\
	WRITE_ONCE((list)->next, list);\
	(list)->prev = list;\
}while(0)

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next);
#define   __list_add(new, prevl, nextl)\
do{\
\
	(nextl)->prev = new;\
	(new)->next = nextl;\
	(new)->prev = prevl;\
    (prevl)->next = new;\
}while(0)


/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
void __list_del(struct list_head * prev, struct list_head * next);
#define   __list_del( prevl,  nextl)\
do{\
	(nextl)->prev = prevl;\
	(prevl)->next = nextl;\
}while(0)


/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
void __list_del_entry(struct list_head *entry);
#define   __list_del_entry(entry)\
do{\
	__list_del((entry)->prev, (entry)->next);\
}while(0)

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
* list_first_entry - get the first element from a list
* @ptr:	the list head to take the element from.
* @type:	the type of the struct this is embedded in.
* @member:	the name of the list_head within the struct.
*
* Note, that list is expected to be not empty.
*/
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/**
* list_last_entry - get the last element from a list
* @ptr:	the list head to take the element from.
* @type:	the type of the struct this is embedded in.
* @member:	the name of the list_head within the struct.
*
* Note, that list is expected to be not empty.
*/
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)



/**
 * list_next_entry - get the next element in list
 * @struct:	typeof(*(pos))
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define list_next_entry(struct, pos, member) \
	list_entry((pos)->member.next, struct, member)

/**
 * list_prev_entry - get the prev element in list
 * @struct:	typeof(*(pos))
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define list_prev_entry(struct, pos, member) \
	list_entry((pos)->member.prev, struct, member)

#endif //__WINIX_LIST_H_
