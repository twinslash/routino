/***************************************
 $Header: /home/amb/routino/src/RCS/output.c,v 1.33 2010/07/07 17:31:06 amb Exp $

 Routing output generator.

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
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "types.h"
#include "functions.h"
#include "translations.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "results.h"
#include "xmlparse.h"


/* Global variables */

/*+ The option to calculate the quickest route insted of the shortest. +*/
extern int option_quickest;

/*+ The options to select the format of the output. +*/
extern int option_html,option_gpx_track,option_gpx_route,option_text,option_text_all;

/* Local variables */

/*+ Heuristics for determining if a junction is important. +*/
static char junction_other_way[Way_Count][Way_Count]=
 { /* M, T, P, S, T, U, R, S, T, C, P, S = Way type of route not taken */
  {   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* Motorway     */
  {   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* Trunk        */
  {   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* Primary      */
  {   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, /* Secondary    */
  {   1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 }, /* Tertiary     */
  {   1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 }, /* Unclassified */
  {   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 }, /* Residential  */
  {   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 }, /* Service      */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 }, /* Track        */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 }, /* Cycleway     */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Path         */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Steps        */
 };


/* Local functions */

static int turn_angle(Nodes *nodes,Segment *segment1,Segment *segment2,index_t node);
static int bearing_angle(Nodes *nodes,Segment *segment,index_t node);


/*++++++++++++++++++++++++++++++++++++++
  Print the optimum route between two nodes.

  Results **results The set of results to print (some may be NULL - ignore them).

  int nresults The number of results in the list.

  Nodes *nodes The list of nodes.

  Segments *segments The set of segments to use.

  Ways *ways The list of ways.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRoute(Results **results,int nresults,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile)
{
 FILE *htmlfile=NULL,*gpxtrackfile=NULL,*gpxroutefile=NULL,*textfile=NULL,*textallfile=NULL;

 int point=1;
 distance_t cum_distance=0;
 duration_t cum_duration=0;
 double finish_lat,finish_lon;
 int segment_count=0;
 int route_count=0;

 /* Open the files */

 if(option_quickest==0)
   {
    /* Print the result for the shortest route */

    if(option_html)
       htmlfile    =fopen("shortest.html","w");
    if(option_gpx_track)
       gpxtrackfile=fopen("shortest-track.gpx","w");
    if(option_gpx_route)
       gpxroutefile=fopen("shortest-route.gpx","w");
    if(option_text)
       textfile    =fopen("shortest.txt","w");
    if(option_text_all)
       textallfile =fopen("shortest-all.txt","w");

    if(option_html && !htmlfile)
       fprintf(stderr,"Warning: Cannot open file 'shortest.html' for writing [%s].\n",strerror(errno));
    if(option_gpx_track && !gpxtrackfile)
       fprintf(stderr,"Warning: Cannot open file 'shortest-track.gpx' for writing [%s].\n",strerror(errno));
    if(option_gpx_route && !gpxroutefile)
       fprintf(stderr,"Warning: Cannot open file 'shortest-route.gpx' for writing [%s].\n",strerror(errno));
    if(option_text && !textfile)
       fprintf(stderr,"Warning: Cannot open file 'shortest.txt' for writing [%s].\n",strerror(errno));
    if(option_text_all && !textallfile)
       fprintf(stderr,"Warning: Cannot open file 'shortest-all.txt' for writing [%s].\n",strerror(errno));
   }
 else
   {
    /* Print the result for the quickest route */

    if(option_html)
       htmlfile    =fopen("quickest.html","w");
    if(option_gpx_track)
       gpxtrackfile=fopen("quickest-track.gpx","w");
    if(option_gpx_route)
       gpxroutefile=fopen("quickest-route.gpx","w");
    if(option_text)
       textfile    =fopen("quickest.txt","w");
    if(option_text_all)
       textallfile =fopen("quickest-all.txt","w");

    if(option_html && !htmlfile)
       fprintf(stderr,"Warning: Cannot open file 'quickest.html' for writing [%s].\n",strerror(errno));
    if(option_gpx_track && !gpxtrackfile)
       fprintf(stderr,"Warning: Cannot open file 'quickest-track.gpx' for writing [%s].\n",strerror(errno));
    if(option_gpx_route && !gpxroutefile)
       fprintf(stderr,"Warning: Cannot open file 'quickest-route.gpx' for writing [%s].\n",strerror(errno));
    if(option_text && !textfile)
       fprintf(stderr,"Warning: Cannot open file 'quickest.txt' for writing [%s].\n",strerror(errno));
    if(option_text_all && !textallfile)
       fprintf(stderr,"Warning: Cannot open file 'quickest-all.txt' for writing [%s].\n",strerror(errno));
   }

 /* Print the head of the files */

 if(htmlfile)
   {
    fprintf(htmlfile,"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
    fprintf(htmlfile,"<HTML>\n");
    if(translate_copyright_creator[0] && translate_copyright_creator[1])
       fprintf(htmlfile,"<!-- %s : %s -->\n",translate_copyright_creator[0],translate_copyright_creator[1]);
    if(translate_copyright_source[0] && translate_copyright_source[1])
       fprintf(htmlfile,"<!-- %s : %s -->\n",translate_copyright_source[0],translate_copyright_source[1]);
    if(translate_copyright_license[0] && translate_copyright_license[1])
       fprintf(htmlfile,"<!-- %s : %s -->\n",translate_copyright_license[0],translate_copyright_license[1]);
    fprintf(htmlfile,"<HEAD>\n");
    fprintf(htmlfile,"<TITLE>");
    fprintf(htmlfile,translate_html_title,option_quickest?translate_route_quickest:translate_route_shortest);
    fprintf(htmlfile,"</TITLE>\n");
    fprintf(htmlfile,"<STYLE type='text/css'>\n");
    fprintf(htmlfile,"<!--\n");
    fprintf(htmlfile,"   table   {table-layout: fixed; border: none; border-collapse: collapse;}\n");
    fprintf(htmlfile,"   table.c {color: grey; font-size: x-small;} /* copyright */\n");
    fprintf(htmlfile,"   tr      {border: 0px;}\n");
    fprintf(htmlfile,"   tr.c    {display: none;} /* coords */\n");
    fprintf(htmlfile,"   tr.n    {} /* node */\n");
    fprintf(htmlfile,"   tr.s    {} /* segment */\n");
    fprintf(htmlfile,"   tr.t    {font-weight: bold;} /* total */\n");
    fprintf(htmlfile,"   td.l    {font-weight: bold;}\n");
    fprintf(htmlfile,"   td.r    {}\n");
    fprintf(htmlfile,"   span.w  {font-weight: bold;} /* waypoint */\n");
    fprintf(htmlfile,"   span.h  {text-decoration: underline;} /* highway */\n");
    fprintf(htmlfile,"   span.d  {} /* segment distance */\n");
    fprintf(htmlfile,"   span.j  {font-style: italic;} /* total journey distance */\n");
    fprintf(htmlfile,"   span.t  {font-variant: small-caps;} /* turn */\n");
    fprintf(htmlfile,"   span.b  {font-variant: small-caps;} /* bearing */\n");
    fprintf(htmlfile,"-->\n");
    fprintf(htmlfile,"</STYLE>\n");
    fprintf(htmlfile,"</HEAD>\n");
    fprintf(htmlfile,"<BODY>\n");
    fprintf(htmlfile,"<H1>");
    fprintf(htmlfile,translate_html_title,option_quickest?translate_route_quickest:translate_route_shortest);
    fprintf(htmlfile,"</H1>\n");
    fprintf(htmlfile,"<table>\n");
   }

 if(gpxtrackfile)
   {
    fprintf(gpxtrackfile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(gpxtrackfile,"<gpx version=\"1.1\" creator=\"Routino\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

    fprintf(gpxtrackfile,"<metadata>\n");
    fprintf(gpxtrackfile,"<desc>%s : %s</desc>\n",translate_copyright_creator[0],translate_copyright_creator[1]);
    if(translate_copyright_source[1])
      {
       fprintf(gpxtrackfile,"<copyright author=\"%s\">\n",translate_copyright_source[1]);

       if(translate_copyright_license[1])
          fprintf(gpxtrackfile,"<license>%s</license>\n",translate_copyright_license[1]);

       fprintf(gpxtrackfile,"</copyright>\n");
      }
    fprintf(gpxtrackfile,"</metadata>\n");

    fprintf(gpxtrackfile,"<trk>\n");
    fprintf(gpxtrackfile,"<name>");
    fprintf(gpxtrackfile,translate_gpx_name,option_quickest?translate_route_quickest:translate_route_shortest);
    fprintf(gpxtrackfile,"</name>\n");
    fprintf(gpxtrackfile,"<desc>");
    fprintf(gpxtrackfile,translate_gpx_desc,option_quickest?translate_route_quickest:translate_route_shortest);
    fprintf(gpxtrackfile,"</desc>\n");
   }

 if(gpxroutefile)
   {
    fprintf(gpxroutefile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(gpxroutefile,"<gpx version=\"1.1\" creator=\"Routino\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

    fprintf(gpxroutefile,"<metadata>\n");
    fprintf(gpxroutefile,"<desc>%s : %s</desc>\n",translate_copyright_creator[0],translate_copyright_creator[1]);
    if(translate_copyright_source[1])
      {
       fprintf(gpxroutefile,"<copyright author=\"%s\">\n",translate_copyright_source[1]);

       if(translate_copyright_license[1])
          fprintf(gpxroutefile,"<license>%s</license>\n",translate_copyright_license[1]);

       fprintf(gpxroutefile,"</copyright>\n");
      }
    fprintf(gpxroutefile,"</metadata>\n");

    fprintf(gpxroutefile,"<rte>\n");
    fprintf(gpxroutefile,"<name>");
    fprintf(gpxroutefile,translate_gpx_name,option_quickest?translate_route_quickest:translate_route_shortest);
    fprintf(gpxroutefile,"</name>\n");
    fprintf(gpxroutefile,"<desc>");
    fprintf(gpxroutefile,translate_gpx_desc,option_quickest?translate_route_quickest:translate_route_shortest);
    fprintf(gpxroutefile,"</desc>\n");
   }

 if(textfile)
   {
    if(translate_copyright_creator[0] && translate_copyright_creator[1])
       fprintf(textfile,"# %s : %s\n",translate_copyright_creator[0],translate_copyright_creator[1]);
    if(translate_copyright_source[0] && translate_copyright_source[1])
       fprintf(textfile,"# %s : %s\n",translate_copyright_source[0],translate_copyright_source[1]);
    if(translate_copyright_license[0] && translate_copyright_license[1])
       fprintf(textfile,"# %s : %s\n",translate_copyright_license[0],translate_copyright_license[1]);
    if((translate_copyright_creator[0] && translate_copyright_creator[1]) ||
       (translate_copyright_source[0]  && translate_copyright_source[1]) ||
       (translate_copyright_license[0] && translate_copyright_license[1]))
       fprintf(textfile,"#\n");

    fprintf(textfile,"#Latitude\tLongitude\tSection \tSection \tTotal   \tTotal   \tPoint\tTurn\tBearing\tHighway\n");
    fprintf(textfile,"#        \t         \tDistance\tDuration\tDistance\tDuration\tType \t    \t       \t       \n");
                     /* "%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t %+d\t %+d\t%s\n" */
   }

 if(textallfile)
   {
    if(translate_copyright_creator[0] && translate_copyright_creator[1])
       fprintf(textallfile,"# %s : %s\n",translate_copyright_creator[0],translate_copyright_creator[1]);
    if(translate_copyright_source[0] && translate_copyright_source[1])
       fprintf(textallfile,"# %s : %s\n",translate_copyright_source[0],translate_copyright_source[1]);
    if(translate_copyright_license[0] && translate_copyright_license[1])
       fprintf(textallfile,"# %s : %s\n",translate_copyright_license[0],translate_copyright_license[1]);
    if((translate_copyright_creator[0] && translate_copyright_creator[1]) ||
       (translate_copyright_source[0]  && translate_copyright_source[1]) ||
       (translate_copyright_license[0] && translate_copyright_license[1]))
       fprintf(textallfile,"#\n");

    fprintf(textallfile,"#Latitude\tLongitude\t    Node\tType\tSegment\tSegment\tTotal\tTotal  \tSpeed\tBearing\tHighway\n");
    fprintf(textallfile,"#        \t         \t        \t    \tDist   \tDurat'n\tDist \tDurat'n\t     \t       \t       \n");
                        /* "%10.6f\t%11.6f\t%8d%c\t%s\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t%3d\t%4d\t%s\n" */
   }

 /* Loop through the segments of the route and print it */

 while(!results[point])
    point++;

 while(point<=nresults)
   {
    int nextpoint=point;
    double start_lat,start_lon;
    distance_t junc_distance=0;
    duration_t junc_duration=0;
    Result *result;

    if(gpxtrackfile)
       fprintf(gpxtrackfile,"<trkseg>\n");

    if(IsFakeNode(results[point]->start))
       GetFakeLatLong(results[point]->start,&start_lat,&start_lon);
    else
       GetLatLong(nodes,results[point]->start,&start_lat,&start_lon);

    if(IsFakeNode(results[point]->finish))
       GetFakeLatLong(results[point]->finish,&finish_lat,&finish_lon);
    else
       GetLatLong(nodes,results[point]->finish,&finish_lat,&finish_lon);

    result=FindResult(results[point],results[point]->start);

    do
      {
       double latitude,longitude;
       Result *nextresult;

       if(result->node==results[point]->start)
         {latitude=start_lat; longitude=start_lon;}
       else if(result->node==results[point]->finish)
         {latitude=finish_lat; longitude=finish_lon;}
       else
          GetLatLong(nodes,result->node,&latitude,&longitude);

       if(gpxtrackfile)
          fprintf(gpxtrackfile,"<trkpt lat=\"%.6f\" lon=\"%.6f\"/>\n",
                  radians_to_degrees(latitude),radians_to_degrees(longitude));

       nextresult=FindResult(results[point],result->next);

       if(!nextresult)
          for(nextpoint=point+1;nextpoint<=nresults;nextpoint++)
             if(results[nextpoint])
               {
                nextresult=FindResult(results[nextpoint],results[nextpoint]->start);
                nextresult=FindResult(results[nextpoint],nextresult->next);
                break;
               }

       if(result->node!=results[point]->start)
         {
          distance_t seg_distance=0;
          duration_t seg_duration=0;
          Way *resultway;
          int important=0;

          /* Cache the values to be printed rather than calculating them repeatedly for each output format */

          char *waynameraw=NULL,*wayname=NULL,*waynamexml=NULL;
          int bearing_int=0,bearing_next_int=0,turn_int=0;
          char *bearing_str=NULL,*bearing_next_str=NULL,*turn_str=NULL;

          /* Get the properties of this segment */

          resultway=LookupWay(ways,result->segment->way);

          seg_distance+=DISTANCE(result->segment->distance);
          seg_duration+=Duration(result->segment,resultway,profile);
          junc_distance+=seg_distance;
          junc_duration+=seg_duration;
          cum_distance+=seg_distance;
          cum_duration+=seg_duration;

          /* Decide if this is an important junction */

          if(result->node==results[point]->finish)
             important=10;
          else
            {
             Segment *segment=FirstSegment(segments,nodes,result->node);

             do
               {
                index_t othernode=OtherNode(segment,result->node);

                if(othernode!=result->prev && segment!=result->segment)
                   if(IsNormalSegment(segment) && (!profile->oneway || !IsOnewayTo(segment,result->node)))
                     {
                      Way *way=LookupWay(ways,segment->way);

                      if(othernode==result->next) /* the next segment that we follow */
                        {
                         if(HIGHWAY(way->type)!=HIGHWAY(resultway->type))
                            if(important<2)
                               important=2;
                        }
                      else /* a segment that we don't follow */
                        {
                         if(junction_other_way[HIGHWAY(resultway->type)-1][HIGHWAY(way->type)-1])
                            if(important<3)
                               important=3;

                         if(important<1)
                            important=1;
                        }
                     }

                segment=NextSegment(segments,segment,result->node);
               }
             while(segment);
            }

          /* Print out the important points (junctions / waypoints) */

          if(important>1)
            {
             /* Print the intermediate finish points (because they have correct junction distances) */

             if(htmlfile)
               {
                char *type;

                if(important==10)
                   type=translate_html_waypoint;
                else
                   type=translate_html_junction;

                if(!waynameraw)
                  {
                   waynameraw=WayNameRaw(ways,resultway);
                   if(!*waynameraw)
                      waynameraw=translate_highway[HIGHWAY(resultway->type)];
                  }

                if(!waynamexml)
                   waynamexml=ParseXML_Encode_Safe_XML(waynameraw);

                fprintf(htmlfile,"<tr class='s'><td class='l'>%s:<td class='r'>",translate_html_segment[0]);
                fprintf(htmlfile,translate_html_segment[1],
                                  waynamexml,
                                  distance_to_km(junc_distance),duration_to_minutes(junc_duration));
                fprintf(htmlfile," [<span class='j'>");
                fprintf(htmlfile,translate_html_total[1],
                                  distance_to_km(cum_distance),duration_to_minutes(cum_duration));
                fprintf(htmlfile,"</span>]\n");

                fprintf(htmlfile,"<tr class='c'><td class='l'><td class='r'>%.6f %.6f\n",
                                 radians_to_degrees(latitude),radians_to_degrees(longitude));

                if(nextresult)
                  {
                   if(!turn_str)
                     {
                      turn_int=turn_angle(nodes,result->segment,nextresult->segment,result->node);
                      turn_str=translate_turn[(4+(22+turn_int)/45)%8];
                     }

                   if(!bearing_next_str)
                     {
                      bearing_next_int=bearing_angle(nodes,nextresult->segment,nextresult->node);
                      bearing_next_str=translate_heading[(4+(22+bearing_next_int)/45)%8];
                     }

                   fprintf(htmlfile,"<tr class='n'><td class='l'>%s:<td class='r'>",translate_html_node[0]);
                   fprintf(htmlfile,translate_html_node[1],
                                    type,
                                    turn_str,
                                    bearing_next_str);
                   fprintf(htmlfile,"\n");
                  }
                else
                  {
                   fprintf(htmlfile,"<tr class='n'><td class='l'>%s:<td class='r'>",translate_html_stop[0]);
                   fprintf(htmlfile,translate_html_stop[1],
                                    translate_html_waypoint);
                   fprintf(htmlfile,"\n");
                   fprintf(htmlfile,"<tr class='t'><td class='l'>%s:<td class='r'><span class='j'>",translate_html_total[0]);
                   fprintf(htmlfile,translate_html_total[1],
                                    distance_to_km(cum_distance),duration_to_minutes(cum_duration));
                   fprintf(htmlfile,"</span>\n");
                  }
               }

             if(gpxroutefile)
               {
                if(!waynameraw)
                  {
                   waynameraw=WayNameRaw(ways,resultway);
                   if(!*waynameraw)
                      waynameraw=translate_highway[HIGHWAY(resultway->type)];
                  }

                if(!waynamexml)
                   waynamexml=ParseXML_Encode_Safe_XML(waynameraw);

                if(!bearing_str)
                  {
                   bearing_int=bearing_angle(nodes,result->segment,result->node);
                   bearing_str=translate_heading[(4+(22+bearing_int)/45)%8];
                  }

                fprintf(gpxroutefile,"<desc>");
                fprintf(gpxroutefile,translate_gpx_step,
                                     bearing_str,
                                     waynamexml,
                                     distance_to_km(junc_distance),duration_to_minutes(junc_duration));
                fprintf(gpxroutefile,"</desc></rtept>\n");

                if(!nextresult)
                  {
                   fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s</name>\n",
                                        radians_to_degrees(finish_lat),radians_to_degrees(finish_lon),
                                        translate_gpx_finish);
                   fprintf(gpxroutefile,"<desc>");
                   fprintf(gpxroutefile,translate_gpx_final,
                                        distance_to_km(cum_distance),duration_to_minutes(cum_duration));
                   fprintf(gpxroutefile,"</desc></rtept>\n");
                  }
                else if(important==10)
                   fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s%d</name>\n",
                                        radians_to_degrees(latitude),radians_to_degrees(longitude),
                                        translate_gpx_inter,++segment_count);
                else
                   fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s%03d</name>\n",
                                        radians_to_degrees(latitude),radians_to_degrees(longitude),
                                        translate_gpx_trip,++route_count);
               }

             if(textfile)
               {
                char *type;

                if(important==10)
                   type="Waypt";
                else
                   type="Junct";

                if(!wayname)
                   wayname=(char*)WayNameHighway(ways,resultway);

                if(nextresult)
                  {
                   if(!turn_str)
                     {
                      turn_int=turn_angle(nodes,result->segment,nextresult->segment,result->node);
                      turn_str=translate_turn[(4+(22+turn_int)/45)%8];
                     }

                   if(!bearing_next_str)
                     {
                      bearing_next_int=bearing_angle(nodes,nextresult->segment,nextresult->node);
                      bearing_next_str=translate_heading[(4+(22+bearing_next_int)/45)%8];
                     }

                   fprintf(textfile,"%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t %+d\t %+d\t%s\n",
                                    radians_to_degrees(latitude),radians_to_degrees(longitude),
                                    distance_to_km(junc_distance),duration_to_minutes(junc_duration),
                                    distance_to_km(cum_distance),duration_to_minutes(cum_duration),
                                    type,
                                    (22+turn_int)/45,
                                    ((22+bearing_next_int)/45+4)%8-4,
                                    wayname);
                  }
                else
                   fprintf(textfile,"%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t\t\t%s\n",
                                    radians_to_degrees(latitude),radians_to_degrees(longitude),
                                    distance_to_km(junc_distance),duration_to_minutes(junc_duration),
                                    distance_to_km(cum_distance),duration_to_minutes(cum_duration),
                                    type,
                                    wayname);
               }

             junc_distance=0;
             junc_duration=0;
            }

          /* Print out all of the results */

          if(textallfile)
            {
             char *type;

             if(important==10)
                type="Waypt";
             else if(important==2)
                type="Change";
             else if(important>=1)
                type="Junct";
             else
                type="Inter";

             if(!wayname)
                wayname=(char*)WayNameHighway(ways,resultway);

             if(!bearing_str)
               {
                bearing_int=bearing_angle(nodes,result->segment,result->node);
                bearing_str=translate_heading[(4+(22+bearing_int)/45)%8];
               }

             fprintf(textallfile,"%10.6f\t%11.6f\t%8d%c\t%s\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t%3d\t%4d\t%s\n",
                                 radians_to_degrees(latitude),radians_to_degrees(longitude),
                                 IsFakeNode(result->node)?-(result->node&(~NODE_SUPER)):result->node,
                                 (!IsFakeNode(result->node) && IsSuperNode(nodes,result->node))?'*':' ',type,
                                 distance_to_km(seg_distance),duration_to_minutes(seg_duration),
                                 distance_to_km(cum_distance),duration_to_minutes(cum_duration),
                                 profile->speed[HIGHWAY(resultway->type)],
                                 bearing_int,
                                 wayname);
            }

          if(waynamexml && waynamexml!=waynameraw)
             free(waynamexml);
         }
       else if(!cum_distance)
         {
          int   bearing_next_int=bearing_angle(nodes,nextresult->segment,nextresult->node);
          char *bearing_next_str=translate_heading[(4+(22+bearing_next_int)/45)%8];

          /* Print out the very first start point */

          if(htmlfile)
            {
             fprintf(htmlfile,"<tr class='c'><td class='l'><td class='r'>%.6f %.6f\n",
                              radians_to_degrees(latitude),radians_to_degrees(longitude));
             fprintf(htmlfile,"<tr class='n'><td class='l'>%s:<td class='r'>",translate_html_start[0]);
             fprintf(htmlfile,translate_html_start[1],
                              translate_html_waypoint,
                              bearing_next_str);
             fprintf(htmlfile,"\n");
            }

          if(gpxroutefile)
             fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s</name>\n",
                                  radians_to_degrees(latitude),radians_to_degrees(longitude),
                                  translate_gpx_start);

          if(textfile)
             fprintf(textfile,"%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t\t +%d\t\n",
                              radians_to_degrees(latitude),radians_to_degrees(longitude),
                              0.0,0.0,0.0,0.0,
                              "Waypt",
                              (22+bearing_next_int)/45);

          if(textallfile)
             fprintf(textallfile,"%10.6f\t%11.6f\t%8d%c\t%s\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t\t\t\n",
                                 radians_to_degrees(latitude),radians_to_degrees(longitude),
                                 IsFakeNode(result->node)?-(result->node&(~NODE_SUPER)):result->node,
                                 (!IsFakeNode(result->node) && IsSuperNode(nodes,result->node))?'*':' ',"Waypt",
                                 0.0,0.0,0.0,0.0);
         }

       result=nextresult;
      }
    while(point==nextpoint);

    if(gpxtrackfile)
       fprintf(gpxtrackfile,"</trkseg>\n");

    point=nextpoint;
   }

 /* Print the tail of the files */

 if(htmlfile)
   {
    fprintf(htmlfile,"</table>\n");

    if((translate_copyright_creator[0] && translate_copyright_creator[1]) ||
       (translate_copyright_source[0]  && translate_copyright_source[1]) ||
       (translate_copyright_license[0] && translate_copyright_license[1]))
      {
       fprintf(htmlfile,"<p>\n");
       fprintf(htmlfile,"<table class='c'>\n");
       if(translate_copyright_creator[0] && translate_copyright_creator[1])
          fprintf(htmlfile,"<tr><td class='l'>%s:<td class='r'>%s\n",translate_copyright_creator[0],translate_copyright_creator[1]);
       if(translate_copyright_source[0] && translate_copyright_source[1])
          fprintf(htmlfile,"<tr><td class='l'>%s:<td class='r'>%s\n",translate_copyright_source[0],translate_copyright_source[1]);
       if(translate_copyright_license[0] && translate_copyright_license[1])
          fprintf(htmlfile,"<tr><td class='l'>%s:<td class='r'>%s\n",translate_copyright_license[0],translate_copyright_license[1]);
       fprintf(htmlfile,"</table>\n");
      }

    fprintf(htmlfile,"</BODY>\n");
    fprintf(htmlfile,"</HTML>\n");
   }

 if(gpxtrackfile)
   {
    fprintf(gpxtrackfile,"</trk>\n");
    fprintf(gpxtrackfile,"</gpx>\n");
   }

 if(gpxroutefile)
   {
    fprintf(gpxroutefile,"</rte>\n");
    fprintf(gpxroutefile,"</gpx>\n");
   }

 /* Close the files */

 if(htmlfile)
    fclose(htmlfile);
 if(gpxtrackfile)
    fclose(gpxtrackfile);
 if(gpxroutefile)
    fclose(gpxroutefile);
 if(textfile)
    fclose(textfile);
 if(textallfile)
    fclose(textallfile);
}


/*++++++++++++++++++++++++++++++++++++++
  Calculate the angle to turn at a junction from segment1 to segment2 at node.

  int turn_angle Returns a value in the range -4 to +4 indicating the angle to turn.

  Nodes *nodes The set of nodes.

  Segment *segment1 The current segment.

  Segment *segment2 The next segment.

  index_t node The node at which they join.

  Straight ahead is zero, turning to the right is positive (90 degrees) and turning to the left is negative.
  Angles are calculated using flat Cartesian lat/long grid approximation (after scaling longitude due to latitude).
  ++++++++++++++++++++++++++++++++++++++*/

static int turn_angle(Nodes *nodes,Segment *segment1,Segment *segment2,index_t node)
{
 double lat1,latm,lat2;
 double lon1,lonm,lon2;
 double angle1,angle2,angle;
 index_t node1,node2;

 node1=OtherNode(segment1,node);
 node2=OtherNode(segment2,node);

 if(IsFakeNode(node1))
    GetFakeLatLong(node1,&lat1,&lon1);
 else
    GetLatLong(nodes,node1,&lat1,&lon1);

 if(IsFakeNode(node))
    GetFakeLatLong(node,&latm,&lonm);
 else
    GetLatLong(nodes,node,&latm,&lonm);

 if(IsFakeNode(node2))
    GetFakeLatLong(node2,&lat2,&lon2);
 else
    GetLatLong(nodes,node2,&lat2,&lon2);

 angle1=atan2((lonm-lon1)*cos(latm),(latm-lat1));
 angle2=atan2((lon2-lonm)*cos(latm),(lat2-latm));

 angle=angle2-angle1;

 angle=radians_to_degrees(angle);

 angle=round(angle);

 if(angle<-180) angle+=360;
 if(angle> 180) angle-=360;

 return((int)angle);
}


/*++++++++++++++++++++++++++++++++++++++
  Calculate the bearing of a segment from the given node.

  int bearing_angle Returns a value in the range 0 to 359 indicating the bearing.

  Nodes *nodes The set of nodes.

  Segment *segment The segment.

  index_t node The node to start.

  Angles are calculated using flat Cartesian lat/long grid approximation (after scaling longitude due to latitude).
  ++++++++++++++++++++++++++++++++++++++*/

static int bearing_angle(Nodes *nodes,Segment *segment,index_t node)
{
 double lat1,lat2;
 double lon1,lon2;
 double angle;
 index_t node1,node2;

 node1=node;
 node2=OtherNode(segment,node);

 if(IsFakeNode(node1))
    GetFakeLatLong(node1,&lat1,&lon1);
 else
    GetLatLong(nodes,node1,&lat1,&lon1);

 if(IsFakeNode(node2))
    GetFakeLatLong(node2,&lat2,&lon2);
 else
    GetLatLong(nodes,node2,&lat2,&lon2);

 angle=atan2((lat2-lat1),(lon2-lon1)*cos(lat1));

 angle=radians_to_degrees(angle);

 angle=round(270-angle);

 if(angle<  0) angle+=360;
 if(angle>360) angle-=360;

 return((int)angle);
}
