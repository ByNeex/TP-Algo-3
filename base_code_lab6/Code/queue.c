/*-----------------------------------------------------------------*/
/*
 Licence Informatique - Structures de données
 Mathias Paulin (Mathias.Paulin@irit.fr)
 
 Implantation du TAD Queue étudié en cours.
 
 */
/*-----------------------------------------------------------------*/
#include "queue.h"/*-----------------------------------------------------------------*/
/*
 Licence Informatique - Structures de données
 Mathias Paulin (Mathias.Paulin@irit.fr)
 
 Implantation du TAD Queue étudié en cours.
 
 */

/*-----------------------------------------------------------------*/
#ifndef __QUEUE__H__
#define __QUEUE__H__
#include <stdio.h>
#include <stdbool.h>

/** \defgroup ADTQueue Queue
 Documentation of the implementation of the abstract data type BinarySearchTree.
 @{
 */

/** \defgroup QueueType Type definition.
 @{
 */
/** Opaque definition of the type Queue */
typedef struct s_queue Queue;
typedef Queue *ptrQueue;
/** @} */

/** \defgroup QueueBase Mandatory functions for using Queues.
 * @{
 *
*/
/** Constructor : builds an empty queue
	queue : -> Queue
*/
Queue *createQueue(void);

/** Constructor : add an element to the queue
	queue_push : Queue x int -> Queue
	@note : side effect on the queue q
*/
Queue *queuePush(Queue *q, const void *v);

/** Destructor : delete the queue.
 */
void deleteQueue(ptrQueue *q);

/** Operator : pop an element from the queue
	queue_pop : Queue -> Queue
	@pre !queue_empty(q)
*/
Queue *queuePop(Queue *q);

/** Operator : acces to the frist element of the queue
	queue_top : Queue -> int
	@pre !queue_empty(q)
*/
const void *queueTop(Queue *q);

/** Operator : is the queue empty ?
	queue_empty : Queue -> boolean
*/
bool queueEmpty(Queue *q);

/** Operator : size of the queue ?
 size : Queue -> int
 */
unsigned int queueSize(Queue *q);

/** @} */

/** @} */
#endif

#include <assert.h>
#include <stdlib.h>

/* Full definition of the queue structure */
typedef struct s_internalQueue {
	const void* value;
	struct s_internalQueue* next;
} InternalQueue;

struct s_queue{
	InternalQueue* head;
	InternalQueue* tail;
	unsigned int size;
};

Queue* create_queue(void){
	Queue* q = calloc(1, sizeof(Queue));
	return(q);
}

void delete_queue(ptrQueue *q) {
	InternalQueue* toDelete = (*q)->head;
	while (toDelete) {
		InternalQueue* f = toDelete;
		toDelete = toDelete->next;
		free (f);
	}
	free(*q);
	*q = NULL;
}

Queue* queue_push(Queue* q, const void* v){
	InternalQueue* new = calloc(1, sizeof(InternalQueue));
	new->value = v;
	InternalQueue** insert_at = (q->size ? &(q->tail->next) : &(q->head));
	*insert_at = new;
	q->tail = new;
	++(q->size);
	return (q);
}

Queue* queue_pop(Queue* q){
	assert (!queue_empty(q));
	InternalQueue* old = q->head;
	q->head = q->head->next;
	--(q->size);
	free (old);
	return (q);
}

const void* queue_top(const Queue* q){
	assert (!queue_empty(q));
	return (q->head->value);
}

bool queue_empty(const Queue* q){
	return (queue_size(q) == 0);
}

unsigned int queue_size(const Queue* q) {
	return q->size;
}

void queue_map(const Queue* q, QueueMapOperator f, void* user_param) {
	for (InternalQueue *c = q->head; c ; c = c->next)
		f(c->value, user_param);
}

