/***************************************
 $Header: /home/amb/routino/src/RCS/ways.c,v 1.44 2010/04/28 17:27:02 amb Exp $

 Way data type functions.

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


#include <stdlib.h>

#include "functions.h"
#include "ways.h"


/*++++++++++++++++++++++++++++++++++++++
  Load in a way list from a file.

  Ways* LoadWayList Returns the way list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Ways *LoadWayList(const char *filename)
{
 void *data;
 Ways *ways;

 ways=(Ways*)malloc(sizeof(Ways));

 data=MapFile(filename);

 /* Copy the Ways structure from the loaded data */

 *ways=*((Ways*)data);

 /* Adjust the pointers in the Ways structure. */

 ways->data =data;
 ways->ways =(Way *)(data+sizeof(Ways));
 ways->names=(char*)(data+(sizeof(Ways)+ways->number*sizeof(Way)));

 return(ways);
}


/*++++++++++++++++++++++++++++++++++++++
  Return 0 if the two ways are the same (in respect of their types and limits),
           otherwise return positive or negative to allow sorting.

  int WaysCompare Returns a comparison.

  Way *way1 The first way.

  Way *way2 The second way.
  ++++++++++++++++++++++++++++++++++++++*/

int WaysCompare(Way *way1,Way *way2)
{
 if(way1==way2)
    return(0);

 if(way1->type!=way2->type)
    return((int)way1->type - (int)way2->type);

 if(way1->allow!=way2->allow)
    return((int)way1->allow - (int)way2->allow);

 if(way1->props!=way2->props)
    return((int)way1->props - (int)way2->props);

 if(way1->speed!=way2->speed)
    return((int)way1->speed - (int)way2->speed);

 if(way1->weight!=way2->weight)
    return((int)way1->weight - (int)way2->weight);

 if(way1->height!=way2->height)
    return((int)way1->height - (int)way2->height);

 if(way1->width!=way2->width)
    return((int)way1->width - (int)way2->width);

 if(way1->length!=way2->length)
    return((int)way1->length - (int)way2->length);

 return(0);
}
