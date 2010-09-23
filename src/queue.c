/***************************************
 $Header: /home/amb/routino/src/RCS/queue.c,v 1.7 2009/11/13 19:24:11 amb Exp $

 Queue data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008,2009 Andrew M. Bishop

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************/


#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "results.h"


/*+ A queue of results. +*/
struct _Queue
{
 uint32_t nallocated;           /*+ The number of entries allocated. +*/
 uint32_t noccupied;            /*+ The number of entries occupied. +*/

 Result **data;                 /*+ The queue of pointers to results. +*/
};


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new queue.

  Queue *NewQueueList Returns the queue.
  ++++++++++++++++++++++++++++++++++++++*/

Queue *NewQueueList(void)
{
 Queue *queue;

 queue=(Queue*)malloc(sizeof(Queue));

 queue->nallocated=1023;
 queue->noccupied=0;

 queue->data=(Result**)malloc(queue->nallocated*sizeof(Result*));

 return(queue);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a queue.

  Queue *queue The queue to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeQueueList(Queue *queue)
{
 free(queue->data);

 free(queue);
}


/*++++++++++++++++++++++++++++++++++++++
  Insert a new item into the queue in the right place.

  The data is stored in a "Binary Heap" http://en.wikipedia.org/wiki/Binary_heap
  and this operation is adding an item to the heap.

  Queue *queue The queue to insert the result into.

  Result *result The result to insert into the queue.
  ++++++++++++++++++++++++++++++++++++++*/

void InsertInQueue(Queue *queue,Result *result)
{
 uint32_t index;

 if(result->queued==NOT_QUEUED)
   {
    if(queue->noccupied==queue->nallocated)
      {
       queue->nallocated=2*queue->nallocated+1;
       queue->data=(Result**)realloc((void*)queue->data,queue->nallocated*sizeof(Result*));
      }

    index=queue->noccupied;
    queue->noccupied++;

    queue->data[index]=result;
    queue->data[index]->queued=index;
   }
 else
   {
    index=result->queued;
   }

 /* Bubble up the new value */

 while(index>0 &&
       queue->data[index]->sortby<queue->data[(index-1)/2]->sortby)
   {
    uint32_t newindex;
    Result *temp;

    newindex=(index-1)/2;

    temp=queue->data[index];
    queue->data[index]=queue->data[newindex];
    queue->data[newindex]=temp;

    queue->data[index]->queued=index;
    queue->data[newindex]->queued=newindex;

    index=newindex;
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Pop an item from the front of the queue.

  The data is stored in a "Binary Heap" http://en.wikipedia.org/wiki/Binary_heap
  and this operation is deleting the root item from the heap.

  Result *PopFromQueue Returns the top item.

  Queue *queue The queue to remove the result from.
  ++++++++++++++++++++++++++++++++++++++*/

Result *PopFromQueue(Queue *queue)
{
 uint32_t index;
 Result *retval;

 if(queue->noccupied==0)
    return(NULL);

 retval=queue->data[0];
 retval->queued=NOT_QUEUED;

 index=0;
 queue->noccupied--;

 queue->data[index]=queue->data[queue->noccupied];

 /* Bubble down the newly promoted value */

 while((2*index+2)<queue->noccupied &&
       (queue->data[index]->sortby>queue->data[2*index+1]->sortby ||
        queue->data[index]->sortby>queue->data[2*index+2]->sortby))
   {
    uint32_t newindex;
    Result *temp;

    if(queue->data[2*index+1]->sortby<queue->data[2*index+2]->sortby)
       newindex=2*index+1;
    else
       newindex=2*index+2;

    temp=queue->data[newindex];
    queue->data[newindex]=queue->data[index];
    queue->data[index]=temp;

    queue->data[index]->queued=index;
    queue->data[newindex]->queued=newindex;

    index=newindex;
   }

 if((2*index+2)==queue->noccupied &&
    queue->data[index]->sortby>queue->data[2*index+1]->sortby)
   {
    uint32_t newindex;
    Result *temp;

    newindex=2*index+1;

    temp=queue->data[newindex];
    queue->data[newindex]=queue->data[index];
    queue->data[index]=temp;

    queue->data[index]->queued=index;
    queue->data[newindex]->queued=newindex;
   }

 return(retval);
}
