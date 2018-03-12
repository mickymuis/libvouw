/* This file is from Linux Kernel (include/linux/list_sort.h) 
 * and modified by simply removing hardware prefetching of list items. 
 * Here by copyright, credits attributed to wherever they belong.
 * Licensed under GPL.
 */

#ifndef _LINUX_LIST_SORT_H
#define _LINUX_LIST_SORT_H

struct list_head;
void list_sort(void *priv, struct list_head *head,
               int (*cmp)(void *priv, struct list_head *a,
                          struct list_head *b));

#endif
