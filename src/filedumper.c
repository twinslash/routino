/***************************************
 $Header: /home/amb/routino/src/RCS/filedumper.c,v 1.43 2010/05/30 12:52:16 amb Exp $

 Memory file dumper.

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "types.h"
#include "functions.h"
#include "visualiser.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "xmlparse.h"


/* Local functions */

static void print_node(Nodes* nodes,index_t item);
static void print_segment(Segments *segments,index_t item);
static void print_way(Ways *ways,index_t item);

static void print_head_osm(void);
static void print_node_osm(Nodes* nodes,index_t item);
static void print_segment_osm(Segments *segments,index_t item,Ways *ways);
static void print_tail_osm(void);

static char *RFC822Date(time_t t);

static void print_usage(int detail);


/*++++++++++++++++++++++++++++++++++++++
  The main program for the file dumper.
  ++++++++++++++++++++++++++++++++++++++*/

int main(int argc,char** argv)
{
 Nodes    *OSMNodes;
 Segments *OSMSegments;
 Ways     *OSMWays;
 int       arg;
 char     *dirname=NULL,*prefix=NULL;
 char     *nodes_filename,*segments_filename,*ways_filename;
 int       option_statistics=0;
 int       option_visualiser=0,coordcount=0;
 double    latmin=0,latmax=0,lonmin=0,lonmax=0;
 char     *option_data=NULL;
 int       option_dump=0;
 int       option_dump_osm=0,option_no_super=0;

 /* Parse the command line arguments */

 for(arg=1;arg<argc;arg++)
   {
    if(!strcmp(argv[arg],"--help"))
       print_usage(1);
    else if(!strncmp(argv[arg],"--dir=",6))
       dirname=&argv[arg][6];
    else if(!strncmp(argv[arg],"--prefix=",9))
       prefix=&argv[arg][9];
    else if(!strcmp(argv[arg],"--statistics"))
       option_statistics=1;
    else if(!strcmp(argv[arg],"--visualiser"))
       option_visualiser=1;
    else if(!strcmp(argv[arg],"--dump"))
       option_dump=1;
    else if(!strcmp(argv[arg],"--dump-osm"))
       option_dump_osm=1;
    else if(!strncmp(argv[arg],"--latmin",8) && argv[arg][8]=='=')
      {latmin=degrees_to_radians(atof(&argv[arg][9]));coordcount++;}
    else if(!strncmp(argv[arg],"--latmax",8) && argv[arg][8]=='=')
      {latmax=degrees_to_radians(atof(&argv[arg][9]));coordcount++;}
    else if(!strncmp(argv[arg],"--lonmin",8) && argv[arg][8]=='=')
      {lonmin=degrees_to_radians(atof(&argv[arg][9]));coordcount++;}
    else if(!strncmp(argv[arg],"--lonmax",8) && argv[arg][8]=='=')
      {lonmax=degrees_to_radians(atof(&argv[arg][9]));coordcount++;}
    else if(!strncmp(argv[arg],"--data",6) && argv[arg][6]=='=')
       option_data=&argv[arg][7];
    else if(!strcmp(argv[arg],"--no-super"))
       option_no_super=1;
    else if(!strncmp(argv[arg],"--node=",7))
       ;
    else if(!strncmp(argv[arg],"--segment=",10))
       ;
    else if(!strncmp(argv[arg],"--way=",6))
       ;
    else
       print_usage(0);
   }

 if(!option_statistics && !option_visualiser && !option_dump && !option_dump_osm)
    print_usage(0);

 /* Load in the data - Note: No error checking because Load*List() will call exit() in case of an error. */

 OSMNodes=LoadNodeList(nodes_filename=FileName(dirname,prefix,"nodes.mem"));

 OSMSegments=LoadSegmentList(segments_filename=FileName(dirname,prefix,"segments.mem"));

 OSMWays=LoadWayList(ways_filename=FileName(dirname,prefix,"ways.mem"));

 /* Write out the visualiser data */

 if(option_visualiser)
   {
    if(coordcount!=4)
      {
       fprintf(stderr,"The --visualiser option must have --latmin, --latmax, --lonmin, --lonmax.\n");
       exit(1);
      }

    if(!option_data)
      {
       fprintf(stderr,"The --visualiser option must have --data.\n");
       exit(1);
      }

    if(!strcmp(option_data,"junctions"))
       OutputJunctions(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"super"))
       OutputSuper(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"oneway"))
       OutputOneway(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"speed"))
       OutputSpeedLimits(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"weight"))
       OutputWeightLimits(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"height"))
       OutputHeightLimits(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"width"))
       OutputWidthLimits(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"length"))
       OutputLengthLimits(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else
      {
       fprintf(stderr,"Unrecognised data option '%s' with --visualiser.\n",option_data);
       exit(1);
      }
   }

 /* Print out statistics */

 if(option_statistics)
   {
    struct stat buf;

    /* Examine the files */

    printf("Files\n");
    printf("-----\n");
    printf("\n");

    stat(nodes_filename,&buf);

    printf("'%s%snodes.mem'    - %9lld Bytes\n",prefix?prefix:"",prefix?"-":"",(long long)buf.st_size);
    printf("%s\n",RFC822Date(buf.st_mtime));
    printf("\n");

    stat(segments_filename,&buf);

    printf("'%s%ssegments.mem' - %9lld Bytes\n",prefix?prefix:"",prefix?"-":"",(long long)buf.st_size);
    printf("%s\n",RFC822Date(buf.st_mtime));
    printf("\n");

    stat(ways_filename,&buf);

    printf("'%s%sways.mem'     - %9lld Bytes\n",prefix?prefix:"",prefix?"-":"",(long long)buf.st_size);
    printf("%s\n",RFC822Date(buf.st_mtime));
    printf("\n");

    /* Examine the nodes */

    printf("Nodes\n");
    printf("-----\n");
    printf("\n");

    printf("sizeof(Node) =%9d Bytes\n",sizeof(Node));
    printf("Number       =%9d\n",OSMNodes->number);
    printf("Number(super)=%9d\n",OSMNodes->snumber);
    printf("\n");

    printf("Lat bins= %4d\n",OSMNodes->latbins);
    printf("Lon bins= %4d\n",OSMNodes->lonbins);
    printf("\n");

    printf("Lat zero=%5d (%8.4f deg)\n",OSMNodes->latzero,radians_to_degrees(latlong_to_radians(bin_to_latlong(OSMNodes->latzero))));
    printf("Lon zero=%5d (%8.4f deg)\n",OSMNodes->lonzero,radians_to_degrees(latlong_to_radians(bin_to_latlong(OSMNodes->lonzero))));

    /* Examine the segments */

    printf("\n");
    printf("Segments\n");
    printf("--------\n");
    printf("\n");

    printf("sizeof(Segment)=%9d Bytes\n",sizeof(Segment));
    printf("Number(total)  =%9d\n",OSMSegments->number);
    printf("Number(super)  =%9d\n",OSMSegments->snumber);
    printf("Number(normal) =%9d\n",OSMSegments->nnumber);

    /* Examine the ways */

    printf("\n");
    printf("Ways\n");
    printf("----\n");
    printf("\n");

    printf("sizeof(Way)      =%9d Bytes\n",sizeof(Way));
    printf("Number(compacted)=%9d\n",OSMWays->number);
    printf("Number(original) =%9d\n",OSMWays->onumber);
    printf("\n");

    printf("Total names =%9ld Bytes\n",(long)buf.st_size-sizeof(Ways)-OSMWays->number*sizeof(Way));
    printf("\n");

    printf("Included transports: %s\n",AllowedNameList(OSMWays->allow));
    printf("Included properties: %s\n",PropertiesNameList(OSMWays->props));
   }

 /* Print out internal data */

 if(option_dump)
   {
    index_t item;

    for(arg=1;arg<argc;arg++)
       if(!strcmp(argv[arg],"--node=all"))
         {
          for(item=0;item<OSMNodes->number;item++)
             print_node(OSMNodes,item);
         }
       else if(!strncmp(argv[arg],"--node=",7))
         {
          item=atoi(&argv[arg][7]);

          if(item>=0 && item<OSMNodes->number)
             print_node(OSMNodes,item);
          else
             printf("Invalid node number; minimum=0, maximum=%d.\n",OSMNodes->number-1);
         }
       else if(!strcmp(argv[arg],"--segment=all"))
         {
          for(item=0;item<OSMSegments->number;item++)
             print_segment(OSMSegments,item);
         }
       else if(!strncmp(argv[arg],"--segment=",10))
         {
          item=atoi(&argv[arg][10]);

          if(item>=0 && item<OSMSegments->number)
             print_segment(OSMSegments,item);
          else
             printf("Invalid segment number; minimum=0, maximum=%d.\n",OSMSegments->number-1);
         }
       else if(!strcmp(argv[arg],"--way=all"))
         {
          for(item=0;item<OSMWays->number;item++)
             print_way(OSMWays,item);
         }
       else if(!strncmp(argv[arg],"--way=",6))
         {
          item=atoi(&argv[arg][6]);

          if(item>=0 && item<OSMWays->number)
             print_way(OSMWays,item);
          else
             printf("Invalid way number; minimum=0, maximum=%d.\n",OSMWays->number-1);
         }
   }

 /* Print out internal data in XML format */

 if(option_dump_osm)
   {
    if(coordcount>0 && coordcount!=4)
      {
       fprintf(stderr,"The --dump-osm option must have all of --latmin, --latmax, --lonmin, --lonmax or none.\n");
       exit(1);
      }

    print_head_osm();

    if(coordcount)
      {
       int32_t latminbin=latlong_to_bin(radians_to_latlong(latmin))-OSMNodes->latzero;
       int32_t latmaxbin=latlong_to_bin(radians_to_latlong(latmax))-OSMNodes->latzero;
       int32_t lonminbin=latlong_to_bin(radians_to_latlong(lonmin))-OSMNodes->lonzero;
       int32_t lonmaxbin=latlong_to_bin(radians_to_latlong(lonmax))-OSMNodes->lonzero;
       int latb,lonb,llbin;
       index_t node;

       /* Loop through all of the nodes. */

       for(latb=latminbin;latb<=latmaxbin;latb++)
          for(lonb=lonminbin;lonb<=lonmaxbin;lonb++)
            {
             llbin=lonb*OSMNodes->latbins+latb;

             if(llbin<0 || llbin>(OSMNodes->latbins*OSMNodes->lonbins))
                continue;

             for(node=OSMNodes->offsets[llbin];node<OSMNodes->offsets[llbin+1];node++)
               {
                double lat=latlong_to_radians(bin_to_latlong(OSMNodes->latzero+latb)+off_to_latlong(OSMNodes->nodes[node].latoffset));
                double lon=latlong_to_radians(bin_to_latlong(OSMNodes->lonzero+lonb)+off_to_latlong(OSMNodes->nodes[node].lonoffset));

                if(lat>latmin && lat<latmax && lon>lonmin && lon<lonmax)
                  {
                   Segment *segment;

                   print_node_osm(OSMNodes,node);

                   segment=FirstSegment(OSMSegments,OSMNodes,node);

                   while(segment)
                     {
                      if(node>OtherNode(segment,node))
                         if(!option_no_super || IsNormalSegment(segment))
                            print_segment_osm(OSMSegments,IndexSegment(OSMSegments,segment),OSMWays);

                      segment=NextSegment(OSMSegments,segment,node);
                     }
                  }
               }
            }
      }
    else
      {
       index_t item;

       for(item=0;item<OSMNodes->number;item++)
          print_node_osm(OSMNodes,item);

       for(item=0;item<OSMSegments->number;item++)
          if(!option_no_super || IsNormalSegment(LookupSegment(OSMSegments,item)))
             print_segment_osm(OSMSegments,item,OSMWays);
      }

    print_tail_osm();
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the contents of a node from the routing database.

  Nodes *nodes The set of nodes to use.

  index_t item The node index to print.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_node(Nodes* nodes,index_t item)
{
 Node *node=LookupNode(nodes,item);
 double latitude,longitude;

 GetLatLong(nodes,item,&latitude,&longitude);

 printf("Node %d\n",item);
 printf("  firstseg=%d\n",SEGMENT(node->firstseg));
 printf("  latoffset=%d lonoffset=%d (latitude=%.6f longitude=%.6f)\n",node->latoffset,node->lonoffset,radians_to_degrees(latitude),radians_to_degrees(longitude));
 if(IsSuperNode(nodes,item))
    printf("  Super-Node\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the contents of a segment from the routing database.

  Segments *segments The set of segments to use.

  index_t item The segment index to print.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_segment(Segments *segments,index_t item)
{
 Segment *segment=LookupSegment(segments,item);

 printf("Segment %d\n",item);
 printf("  node1=%d node2=%d\n",segment->node1,segment->node2);
 printf("  next2=%d\n",segment->next2);
 printf("  way=%d\n",segment->way);
 printf("  distance=%d (%.3f km)\n",DISTANCE(segment->distance),distance_to_km(DISTANCE(segment->distance)));
 if(IsSuperSegment(segment) && IsNormalSegment(segment))
    printf("  Super-Segment AND normal Segment\n");
 else if(IsSuperSegment(segment) && !IsNormalSegment(segment))
    printf("  Super-Segment\n");
 if(IsOnewayTo(segment,segment->node1))
    printf("  One-Way from node2 to node1\n");
 if(IsOnewayTo(segment,segment->node2))
    printf("  One-Way from node1 to node2\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the contents of a way from the routing database.

  Ways *ways The set of ways to use.

  index_t item The way index to print.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_way(Ways *ways,index_t item)
{
 Way *way=LookupWay(ways,item);

 printf("Way %d\n",item);
 printf("  name=%s\n",WayNameHighway(ways,way));
 printf("  type=%02x (%s%s%s)\n",way->type,HighwayName(HIGHWAY(way->type)),way->type&Way_OneWay?",One-Way":"",way->type&Way_Roundabout?",Roundabout":"");
 printf("  allow=%02x (%s)\n",way->allow,AllowedNameList(way->allow));
 if(way->props)
    printf("  props=%02x (%s)\n",way->props,PropertiesNameList(way->props));
 if(way->speed)
    printf("  speed=%d (%d km/hr)\n",way->speed,speed_to_kph(way->speed));
 if(way->weight)
    printf("  weight=%d (%.1f tonnes)\n",way->weight,weight_to_tonnes(way->weight));
 if(way->height)
    printf("  height=%d (%.1f m)\n",way->height,height_to_metres(way->height));
 if(way->width)
    printf("  width=%d (%.1f m)\n",way->width,width_to_metres(way->width));
 if(way->length)
    printf("  length=%d (%.1f m)\n",way->length,length_to_metres(way->length));
}


/*++++++++++++++++++++++++++++++++++++++
  Print out a header in OSM XML format.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_head_osm(void)
{
 printf("<?xml version='1.0' encoding='UTF-8'?>\n");
 printf("<osm version='0.6' generator='JOSM'>\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the contents of a node from the routing database in OSM XML format.

  Nodes *nodes The set of nodes to use.

  index_t item The node index to print.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_node_osm(Nodes* nodes,index_t item)
{
 double latitude,longitude;

 GetLatLong(nodes,item,&latitude,&longitude);

 if(IsSuperNode(nodes,item))
   {
    printf("  <node id='%lu' lat='%.7f' lon='%.7f' version='1'>\n",(unsigned long)item+1,radians_to_degrees(latitude),radians_to_degrees(longitude));
    printf("    <tag k='routino:super' v='yes' />\n");
    printf("  </node>\n");
   }
 else
    printf("  <node id='%lu' lat='%.7f' lon='%.7f' version='1' />\n",(unsigned long)item+1,radians_to_degrees(latitude),radians_to_degrees(longitude));
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the contents of a segment from the routing database as a way in OSM XML format.

  Segments *segments The set of segments to use.

  index_t item The segment index to print.

  Ways *ways The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_segment_osm(Segments *segments,index_t item,Ways *ways)
{
 Segment *segment=LookupSegment(segments,item);
 Way *way=LookupWay(ways,segment->way);
 int i;

 printf("  <way id='%lu' version='1'>\n",(unsigned long)item+1);

 if(IsOnewayTo(segment,segment->node1))
   {
    printf("    <nd ref='%lu' />\n",(unsigned long)segment->node2+1);
    printf("    <nd ref='%lu' />\n",(unsigned long)segment->node1+1);
   }
 else
   {
    printf("    <nd ref='%lu' />\n",(unsigned long)segment->node1+1);
    printf("    <nd ref='%lu' />\n",(unsigned long)segment->node2+1);
   }

 if(IsSuperSegment(segment))
    printf("    <tag k='routino:super' v='yes' />\n");
 if(IsNormalSegment(segment))
    printf("    <tag k='routino:normal' v='yes' />\n");

 if(way->type & Way_OneWay)
    printf("    <tag k='oneway' v='yes' />\n");
 if(way->type & Way_Roundabout)
    printf("    <tag k='junction' v='roundabout' />\n");

 printf("    <tag k='highway' v='%s' />\n",HighwayName(HIGHWAY(way->type)));

 if(IsNormalSegment(segment) && WayNamed(ways,way))
    printf("    <tag k='name' v='%s' />\n",ParseXML_Encode_Safe_XML(WayNameHighway(ways,way)));

 for(i=1;i<Transport_Count;i++)
    if(way->allow & ALLOWED(i))
       printf("    <tag k='%s' v='yes' />\n",TransportName(i));

 for(i=1;i<Property_Count;i++)
    if(way->props & PROPERTIES(i))
       printf("    <tag k='%s' v='yes' />\n",PropertyName(i));

 if(way->speed)
    printf("    <tag k='maxspeed' v='%d' />\n",speed_to_kph(way->speed));

 if(way->weight)
    printf("    <tag k='maxweight' v='%.1f' />\n",weight_to_tonnes(way->weight));
 if(way->height)
    printf("    <tag k='maxheight' v='%.1f' />\n",height_to_metres(way->height));
 if(way->width)
    printf("    <tag k='maxwidth' v='%.1f' />\n",width_to_metres(way->width));
 if(way->length)
    printf("    <tag k='maxlength' v='%.1f' />\n",length_to_metres(way->length));

 printf("  </way>\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out a tail in OSM XML format.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_tail_osm(void)
{
 printf("</osm>\n");
}


/*+ Conversion from time_t to date string (day of week). +*/
static const char* const weekdays[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

/*+ Conversion from time_t to date string (month of year). +*/
static const char* const months[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};


/*++++++++++++++++++++++++++++++++++++++
  Convert the time into an RFC 822 compliant date.

  char *RFC822Date Returns a pointer to a fixed string containing the date.

  time_t t The time.
  ++++++++++++++++++++++++++++++++++++++*/

static char *RFC822Date(time_t t)
{
 static char value[32];
 char weekday[4];
 char month[4];
 struct tm *tim;

 tim=gmtime(&t);

 strcpy(weekday,weekdays[tim->tm_wday]);
 strcpy(month,months[tim->tm_mon]);

 /* Sun, 06 Nov 1994 08:49:37 GMT    ; RFC 822, updated by RFC 1123 */

 sprintf(value,"%3s, %02d %3s %4d %02d:%02d:%02d %s",
         weekday,
         tim->tm_mday,
         month,
         tim->tm_year+1900,
         tim->tm_hour,
         tim->tm_min,
         tim->tm_sec,
         "GMT"
         );

 return(value);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the usage information.

  int detail The level of detail to use - 0 = low, 1 = high.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_usage(int detail)
{
 fprintf(stderr,
         "Usage: filedumper [--help]\n"
         "                  [--dir=<dirname>] [--prefix=<name>]\n"
         "                  [--statistics]\n"
         "                  [--visualiser --latmin=<latmin> --latmax=<latmax>\n"
         "                                --lonmin=<lonmin> --lonmax=<lonmax>\n"
         "                                --data=<data-type>]\n"
         "                  [--dump [--node=<node> ...]\n"
         "                          [--segment=<segment> ...]\n"
         "                          [--way=<way> ...]]\n"
         "                  [--dump-osm [--no-super]\n"
         "                              [--latmin=<latmin> --latmax=<latmax>\n"
         "                               --lonmin=<lonmin> --lonmax=<lonmax>]]\n");

 if(detail)
    fprintf(stderr,
            "\n"
            "--help                    Prints this information.\n"
            "\n"
            "--dir=<dirname>           The directory containing the routing database.\n"
            "--prefix=<name>           The filename prefix for the routing database.\n"
            "\n"
            "--statistics              Print statistics about the routing database.\n"
            "\n"
            "--visualiser              Extract selected data from the routing database:\n"
            "  --latmin=<latmin>       * the minimum latitude (degrees N).\n"
            "  --latmax=<latmax>       * the maximum latitude (degrees N).\n"
            "  --lonmin=<lonmin>       * the minimum longitude (degrees E).\n"
            "  --lonmax=<lonmax>       * the maximum longitude (degrees E).\n"
            "  --data=<data-type>      * the type of data to select.\n"
            "\n"
            "  <data-type> can be selected from:\n"
            "      junctions = segment count at each junction.\n"
            "      super     = super-node and super-segments.\n"
            "      oneway    = oneway segments.\n"
            "      speed     = speed limits.\n"
            "      weight    = weight limits.\n"
            "      height    = height limits.\n"
            "      width     = width limits.\n"
            "      length    = length limits.\n"
            "\n"
            "--dump                    Dump selected contents of the database.\n"
            "  --node=<node>           * the node with the selected number.\n"
            "  --segment=<segment>     * the segment with the selected number.\n"
            "  --way=<way>             * the way with the selected number.\n"
            "                          Use 'all' instead of a number to get all of them.\n"
            "\n"
            "--dump-osm                Dump all or part of the database as an XML file.\n"
            "  --no-super              * exclude the super-segments.\n"
            "  --latmin=<latmin>       * the minimum latitude (degrees N).\n"
            "  --latmax=<latmax>       * the maximum latitude (degrees N).\n"
            "  --lonmin=<lonmin>       * the minimum longitude (degrees E).\n"
            "  --lonmax=<lonmax>       * the maximum longitude (degrees E).\n");

 exit(!detail);
}
