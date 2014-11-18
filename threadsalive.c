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
    unsigned char *stack = (unsigned char *)malloc(128000);
    main1.uc_stack.ss_sp = stack;
    main1.uc_stack.ss_size = 128000;
    main1.uc_link = NULL;
	
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
    if (threadhead != NULL){
        Node *temp = threadhead;
	threadhead = threadhead->next;
	temp->next = NULL;
        insert(temp, &threadhead); 
        //temp->next = NULL;
        swapcontext(&temp->ctx, &(threadhead->ctx));
    }
	return;
}
    
int ta_waitall(void) {
    if (threadhead != NULL){
        while (1){
            if (threadhead == NULL){
                free(main1.uc_stack.ss_sp);
                return 0;
            }
            //Node *temp = threadhead;
            //assert(threadhead->ctx.uc_link != &main1);
            printf("ATTEMPTING SWAP CONTEXT\n");
            swapcontext(&main1, &threadhead->ctx);
            printf("THREAD COMPLETED\n");
            Node *temp = threadhead;
            threadhead = threadhead->next;
	    if  (threadhead == NULL) {
		}
            free(temp->ctx.uc_stack.ss_sp);
            free(temp);
        }
    }  
        
    else {
        free(main1.uc_stack.ss_sp);
        return 0;
    }
    free(main1.uc_stack.ss_sp);
   return -1;
    
}


/* ***************************** 
     stage 2 library functions
   ***************************** */

void ta_sem_init(tasem_t *sem, int value) {
    assert(value >= 0);
    
    sem->count = value;
    sem->queue = malloc(sizeof(Node));
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
	sem->count --;
	while(sem->count==0){
		Node *temp = threadhead;
		threadhead = threadhead->next;
		insert(sem->queue, &temp); // add it to the waiting queue and do CS
		swapcontext(&temp->ctx, &threadhead->ctx);
	}
	//sem->count --;
	return;	
}

void ta_lock_init(talock_t *mutex) {
	mutex->semaphore = malloc(sizeof(tasem_t));
	mutex->semaphore->count = 0;
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
	//while (compareandswap(&mutex->sem->value, 0, 1)!=0){;}
	return;
}

void ta_unlock(talock_t *mutex) {
	ta_sem_post(mutex->semaphore);
	//mutex->sem->value = 0;
}


/* ***************************** 
     stage 3 library functions
   ***************************** */

void ta_cond_init(tacond_t *cond) {
	ta_sem_init(cond->semaphore, 0);
	cond->state = 0;
}

void ta_cond_destroy(tacond_t *cond) {
	ta_sem_destroy(cond->semaphore);

}

void ta_wait(talock_t *mutex, tacond_t *cond) {
	ta_lock(mutex);	
	ta_sem_wait(cond);
	cond->state = 0;
	ta_unlock(mutex);
}

void ta_signal(tacond_t *cond) {
	ta_sem_post(cond);
}

