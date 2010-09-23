/***************************************
 $Header: /home/amb/routino/src/RCS/waysx.h,v 1.21 2010/05/22 18:40:47 amb Exp $

 A header file for the extended Ways structure.

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


#ifndef WAYSX_H
#define WAYSX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "typesx.h"
#include "types.h"
#include "ways.h"


/* Data structures */


/*+ An extended structure containing a single way. +*/
struct _WayX
{
 way_t    id;                   /*+ The way identifier. +*/

 index_t  prop;                 /*+ The index of the properties of the way in the compacted list. +*/

 Way      way;                  /*+ The real Way data. +*/
};


/*+ A structure containing a set of ways (memory format). +*/
struct _WaysX
{
 char    *filename;             /*+ The name of the temporary file (for the WaysX). +*/
 int      fd;                   /*+ The file descriptor of the temporary file (for the WaysX). +*/

 uint32_t xnumber;              /*+ The number of unsorted extended ways. +*/

 WayX    *xdata;                /*+ The extended data for the Ways (sorted). +*/
 WayX     cached[2];            /*+ Two cached ways read from the file in slim mode. +*/

 uint32_t number;               /*+ How many entries are still useful? +*/

 uint32_t cnumber;              /*+ How many entries are there after compacting? +*/

 index_t *idata;                /*+ The index of the extended data for the Ways (sorted by ID). +*/

 char    *nfilename;            /*+ The name of the temporary file (for the names). +*/

 uint32_t nlength;              /*+ How long is the string of name entries? +*/
};


/* Functions */


WaysX *NewWayList(int append);
void FreeWayList(WaysX *waysx,int keep);

void SaveWayList(WaysX *waysx,const char *filename);

index_t IndexWayX(WaysX* waysx,way_t id);
WayX *LookupWayX(WaysX* waysx,index_t index,int position);

void AppendWay(WaysX* waysx,way_t id,Way *way,const char *name);

void SortWayList(WaysX *waysx);

#endif /* WAYSX_H */
