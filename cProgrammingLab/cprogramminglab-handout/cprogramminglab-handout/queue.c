/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  malloc(sizeof(queue_t));
    /* What if malloc returned NULL? */
    if (q == NULL) {
      return NULL;
    }
    q->head = NULL;  /* sentinel code */
    q->size = 0;
    q->tail = NULL;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    /* How about freeing the list elements? */
    /* Free queue structure */
    if (q == NULL) {
      return;
    }
    list_ele_t *temp = q->head;
    if (temp == NULL) {
      free(q);
      return;
    }

    while (temp != NULL) {
      list_ele_t *original = temp;
      list_ele_t *next = original->next;
      temp = next;
      free(original);
    }
    free(q);

}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
*/
bool q_insert_head(queue_t *q, int v)
{
    list_ele_t *newh;
    if (q == NULL) {
      return false;
    }
    /* What should you do if the q is NULL? */
    newh = malloc(sizeof(list_ele_t));
    if (newh == NULL) {
      return false;
    }
    /* What if malloc returned NULL? */
    newh->value = v;
    if (q->head == NULL) {
      q->head = newh;
      newh->next = NULL;
      q->tail = q->head;
    }
    else {
      newh->next = q->head;
      q->head = newh;
    }
    q->size++;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_tail(queue_t *q, int v)
{
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */
    if (q == NULL) {
      return false;
    }
    list_ele_t *pointer = malloc(sizeof(list_ele_t));
    if (pointer == NULL) {
      return false;
    }
    q->tail->next = pointer;
    pointer->value = v;
    pointer->next = NULL;  // important stuff!
    q->tail = q->tail->next;
    q->size++;
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If vp non-NULL and element removed, store removed value at *vp.
  Any unused storage should be freed
*/
bool q_remove_head(queue_t *q, int *vp)
{
    /* You need to fix up this code. */
    if (q == NULL || q->head == NULL) {
      return false;
    }
    list_ele_t *headPointer = q->head;
    int returnValue = headPointer->value;
    if (vp) {
      *vp = returnValue;
    }
    if (q->size == 1) {
      q->tail = NULL;
    }
    q->head = headPointer->next;
    free(headPointer);
    q->size--;
    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
    if (q == NULL) {
      return 0;
    }
    return q->size;
}

/*
  Reverse elements in queue.

  Your implementation must not allocate or free any elements (e.g., by
  calling q_insert_head or q_remove_head).  Instead, it should modify
  the pointers in the existing data structure.
 */
void q_reverse(queue_t *q)
{
    if (q == NULL) {
      return;
    }
    list_ele_t *start = q->head;
    if (start == NULL || q->size <= 1) {
      return;
    }
    list_ele_t *middle = start->next;
    if (q->size == 2) {
      middle->next = start;
      start->next = NULL;
      q->tail = start;
      q->head = middle;
      return;
    }

    q->tail = start;
    list_ele_t *end = middle->next;
    while (middle)
    {
      end = middle->next;
      middle->next = start;
      start = middle;
      middle = end;
    }
    q->head = start;
    q->tail->next = NULL;
    
    /* You need to write the code for this function */
}
