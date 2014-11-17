#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>
//#include "linkedlist.h"
#include "threadsalive.h"

void release_queue(tasem_t *sem){
	// release any associated resources
	Node *temp = sem->queue;
	while (sem->queue!=NULL) {
		temp = sem->queue;
		sem->queue = sem->queue->next;
		free(temp);
	}
	free(sem->queue);
}

void delete(Node *head){
    while(head != NULL){
        Node *temp = head;
        head = head->next;
        free(temp->ctx.uc_stack.ss_sp);
        free(temp);
    }
}
void insert(Node *newta, Node **head){
    if ((*head) == NULL) {
        *head = newta; 
    }
    else{
        Node *curr = *head;
        while(curr->next != NULL){
            curr = curr->next;
        }

        curr->next = newta;

    }
}





