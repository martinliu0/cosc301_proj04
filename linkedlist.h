#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__
#include <ucontext.h>
/* your list data structure declarations */

typedef struct node {
	//#define STACKSIZE 8192
	//unsigned char *stack = (unsigned char *)malloc(8192);
	ucontext_t ctx; 

	//int done; //not done, initially
	//int previous; //if it is 1, it means it was the previous thread that was running, there should only be 1 in the q for stage 1
    struct node *next;
} Node;

/* your function declarations associated with the list */
void insert(Node *, Node **);
void delete(Node*);

#endif // __LINKEDLIST_H__
