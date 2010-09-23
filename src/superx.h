/***************************************
 $Header: /home/amb/routino/src/RCS/superx.h,v 1.7 2009/09/06 15:51:09 amb Exp $

 Header for super-node and super-segment functions.

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


#ifndef SUPERX_H
#define SUPERX_H    /*+ To stop multiple inclusions. +*/

#include "typesx.h"


void ChooseSuperNodes(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx);

SegmentsX *CreateSuperSegments(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,int iteration);

SegmentsX* MergeSuperSegments(SegmentsX* segmentsx,SegmentsX* supersegmentsx);


#endif /* SUPERX_H */
