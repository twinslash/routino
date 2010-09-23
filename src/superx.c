/***************************************
 $Header: /home/amb/routino/src/RCS/superx.c,v 1.38 2010/07/05 19:05:51 amb Exp $

 Super-Segment data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2010 Andrew M. Bishop

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


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "results.h"
#include "functions.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "superx.h"
#include "ways.h"


/* Variables */

/*+ The command line '--slim' option. +*/
extern int option_slim;

/* Local Functions */

static Results *FindRoutesWay(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,node_t start,Way *match,int iteration);


/*++++++++++++++++++++++++++++++++++++++
  Select the super-segments from the list of segments.

  NodesX *nodesx The nodes.

  SegmentsX *segmentsx The segments.

  WaysX *waysx The ways.
  ++++++++++++++++++++++++++++++++++++++*/

void ChooseSuperNodes(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx)
{
 index_t i;
 int nnodes=0;

 /* Check the start conditions */

 assert(segmentsx->firstnode); /* Must have firstnode filled in => segments are updated */

 /* Print the start message */

 printf("Finding Super-Nodes: Nodes=0 Super-Nodes=0");
 fflush(stdout);

 /* Map into memory */

 if(!option_slim)
   {
    segmentsx->xdata=MapFile(segmentsx->filename);
    waysx->xdata=MapFile(waysx->filename);
   }

 /* Find super-nodes */

 for(i=0;i<nodesx->number;i++)
   {
    int difference=0;
    index_t index1,index2;

    index1=IndexFirstSegmentX(segmentsx,i);

    while(index1!=NO_SEGMENT)
      {
       SegmentX *segmentx1=LookupSegmentX(segmentsx,index1,1);
       WayX *wayx1=LookupWayX(waysx,segmentx1->way,1);

       index1=IndexNextSegmentX(segmentsx,index1,i);
       index2=index1;

       while(index2!=NO_SEGMENT)
         {
          SegmentX *segmentx2=LookupSegmentX(segmentsx,index2,2);
          WayX *wayx2=LookupWayX(waysx,segmentx2->way,2);

          /* If the ways are different in any way and there is a type of traffic that can use both ... */

          if(WaysCompare(&wayx1->way,&wayx2->way))
             if(wayx1->way.allow & wayx2->way.allow)
               {
                difference=1;
                break;
               }

          index2=IndexNextSegmentX(segmentsx,index2,i);
         }

       if(difference)
          break;
      }

    /* Store the node if there is a difference in the connected ways that could affect routing. */

    if(difference)
      {
       nodesx->super[i]++;

       nnodes++;
      }

    if(!((i+1)%10000))
      {
       printf("\rFinding Super-Nodes: Nodes=%d Super-Nodes=%d",i+1,nnodes);
       fflush(stdout);
      }
   }

 /* Unmap from memory */

 if(!option_slim)
   {
    segmentsx->xdata=UnmapFile(segmentsx->filename);
    waysx->xdata=UnmapFile(waysx->filename);
   }

 /* Print the final message */

 printf("\rFound Super-Nodes: Nodes=%d Super-Nodes=%d  \n",nodesx->number,nnodes);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Create the super-segments.

  SegmentsX *CreateSuperSegments Creates the super segments.

  NodesX *nodesx The nodes.

  SegmentsX *segmentsx The segments.

  WaysX *waysx The ways.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *CreateSuperSegments(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,int iteration)
{
 index_t i;
 SegmentsX *supersegmentsx;
 int sn=0,ss=0;

 /* Check the start conditions */

 assert(segmentsx->firstnode); /* Must have firstnode filled in => segments are updated */

 /* Print the start message */

 printf("Creating Super-Segments: Super-Nodes=0 Super-Segments=0");
 fflush(stdout);

 /* Map into memory */

 if(!option_slim)
   {
    segmentsx->xdata=MapFile(segmentsx->filename);
    waysx->xdata=MapFile(waysx->filename);
   }

 /* Create super-segments for each super-node. */

 supersegmentsx=NewSegmentList(0);

 for(i=0;i<nodesx->number;i++)
   {
    if(nodesx->super[i]>iteration)
      {
       index_t index,first;

       index=first=IndexFirstSegmentX(segmentsx,i);

       while(index!=NO_SEGMENT)
         {
          SegmentX *segmentx=LookupSegmentX(segmentsx,index,1);
          WayX *wayx=LookupWayX(waysx,segmentx->way,1);

          /* Check that this type of way hasn't already been routed */

          if(index!=first)
            {
             index_t otherindex=first;

             while(otherindex!=NO_SEGMENT && otherindex!=index)
               {
                SegmentX *othersegmentx=LookupSegmentX(segmentsx,otherindex,2);
                WayX *otherwayx=LookupWayX(waysx,othersegmentx->way,2);

                if(!WaysCompare(&otherwayx->way,&wayx->way))
                  {
                   wayx=NULL;
                   break;
                  }

                otherindex=IndexNextSegmentX(segmentsx,otherindex,i);
               }
            }

          /* Route the way and store the super-segments. */

          if(wayx)
            {
             Results *results=FindRoutesWay(nodesx,segmentsx,waysx,i,&wayx->way,iteration);
             Result *result=FirstResult(results);

             while(result)
               {
                if(result->node!=i && nodesx->super[result->node]>iteration)
                  {
                   if(wayx->way.type&Way_OneWay)
                     {
                      AppendSegment(supersegmentsx,segmentx->way,i,result->node,DISTANCE((distance_t)result->score)|ONEWAY_1TO2);
                      AppendSegment(supersegmentsx,segmentx->way,result->node,i,DISTANCE((distance_t)result->score)|ONEWAY_2TO1);
                     }
                   else
                      AppendSegment(supersegmentsx,segmentx->way,i,result->node,DISTANCE((distance_t)result->score));

                   ss++;
                  }

                result=NextResult(results,result);
               }

             FreeResultsList(results);
            }

          index=IndexNextSegmentX(segmentsx,index,i);
         }

       sn++;

       if(!(sn%10000))
         {
          printf("\rCreating Super-Segments: Super-Nodes=%d Super-Segments=%d",sn,ss);
          fflush(stdout);
         }
      }
   }

 /* Unmap from memory */

 if(!option_slim)
   {
    segmentsx->xdata=UnmapFile(segmentsx->filename);
    waysx->xdata=UnmapFile(waysx->filename);
   }

 /* Print the final message */

 printf("\rCreated Super-Segments: Super-Nodes=%d Super-Segments=%d \n",sn,ss);
 fflush(stdout);

 return(supersegmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Merge the segments and super-segments into a new segment list.

  SegmentsX* MergeSuperSegments Returns a new set of merged segments.

  SegmentsX* segmentsx The set of segments to process.

  SegmentsX* supersegmentsx The set of super-segments to merge.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *MergeSuperSegments(SegmentsX* segmentsx,SegmentsX* supersegmentsx)
{
 index_t i,j;
 int m=0,a=0;
 SegmentsX* mergedsegmentsx;

 /* Print the start message */

 printf("Merging: Segments=0 Super-Segments=0 Merged=0 Added=0");
 fflush(stdout);

 /* Map into memory */

 if(!option_slim)
   {
    segmentsx->xdata=MapFile(segmentsx->filename);
    supersegmentsx->xdata=MapFile(supersegmentsx->filename);
   }

 /* Loop through and create a new list of combined segments */

 mergedsegmentsx=NewSegmentList(0);

 for(i=0,j=0;i<segmentsx->number;i++)
   {
    int super=0;
    SegmentX *segmentx=LookupSegmentX(segmentsx,i,1);

    while(j<supersegmentsx->number)
      {
       SegmentX *supersegmentx=LookupSegmentX(supersegmentsx,j,1);

       if(segmentx->node1   ==supersegmentx->node1 &&
          segmentx->node2   ==supersegmentx->node2 &&
          segmentx->distance==supersegmentx->distance)
         {
          m++;
          j++;
          /* mark as super-segment and normal segment */
          super=1;
          break;
         }
       else if((segmentx->node1==supersegmentx->node1 &&
                segmentx->node2==supersegmentx->node2) ||
               (segmentx->node1==supersegmentx->node1 &&
                segmentx->node2>supersegmentx->node2) ||
               (segmentx->node1>supersegmentx->node1))
         {
          /* mark as super-segment */
          AppendSegment(mergedsegmentsx,supersegmentx->way,supersegmentx->node1,supersegmentx->node2,supersegmentx->distance|SEGMENT_SUPER);
          a++;
          j++;
         }
       else
         {
          /* mark as normal segment */
          break;
         }
      }

    if(super)
       AppendSegment(mergedsegmentsx,segmentx->way,segmentx->node1,segmentx->node2,segmentx->distance|SEGMENT_SUPER|SEGMENT_NORMAL);
    else
       AppendSegment(mergedsegmentsx,segmentx->way,segmentx->node1,segmentx->node2,segmentx->distance|SEGMENT_NORMAL);

    if(!((i+1)%10000))
      {
       printf("\rMerging: Segments=%d Super-Segments=%d Merged=%d Added=%d",i+1,j,m,a);
       fflush(stdout);
      }
   }

 /* Unmap from memory */

 if(!option_slim)
   {
    segmentsx->xdata=UnmapFile(segmentsx->filename);
    supersegmentsx->xdata=UnmapFile(supersegmentsx->filename);
   }

 /* Print the final message */

 printf("\rMerged: Segments=%d Super-Segments=%d Merged=%d Added=%d \n",segmentsx->number,supersegmentsx->number,m,a);
 fflush(stdout);

 return(mergedsegmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any node in the specified list that follows a certain type of way.

  Results *FindRoutesWay Returns a set of results.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  WaysX *waysx The set of ways to use.

  node_t start The start node.

  Way *match The way that the route must match.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

static Results *FindRoutesWay(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,node_t start,Way *match,int iteration)
{
 Results *results;
 Queue *queue;
 index_t node1,node2;
 Result *result1,*result2;
 index_t index;
 WayX *wayx;

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 queue=NewQueueList();

 result1=InsertResult(results,start);

 ZeroResult(result1);

 InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    node1=result1->node;

    index=IndexFirstSegmentX(segmentsx,node1);

    while(index!=NO_SEGMENT)
      {
       SegmentX *segmentx=LookupSegmentX(segmentsx,index,2); /* must use 2 here */
       distance_t cumulative_distance;

       if(segmentx->distance&ONEWAY_2TO1)
          goto endloop;

       node2=segmentx->node2;

       if(result1->prev==node2)
          goto endloop;

       wayx=LookupWayX(waysx,segmentx->way,2);

       if(WaysCompare(&wayx->way,match))
          goto endloop;

       cumulative_distance=(distance_t)result1->score+DISTANCE(segmentx->distance);

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->prev=node1;
          result2->next=NO_NODE;
          result2->score=cumulative_distance;
          result2->sortby=cumulative_distance;

          if(nodesx->super[node2]<=iteration)
             InsertInQueue(queue,result2);
         }
       else if(cumulative_distance<result2->score)
         {
          result2->prev=node1;
          result2->score=cumulative_distance;
          result2->sortby=cumulative_distance;

          if(nodesx->super[node2]<=iteration)
             InsertInQueue(queue,result2);
         }

      endloop:

       index=IndexNextSegmentX(segmentsx,index,node1);
      }
   }

 FreeQueueList(queue);

 return(results);
}
