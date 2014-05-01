#include <stdio.h>
#include <pthread.h>
#include "zmalloc.h"
#include "array.h"
void arrayPush(ARRAY *arr,ANODE *node,pthread_mutex_t *lock)
{
	if(lock != NULL) {
		pthread_mutex_lock(lock);
	}

    node->next = NULL;
    if(arr->end == NULL) {
        arr->head = node;
        node->prev = NULL;
    } else {
        arr->end->next = node;
        node->prev = arr->end;
    }
    arr->end = node;
    arr->length++;
	if(lock != NULL) {
		pthread_mutex_unlock(lock);
	}
}

ANODE *arrayPop(ARRAY *arr,pthread_mutex_t *lock)
{
    ANODE *a = NULL;

	if(lock != NULL) {
		pthread_mutex_lock(lock);
	}

    if(arr->length > 0) {
        a = arr->end;
        arr->end = arr->end->prev;
        if(arr->end) {
            arr->end->next = NULL;
        } else {
            arr->head = NULL;
        }
        arr->length--;
    }	

	if(lock != NULL) {
		pthread_mutex_unlock(lock);
	}
    return a;
}

void arrayUnshift(ARRAY *arr,ANODE *node,pthread_mutex_t *lock)
{
	if(lock != NULL) {
		pthread_mutex_lock(lock);
	}

    node->next = arr->head;
    node->prev = NULL;
    if(arr->head == NULL) {
        arr->head = arr->end = node;
    } else {
        arr->head->prev = node;
        arr->head = node;
    }
    arr->length++;

	if(lock != NULL) {
		pthread_mutex_unlock(lock);
	}
}

ANODE *arrayShift(ARRAY *arr,pthread_mutex_t *lock)
{
    ANODE *a = NULL;
	if(lock != NULL) {
		pthread_mutex_lock(lock);
	}
    if(arr->length > 0) {
        a = arr->head;
        arr->head = a->next;
        if(arr->head == NULL) {
            arr->end = NULL;
        } else {
            arr->head->prev = NULL;
        }
        arr->length--;
    }
	
	if(lock != NULL) {
		pthread_mutex_unlock(lock);
	}
    return a;
}

ARRAY * arrayCreate()
{
	ARRAY *arr = zmalloc(sizeof(ARRAY));
	arr->length = 0;
	arr->head = arr->end = NULL;
	return arr;
}

ANODE *arrayNodeCreate(int val,void *data)
{
	ANODE *n= zmalloc(sizeof(struct array_node));    
	n->val = val;
	n->data = data;
	return n;
}

/*
	typedef struct t{
		int c;
	} T;
	struct array_node *tmp(int i)
	{
		ANODE *n= (struct array_node *)malloc(sizeof(struct array_node));
		T *t = (T*)malloc(sizeof(T));
    t->c = i;
    n->data =t;
    n->val = i;
    return n;
}

int main()
{
    ARRAY *arr = arrayCreate();
    arrayPush(arr,tmp(1));
    arrayPush(arr,tmp(2));
    arrayPush(arr,tmp(3));
    arrayPush(arr,tmp(4));
    ANODE *n;
    T *t;
    int i = 4;
    while(i) {

        n = arrayShift(arr);
        if(n == NULL) break;
        t = (T*)n->data;
        printf("%d\n",t->c);
    }

    return 0;
}*/

