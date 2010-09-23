/***************************************
 $Header: /home/amb/routino/src/RCS/waysx.c,v 1.38 2010/05/22 18:40:47 amb Exp $

 Extended Way data type functions.

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
#include <string.h>
#include <sys/stat.h>

#include "functions.h"
#include "waysx.h"
#include "ways.h"


/* Variables */

/*+ The command line '--slim' option. +*/
extern int option_slim;

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;

/*+ A temporary file-local variable for use by the sort functions. +*/
static WaysX *sortwaysx;

/* Functions */

static int sort_by_name_and_prop_and_id(WayX *a,WayX *b);
static int deduplicate_by_id(WayX *wayx,index_t index);

static int sort_by_id(WayX *a,WayX *b);
static int index_by_id(WayX *wayx,index_t index);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new way list (create a new file or open an existing one).

  WaysX *NewWayList Returns the way list.

  int append Set to 1 if the file is to be opened for appending (now or later).
  ++++++++++++++++++++++++++++++++++++++*/

WaysX *NewWayList(int append)
{
 WaysX *waysx;

 waysx=(WaysX*)calloc(1,sizeof(WaysX));

 assert(waysx); /* Check calloc() worked */

 waysx->filename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(waysx->filename,"%s/ways.input.tmp",option_tmpdirname);
 else
    sprintf(waysx->filename,"%s/ways.%p.tmp",option_tmpdirname,waysx);

 if(append)
   {
    off_t size,position=0;

    waysx->fd=AppendFile(waysx->filename);

    size=SizeFile(waysx->filename);

    while(position<size)
      {
       FILESORT_VARINT waysize;

       SeekFile(waysx->fd,position);
       ReadFile(waysx->fd,&waysize,FILESORT_VARSIZE);

       waysx->xnumber++;
       position+=waysize+FILESORT_VARSIZE;
      }

    SeekFile(waysx->fd,size);
   }
 else
    waysx->fd=OpenFile(waysx->filename);

 waysx->nfilename=(char*)malloc(strlen(option_tmpdirname)+32);
 sprintf(waysx->nfilename,"%s/waynames.%p.tmp",option_tmpdirname,waysx);

 return(waysx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a way list.

  WaysX *waysx The list to be freed.

  int keep Set to 1 if the file is to be kept.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeWayList(WaysX *waysx,int keep)
{
 if(!keep)
    DeleteFile(waysx->filename);

 free(waysx->filename);

 if(waysx->idata)
    free(waysx->idata);

 DeleteFile(waysx->nfilename);

 free(waysx->nfilename);

 free(waysx);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a way to a way list.

  void AppendWay Returns the newly appended way.

  WaysX* waysx The set of ways to process.

  way_t id The ID of the way.

  Way *way The way data itself.

  const char *name The name or reference of the way.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendWay(WaysX* waysx,way_t id,Way *way,const char *name)
{
 WayX wayx;
 FILESORT_VARINT size;

 assert(!waysx->idata);       /* Must not have idata filled in => unsorted */

 wayx.id=id;
 wayx.prop=0;
 wayx.way=*way;

 size=sizeof(WayX)+strlen(name)+1;

 WriteFile(waysx->fd,&size,FILESORT_VARSIZE);
 WriteFile(waysx->fd,&wayx,sizeof(WayX));
 WriteFile(waysx->fd,name,strlen(name)+1);

 waysx->xnumber++;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of ways.

  WaysX* waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortWayList(WaysX* waysx)
{
 index_t i;
 int fd,nfd;
 char *names[2]={NULL,NULL};
 int namelen[2]={0,0};
 int nnames=0,nprops=0;
 uint32_t lastlength=0;
 Way lastway;

 /* Check the start conditions */

 assert(!waysx->idata);         /* Must not have idata filled in => unsorted */

 /* Print the start message */

 printf("Sorting Ways");
 fflush(stdout);

 /* Close the file and re-open it (finished appending) */

 CloseFile(waysx->fd);
 waysx->fd=ReOpenFile(waysx->filename);

 DeleteFile(waysx->filename);

 fd=OpenFile(waysx->filename);

 /* Sort the ways to allow compacting them and remove duplicates */

 sortwaysx=waysx;

 filesort_vary(waysx->fd,fd,(int (*)(const void*,const void*))sort_by_name_and_prop_and_id,(int (*)(void*,index_t))deduplicate_by_id);

 /* Close the files */

 CloseFile(waysx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf("\rSorted Ways: Ways=%d Duplicates=%d\n",waysx->xnumber,waysx->xnumber-waysx->number);
 fflush(stdout);


 /* Print the start message */

 printf("Compacting Ways: Ways=0 Names=0 Properties=0");
 fflush(stdout);

 /* Open the files */

 waysx->fd=ReOpenFile(waysx->filename);

 DeleteFile(waysx->filename);

 fd=OpenFile(waysx->filename);
 nfd=OpenFile(waysx->nfilename);

 /* Copy from the single file into two files and index as we go. */

 for(i=0;i<waysx->number;i++)
   {
    WayX wayx;
    FILESORT_VARINT size;

    ReadFile(waysx->fd,&size,FILESORT_VARSIZE);

    if(namelen[nnames%2]<size)
       names[nnames%2]=(char*)realloc((void*)names[nnames%2],namelen[nnames%2]=size);

    ReadFile(waysx->fd,&wayx,sizeof(WayX));
    ReadFile(waysx->fd,names[nnames%2],size-sizeof(WayX));

    if(nnames==0 || strcmp(names[0],names[1]))
      {
       WriteFile(nfd,names[nnames%2],size-sizeof(WayX));

       lastlength=waysx->nlength;
       waysx->nlength+=size-sizeof(WayX);

       nnames++;
      }

    wayx.way.name=lastlength;

    if(nprops==0 || wayx.way.name!=lastway.name || WaysCompare(&lastway,&wayx.way))
      {
       lastway=wayx.way;

       waysx->cnumber++;

       nprops++;
      }

    wayx.prop=nprops-1;

    WriteFile(fd,&wayx,sizeof(WayX));

    if(!((i+1)%10000))
      {
       printf("\rCompacting Ways: Ways=%d Names=%d Properties=%d",i+1,nnames,nprops);
       fflush(stdout);
      }
   }

 if(names[0]) free(names[0]);
 if(names[1]) free(names[1]);

 /* Close the files */

 CloseFile(waysx->fd);
 CloseFile(fd);

 waysx->fd=ReOpenFile(waysx->filename);

 CloseFile(nfd);

 /* Print the final message */

 printf("\rCompacted Ways: Ways=%d Names=%d Properties=%d \n",waysx->number,nnames,nprops);
 fflush(stdout);


 /* Print the start message */

 printf("Sorting Ways");
 fflush(stdout);

 /* Open the files */

 waysx->fd=ReOpenFile(waysx->filename);

 DeleteFile(waysx->filename);

 fd=OpenFile(waysx->filename);

 /* Allocate the array of indexes */

 waysx->idata=(way_t*)malloc(waysx->number*sizeof(way_t));

 assert(waysx->idata); /* Check malloc() worked */

 /* Sort the ways by index and index them */

 sortwaysx=waysx;

 filesort_fixed(waysx->fd,fd,sizeof(WayX),(int (*)(const void*,const void*))sort_by_id,(int (*)(void*,index_t))index_by_id);

 /* Close the files and re-open them */

 CloseFile(waysx->fd);
 CloseFile(fd);

 waysx->fd=ReOpenFile(waysx->filename);

 /* Print the final message */

 printf("\rSorted Ways: Ways=%d\n",waysx->number);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into id order.

  int sort_by_id Returns the comparison of the id fields.

  WayX *a The first extended way.

  WayX *b The second extended way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(WayX *a,WayX *b)
{
 way_t a_id=a->id;
 way_t b_id=b->id;

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else
    return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into name, properties and id order.

  int sort_by_name_and_prop_and_id Returns the comparison of the name, properties and id fields.

  WayX *a The first extended Way.

  WayX *b The second extended Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_name_and_prop_and_id(WayX *a,WayX *b)
{
 int compare;
 char *a_name=(char*)a+sizeof(WayX);
 char *b_name=(char*)b+sizeof(WayX);

 compare=strcmp(a_name,b_name);

 if(compare)
    return(compare);

 compare=WaysCompare(&a->way,&b->way);

 if(compare)
    return(compare);

 return(sort_by_id(a,b));
}


/*++++++++++++++++++++++++++++++++++++++
  Deduplicate the extended ways using the id after sorting.

  int deduplicate_by_id Return 1 if the value is to be kept, otherwise zero.

  WayX *wayx The extended way.

  index_t index The index of this way in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int deduplicate_by_id(WayX *wayx,index_t index)
{
 static way_t previd;

 if(index==0 || wayx->id!=previd)
   {
    previd=wayx->id;

    sortwaysx->number++;

    return(1);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Index the ways after sorting.

  int index_by_id Return 1 if the value is to be kept, otherwise zero.

  WayX *wayx The extended way.

  index_t index The index of this way in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int index_by_id(WayX *wayx,index_t index)
{
 sortwaysx->idata[index]=wayx->id;

 return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular way index.

  index_t IndexWayX Returns the index of the extended way with the specified id.

  WaysX* waysx The set of ways to process.

  way_t id The way id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexWayX(WaysX* waysx,way_t id)
{
 int start=0;
 int end=waysx->number-1;
 int mid;

 assert(waysx->idata);         /* Must have idata filled in => sorted */

 /* Binary search - search key exact match only is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 if(end<start)                   /* There are no ways */
    return(NO_WAY);
 else if(id<waysx->idata[start]) /* Check key is not before start */
    return(NO_WAY);
 else if(id>waysx->idata[end])   /* Check key is not after end */
    return(NO_WAY);
 else
   {
    do
      {
       mid=(start+end)/2;            /* Choose mid point */

       if(waysx->idata[mid]<id)      /* Mid point is too low */
          start=mid+1;
       else if(waysx->idata[mid]>id) /* Mid point is too high */
          end=mid-1;
       else                          /* Mid point is correct */
          return(mid);
      }
    while((end-start)>1);

    if(waysx->idata[start]==id)      /* Start is correct */
       return(start);

    if(waysx->idata[end]==id)        /* End is correct */
       return(end);
   }

 return(NO_WAY);
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup a particular way.

  WayX *LookupWayX Returns a pointer to the extended way with the specified id.

  WaysX* waysx The set of ways to process.

  index_t index The way index to look for.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

WayX *LookupWayX(WaysX* waysx,index_t index,int position)
{
 assert(index!=NO_WAY);     /* Must be a valid way */

 if(option_slim)
   {
    SeekFile(waysx->fd,index*sizeof(WayX));

    ReadFile(waysx->fd,&waysx->cached[position-1],sizeof(WayX));

    return(&waysx->cached[position-1]);
   }
 else
   {
    return(&waysx->xdata[index]);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Save the way list to a file.

  WaysX* waysx The set of ways to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveWayList(WaysX* waysx,const char *filename)
{
 index_t i;
 int fd,nfd;
 int position=0;
 Ways *ways;

 printf("Writing Ways: Ways=0");
 fflush(stdout);

 if(!option_slim)
    waysx->xdata=MapFile(waysx->filename);

 /* Fill in a Ways structure with the offset of the real data in the file after
    the Way structure itself. */

 ways=calloc(1,sizeof(Ways));

 assert(ways); /* Check calloc() worked */

 ways->number=waysx->cnumber;
 ways->onumber=waysx->number;

 ways->allow=0;
 ways->props=0;

 ways->data=NULL;
 ways->ways=NULL;
 ways->names=NULL;

 /* Write out the Ways structure and then the real data. */

 fd=OpenFile(filename);

 for(i=0;i<waysx->number;i++)
   {
    WayX *wayx=LookupWayX(waysx,i,1);

    ways->allow|=wayx->way.allow;
    ways->props|=wayx->way.props;

    SeekFile(fd,sizeof(Ways)+wayx->prop*sizeof(Way));
    WriteFile(fd,&wayx->way,sizeof(Way));

    if(!((i+1)%10000))
      {
       printf("\rWriting Ways: Ways=%d",i+1);
       fflush(stdout);
      }
   }

 SeekFile(fd,0);
 WriteFile(fd,ways,sizeof(Ways));

 if(!option_slim)
    waysx->xdata=UnmapFile(waysx->filename);

 SeekFile(fd,sizeof(Ways)+ways->number*sizeof(Way));

 nfd=ReOpenFile(waysx->nfilename);

 while(position<waysx->nlength)
   {
    int len=1024;
    char temp[1024];

    if((waysx->nlength-position)<1024)
       len=waysx->nlength-position;

    ReadFile(nfd,temp,len);
    WriteFile(fd,temp,len);

    position+=len;
   }

 CloseFile(nfd);

 CloseFile(fd);

 printf("\rWrote Ways: Ways=%d  \n",waysx->number);
 fflush(stdout);

 /* Free the fake Ways */

 free(ways);
}
