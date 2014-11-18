/*
Claire Grace and Martin Liu
We both worked on writing and debugging all of the code together.

 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <ucontext.h>
#include <string.h>
 #include <errno.h>
#include "linkedlist.h"
#include "threadsalive.h"

/* ***************************** 
     stage 1 library functions
   ***************************** */
static ucontext_t main1;
static Node *threadhead = NULL;
//static Node *previous = NULL;
int any_waiting = 0;


void ta_libinit(void) {
    //unsigned char *stack = (unsigned char *)malloc(128000);

	
    return;
}

void ta_create(void (*func)(void *), void *arg) {
    #define STACKSIZE 128000

    unsigned char *stack = (unsigned char *)malloc(STACKSIZE);
    Node *newta = malloc(sizeof(Node));
    assert(stack); //make sure it was allocated
    getcontext(&newta->ctx);
    newta->ctx.uc_stack.ss_sp = stack;
    newta->ctx.uc_stack.ss_size = STACKSIZE;
    newta->ctx.uc_link = &main1;
    //previous = &newta->ctx;
    insert(newta, &threadhead);
    makecontext(&(newta->ctx), (void (*)(void))func, 1, arg);
   
    return;
}

void ta_yield(void) {
    if (threadhead->next != NULL){
        Node *temp = threadhead;
	    threadhead = threadhead->next;

	    temp->next = NULL;
        insert(temp, &threadhead);

        if (swapcontext(&temp->ctx, &threadhead->ctx) == -1){
            fprintf(stderr, "swapcontext failed: %s\n", strerror(errno));
        }
    }
	return;
}
    
int ta_waitall(void) {
    if (threadhead != NULL){
        while (1){
            if (threadhead == NULL){
                return 0;
            }
            if (swapcontext(&main1, &threadhead->ctx) == -1){
                fprintf(stderr, "swapcontext failed: %s\n", strerror(errno));
            }
  
            Node *temp = threadhead;
            threadhead = threadhead->next;
            free(temp->ctx.uc_stack.ss_sp);
            free(temp);
		
        }
        return 0;
    }  
        

   return -1;
    
}


/* ***************************** 
     stage 2 library functions
   ***************************** */

void ta_sem_init(tasem_t *sem, int value) {
    assert(value >= 0);
    
    sem->count = value;
    sem->queue = malloc(sizeof(Node));
    sem->queue = NULL;
}

void ta_sem_destroy(tasem_t *sem) {
    delete(sem->queue);

}

void ta_sem_post(tasem_t *sem) {
    sem->count ++;
    if (sem->count > 0 && sem->queue != NULL){
        Node *temp = sem->queue;
        sem->queue = sem->queue->next;
	    insert(temp, &threadhead); 
    }
    return;
}


void ta_sem_wait(tasem_t *sem) {
	
	while(sem->count==0){
        
		Node *temp = threadhead;
		threadhead = threadhead->next;
		insert(temp, &sem->queue); // add it to the waiting queue and do CS
		if (swapcontext(&temp->ctx, &threadhead->ctx) == -1){
            fprintf(stderr, "swapcontext failed: %s\n", strerror(errno));
        } 
	}
	sem->count --;
	return;	
}

void ta_lock_init(talock_t *mutex) {
	mutex->semaphore = malloc(sizeof(tasem_t));
	mutex->semaphore->count = 1;
	mutex->semaphore->queue = malloc(sizeof(Node));
	return;
	//ta_sem_init(mutex->semaphore, 0);
}

void ta_lock_destroy(talock_t *mutex) {
	ta_sem_destroy(mutex->semaphore);
	//free(mutex);
}

void ta_lock(talock_t *mutex) {
	ta_sem_wait(mutex->semaphore);
	return;
}

void ta_unlock(talock_t *mutex) {
	ta_sem_post(mutex->semaphore);
}


/* ***************************** 
     stage 3 library functions
   ***************************** */

void ta_cond_init(tacond_t *cond) {
    cond->queue = malloc(sizeof(Node));
    cond->queue = NULL;
}

void ta_cond_destroy(tacond_t *cond) {
	delete(cond->queue);

}

void ta_wait(talock_t *mutex, tacond_t *cond) {
    ta_unlock(mutex);
	if (threadhead->next != NULL){
        Node *temp = threadhead;
        threadhead = threadhead->next;
        insert(temp, &cond->queue); 

        if (swapcontext(&temp->ctx, &threadhead->ctx) == -1){
            fprintf(stderr, "swapcontext failed: %s\n", strerror(errno));
        } 
        ta_lock(mutex);
    }
}

void ta_signal(tacond_t *cond) {
    if (cond->queue != NULL){
        Node *temp = cond->queue;
        cond->queue = cond->queue->next;
        insert(temp, &threadhead);

    }
}

