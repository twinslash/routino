/***************************************
 $Header: /home/amb/routino/src/RCS/segmentsx.h,v 1.20 2010/03/19 19:47:09 amb Exp $

 A header file for the extended segments.

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


#ifndef SEGMENTSX_H
#define SEGMENTSX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "typesx.h"
#include "types.h"


/* Data structures */


/*+ An extended structure used for processing. +*/
struct _SegmentX
{
 node_t     node1;              /*+ The id of the starting node. +*/
 node_t     node2;              /*+ The id of the finishing node. +*/

 way_t      way;                /*+ The id of the way. +*/

 distance_t distance;           /*+ The distance between the nodes. +*/
};


/*+ A structure containing a set of segments (memory format). +*/
struct _SegmentsX
{
 char      *filename;           /*+ The name of the temporary file. +*/
 int        fd;                 /*+ The file descriptor of the temporary file. +*/

 uint32_t   xnumber;            /*+ The number of unsorted extended nodes. +*/

 SegmentX  *xdata;              /*+ The extended segment data (unsorted). +*/
 SegmentX   cached[2];          /*+ Two cached segments read from the file in slim mode. +*/

 uint32_t   number;             /*+ How many entries are still useful? +*/

 node_t   *idata;               /*+ The extended segment data (sorted by node1 then node2). +*/
 index_t  *firstnode;           /*+ The first segment index for each node. +*/

 Segment   *sdata;              /*+ The segment data (same order as n1data). +*/
};


/* Functions */


SegmentsX *NewSegmentList(int append);
void FreeSegmentList(SegmentsX *segmentsx,int keep);

void SaveSegmentList(SegmentsX *segmentsx,const char *filename);

SegmentX *LookupSegmentX(SegmentsX* segmentsx,index_t index,int position);

index_t IndexFirstSegmentX(SegmentsX* segmentsx,node_t node);

index_t IndexNextSegmentX(SegmentsX* segmentsx,index_t segindex,index_t nodeindex);

void AppendSegment(SegmentsX* segmentsx,way_t way,node_t node1,node_t node2,distance_t distance);

void SortSegmentList(SegmentsX* segmentsx);

void RemoveBadSegments(NodesX *nodesx,SegmentsX *segmentsx);

void UpdateSegments(SegmentsX *segmentsx,NodesX *nodesx,WaysX *waysx);

void RotateSegments(SegmentsX* segmentsx);

void DeduplicateSegments(SegmentsX* segmentsx,NodesX *nodesx,WaysX *waysx);

void CreateRealSegments(SegmentsX *segmentsx,WaysX *waysx);

void IndexSegments(SegmentsX* segmentsx,NodesX *nodesx);


#endif /* SEGMENTSX_H */
