/***************************************
 $Header: /home/amb/routino/src/RCS/functionsx.h,v 1.5 2010/05/22 18:40:47 amb Exp $

 Header file for function prototypes for extended data types.

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


#ifndef FUNCTIONSX_H
#define FUNCTIONSX_H    /*+ To stop multiple inclusions. +*/

#include <stdio.h>

#include "typesx.h"
#include "profiles.h"


/* In osmparser.c */

int ParseOSM(FILE *file,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays);


#endif /* FUNCTIONSX_H */
