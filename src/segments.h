/***************************************
 $Header: /home/amb/routino/src/RCS/segments.h,v 1.34 2009/11/14 19:39:20 amb Exp $

 A header file for the segments.

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


#ifndef SEGMENTS_H
#define SEGMENTS_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"
#include "profiles.h"


/* Data structures */


/*+ A structure containing a single segment. +*/
struct _Segment
{
 index_t    node1;              /*+ The index of the starting node. +*/
 index_t    node2;              /*+ The index of the finishing node. +*/

 index_t    next2;              /*+ The index of the next segment sharing node2. +*/

 index_t    way;                /*+ The index of the way associated with the segment. +*/

 distance_t distance;           /*+ The distance between the nodes. +*/
};


/*+ A structure containing a set of segments (mmap format). +*/
struct _Segments
{
 uint32_t  number;              /*+ How many segments in total? +*/
 uint32_t  snumber;             /*+ How many super-segments? +*/
 uint32_t  nnumber;             /*+ How many normal segments? +*/

 Segment  *segments;            /*+ An array of segments. +*/

 void     *data;                /*+ The memory mapped data. +*/
};


/* Macros */


/*+ Return a segment pointer given a set of segments and an index. +*/
#define LookupSegment(xxx,yyy) (&(xxx)->segments[yyy])

/*+ Return a segment index given a set of segments and a pointer. +*/
#define IndexSegment(xxx,yyy)  ((yyy)-&(xxx)->segments[0])

/*+ Return true if this is a normal segment. +*/
#define IsNormalSegment(xxx)   (((xxx)->distance)&SEGMENT_NORMAL)

/*+ Return true if this is a super-segment. +*/
#define IsSuperSegment(xxx)    (((xxx)->distance)&SEGMENT_SUPER)

/*+ Return true if the segment is oneway towards the specified node. +*/
#define IsOnewayTo(xxx,yyy)    ((xxx)->node1==(yyy)?((xxx)->distance&ONEWAY_2TO1):((xxx)->distance&ONEWAY_1TO2))

/*+ Return true if the segment is oneway from the specified node. +*/
#define IsOnewayFrom(xxx,yyy)  ((xxx)->node2==(yyy)?((xxx)->distance&ONEWAY_2TO1):((xxx)->distance&ONEWAY_1TO2))

/*+ Return the other node in the segment that is not the specified node. +*/
#define OtherNode(xxx,yyy)     ((xxx)->node1==(yyy)?(xxx)->node2:(xxx)->node1)


/* Functions */


Segments *LoadSegmentList(const char *filename);

Segment *NextSegment(Segments* segments,Segment *segment,index_t node);

distance_t Distance(double lat1,double lon1,double lat2,double lon2);

duration_t Duration(Segment *segment,Way *way,Profile *profile);


#endif /* SEGMENTS_H */
