#include <stdlib.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0

typedef struct
{
    int ID;
    int type;
    int reqTime;
    int prioritize;
    int taskArr;
    // you might want to add variables here!
} Task;

/* a link in the queue, holds the data and point to the next Node */
typedef struct Node_t
{
    Task data;
    struct Node_t *prev;
} NODE;

/* the HEAD of the Queue, hold the amount of node's that are in the queue */
typedef struct Queue
{
    NODE *head;
    NODE *tail;
    int size;
    int limit;
} Queue;

Queue *ConstructQueue(int limit);
void DestructQueue(Queue *queue);
int Enqueue(Queue *pQueue, Task t);
Task Dequeue(Queue *pQueue);
int isEmpty(Queue *pQueue);
int Contains(Queue *pQueue, int id);

Queue *ConstructQueue(int limit)
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    if (queue == NULL)
    {
        return NULL;
    }
    if (limit <= 0)
    {
        limit = 65535;
    }
    queue->limit = limit;
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void DestructQueue(Queue *queue)
{
    NODE *pN;
    while (!isEmpty(queue))
    {
        Dequeue(queue);
    }
    free(queue);
}

int Enqueue(Queue *pQueue, Task t)
{
    /* Bad parameter */
    NODE *item = (NODE *)malloc(sizeof(NODE));
    NODE *oldFront;
    item->data = t;

    if ((pQueue == NULL) || (item == NULL))
    {
        return FALSE;
    }
    // if(pQueue->limit != 0)
    if (pQueue->size >= pQueue->limit)
    {
        return FALSE;
    }
    /*the queue is empty*/
    item->prev = NULL;
    if (pQueue->size != 0 && t.prioritize == 1)
    {
        item->prev = pQueue->head;
        pQueue->head = item;
        // pQueue->head->prev = oldFront;
        /*printf("\n\n");
        printf("\nNEW ZEALANDER ADDED TO FRONT OF THE QUEUE!\n");
        NODE *iterate = pQueue->head;

        while (iterate != NULL)
        {
            printf("queue data: %d, queue type: %d\n", iterate->data.ID, iterate->data.type);
            iterate = iterate->prev;
        }
        printf("\n\n");*/
    }
    else if (pQueue->size == 0)
    {
        pQueue->head = item;
        pQueue->tail = item;
    }
    else
    {
        /*adding item to the end of the queue*/
        pQueue->tail->prev = item;
        pQueue->tail = item;
    }
    pQueue->size++;

    return TRUE;
}

Task Dequeue(Queue *pQueue)
{
    /*the queue is empty or bad param*/
    NODE *item;
    Task ret;
    if (isEmpty(pQueue))
        return ret;
    item = pQueue->head;
    pQueue->head = (pQueue->head)->prev;
    pQueue->size--;
    ret = item->data;
    free(item);
    return ret;
}

Task Peek(Queue *pQueue)
{
    /*the queue is empty or bad param*/
    Task ret;
    if (isEmpty(pQueue))
        return ret;
    return pQueue->head->data;
}

int isEmpty(Queue *pQueue)
{
    if (pQueue == NULL)
    {
        return FALSE;
    }
    if (pQueue->size == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int Contains(Queue *pQueue, int id)
{
    NODE *item;
    Task t;
    if (isEmpty(pQueue))
    {
        return FALSE;
        // printf("BOŞ");
    }
    item = pQueue->head;
    while (item != NULL)
    {
        t = item->data;
        if (t.ID == id)
        {
            return TRUE;
        }
        item = item->prev;
    }
    return FALSE;
}

int Size(Queue *pQueue)
{
    NODE *item;
    int counter = 0;
    if (isEmpty(pQueue))
    {
        counter;
    }
    item = pQueue->head;
    while (item != NULL)
    {
        counter = counter + 1;
        item = item->prev;
    }
    return counter;
}

int ContainsNZ(Queue *pQueue)
{
    NODE *item;
    Task t;
    if (isEmpty(pQueue))
    {
        return FALSE;
        // printf("BOŞ");
    }
    item = pQueue->head;
    while (item != NULL)
    {
        t = item->data;
        if (t.prioritize == 1)
        {
            return TRUE;
        }
        item = item->prev;
    }
    return FALSE;
}