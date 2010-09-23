/***************************************
 $Header: /home/amb/routino/src/RCS/nodesx.h,v 1.23 2010/03/19 19:47:09 amb Exp $

 A header file for the extended nodes.

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


#ifndef NODESX_H
#define NODESX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "typesx.h"
#include "types.h"


/* Data structures */


/*+ An extended structure used for processing. +*/
struct _NodeX
{
 node_t    id;                  /*+ The node identifier. +*/

 latlong_t latitude;            /*+ The node latitude. +*/
 latlong_t longitude;           /*+ The node longitude. +*/
};

/*+ A structure containing a set of nodes (memory format). +*/
struct _NodesX
{
 char     *filename;            /*+ The name of the temporary file. +*/
 int       fd;                  /*+ The file descriptor of the temporary file. +*/

 uint32_t  xnumber;             /*+ The number of unsorted extended nodes. +*/

 NodeX    *xdata;               /*+ The extended node data (sorted). +*/
 NodeX     cached[2];           /*+ Two cached nodes read from the file in slim mode. +*/

 uint32_t  number;              /*+ How many entries are still useful? +*/

 node_t   *idata;               /*+ The extended node IDs (sorted by ID). +*/

 uint8_t  *super;               /*+ A marker for super nodes (same order sorted nodes). +*/

 Node     *ndata;               /*+ The actual nodes (same order as geographically sorted nodes). +*/

 uint32_t  latbins;             /*+ The number of bins containing latitude. +*/
 uint32_t  lonbins;             /*+ The number of bins containing longitude. +*/

 ll_bin_t  latzero;             /*+ The bin number of the furthest south bin. +*/
 ll_bin_t  lonzero;             /*+ The bin number of the furthest west bin. +*/

 uint32_t  latlonbin;           /*+ A temporary index into the offsets array. +*/

 index_t  *offsets;             /*+ An array of offset to the first node in each bin. +*/
};


/* Functions */

NodesX *NewNodeList(int append);
void FreeNodeList(NodesX *nodesx,int keep);

void SaveNodeList(NodesX *nodesx,const char *filename);

index_t IndexNodeX(NodesX* nodesx,node_t id);
NodeX *LookupNodeX(NodesX* nodesx,index_t index,int position);

void AppendNode(NodesX* nodesx,node_t id,double latitude,double longitude);

void SortNodeList(NodesX *nodesx);

void SortNodeListGeographically(NodesX* nodesx);

void RemoveNonHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx);

void CreateRealNodes(NodesX *nodesx,int iteration);

void IndexNodes(NodesX *nodesx,SegmentsX *segmentsx);


#endif /* NODESX_H */
