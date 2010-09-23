/***************************************
 $Header: /home/amb/routino/src/RCS/nodes.h,v 1.30 2009/11/14 19:39:19 amb Exp $

 A header file for the nodes.

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


#ifndef NODES_H
#define NODES_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"
#include "profiles.h"


/* Data structures */


/*+ A structure containing a single node. +*/
struct _Node
{
 index_t    firstseg;           /*+ The index of the first segment. +*/

 ll_off_t   latoffset;          /*+ The node latitude offset within its bin. +*/
 ll_off_t   lonoffset;          /*+ The node longitude offset within its bin. +*/
};


/*+ A structure containing a set of nodes (mmap format). +*/
struct _Nodes
{
 uint32_t number;               /*+ How many nodes in total? +*/
 uint32_t snumber;              /*+ How many super-nodes? +*/

 uint32_t latbins;              /*+ The number of bins containing latitude. +*/
 uint32_t lonbins;              /*+ The number of bins containing longitude. +*/

 ll_bin_t latzero;              /*+ The bin number of the furthest south bin. +*/
 ll_bin_t lonzero;              /*+ The bin number of the furthest west bin. +*/

 index_t *offsets;              /*+ An array of offset to the first node in each bin. +*/

 Node    *nodes;                /*+ An array of nodes. +*/

 void    *data;                 /*+ The memory mapped data. +*/
};


/* Macros */

/*+ Return a Node pointer given a set of nodes and an index. +*/
#define LookupNode(xxx,yyy)        (&(xxx)->nodes[yyy])

/*+ Return a Segment points given a Node pointer and a set of segments. +*/
#define FirstSegment(xxx,yyy,zzz)  LookupSegment((xxx),SEGMENT((yyy)->nodes[zzz].firstseg))

/*+ Return true if this is a super-node. +*/
#define IsSuperNode(xxx,yyy)       (((xxx)->nodes[yyy].firstseg)&NODE_SUPER)


/* Functions */


Nodes *LoadNodeList(const char *filename);

index_t FindClosestNode(Nodes* nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                        distance_t distance,Profile *profile,distance_t *bestdist);

Segment *FindClosestSegment(Nodes* nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                            distance_t distance,Profile *profile, distance_t *bestdist,
                            index_t *bestnode1,index_t *bestnode2,distance_t *bestdist1,distance_t *bestdist2);

void GetLatLong(Nodes *nodes,index_t index,double *latitude,double *longitude);


#endif /* NODES_H */
