/***************************************
 $Header: /home/amb/routino/src/RCS/functions.h,v 1.54 2010/04/24 16:47:56 amb Exp $

 Header file for function prototypes

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


#ifndef FUNCTIONS_H
#define FUNCTIONS_H    /*+ To stop multiple inclusions. +*/

#include <sys/types.h>
#include <stdio.h>

#include "types.h"
#include "profiles.h"
#include "results.h"


/* In router.c */

/*+ Return true if this is a fake node. +*/
#define IsFakeNode(xxx)  ((xxx)&NODE_SUPER)

index_t CreateFakes(Nodes *nodes,int point,Segment *segment,index_t node1,index_t node2,distance_t dist1,distance_t dist2);

void GetFakeLatLong(index_t node, double *latitude,double *longitude);

Segment *FirstFakeSegment(index_t node);
Segment *NextFakeSegment(Segment *segment,index_t node);
Segment *ExtraFakeSegment(index_t node,index_t fakenode);


/* In files.c */

char *FileName(const char *dirname,const char *prefix, const char *name);

void *MapFile(const char *filename);
void *UnmapFile(const char *filename);

int OpenFile(const char *filename);
int AppendFile(const char *filename);
int ReOpenFile(const char *filename);

int WriteFile(int fd,const void *address,size_t length);
int ReadFile(int fd,void *address,size_t length);

off_t SizeFile(const char *filename);
int ExistsFile(const char *filename);

int SeekFile(int fd,off_t position);

void CloseFile(int fd);

int DeleteFile(char *filename);


/* In optimiser.c */

Results *FindNormalRoute(Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile);
Results *FindMiddleRoute(Nodes *supernodes,Segments *supersegments,Ways *superways,Results *begin,Results *end,Profile *profile);

Results *FindStartRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t start,Profile *profile);
Results *FindFinishRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t finish,Profile *profile);

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile);

void FixForwardRoute(Results *results,index_t finish);


/* In output.c */

void PrintRoute(Results **results,int nresults,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile);


/* In sorting.c */

/*+ The type, size and alignment of variable to store the variable length +*/
#define FILESORT_VARINT   unsigned short
#define FILESORT_VARSIZE  sizeof(FILESORT_VARINT)
#define FILESORT_VARALIGN sizeof(void*)

void filesort_fixed(int fd_in,int fd_out,size_t itemsize,int (*compare)(const void*,const void*),int (*buildindex)(void*,index_t));

void filesort_vary(int fd_in,int fd_out,int (*compare)(const void*,const void*),int (*buildindex)(void*,index_t));

void heapsort(void **datap,size_t nitems,int(*compare)(const void*, const void*));


#endif /* FUNCTIONS_H */
