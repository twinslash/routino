%{
/***************************************
 $Header: /home/amb/routino/src/RCS/xmlparse.l,v 1.17 2010/05/25 18:24:20 amb Exp $

 A simple generic XML parser where the structure comes from the function parameters.
 Not intended to be fully conforming to XML staandard or a validating parser but
 sufficient to parse OSM XML and simple program configuration files.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010 Andrew M. Bishop

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
#include <ctype.h>
#include <string.h>

#include "xmlparse.h"


/* Parser outputs */

#define LEX_EOF                    0

#define LEX_TAG_BEGIN              1
#define LEX_XML_DECL_BEGIN         2
#define LEX_TAG_POP                3
#define LEX_TAG_PUSH               4
#define LEX_XML_DECL_FINISH        6
#define LEX_TAG_FINISH             7
#define LEX_ATTR_KEY               8
#define LEX_ATTR_VAL               9

#define LEX_ERROR                100

#define LEX_ERROR_TAG_START      101
#define LEX_ERROR_XML_DECL_START 102
#define LEX_ERROR_TAG            103
#define LEX_ERROR_XML_DECL       104
#define LEX_ERROR_ATTR           105
#define LEX_ERROR_END_TAG        106
#define LEX_ERROR_COMMENT        107
#define LEX_ERROR_CLOSE          108
#define LEX_ERROR_ATTR_VAL       109
#define LEX_ERROR_ENTITY_REF     110
#define LEX_ERROR_CHAR_REF       111

#define LEX_ERROR_UNEXP_TAG      201
#define LEX_ERROR_UNBALANCED     202
#define LEX_ERROR_NO_START       203
#define LEX_ERROR_UNEXP_ATT      204
#define LEX_ERROR_UNEXP_EOF      205
#define LEX_ERROR_XML_NOT_FIRST  206

#define LEX_ERROR_CALLBACK       255


/* Lexer definitions */

#define YY_SKIP_YYWRAP 1 /* Remove error with prototype of ..._yywrap */
#ifndef yywrap
/*+ Needed in lex but does nothing. +*/
#define yywrap() 1
#endif

/*+ Reset the current string. +*/
#define reset_string \
 if(!string) string=(char*)malloc(16); \
 *string=0; \
 stringused=0;

/*+ append information to the current string. +*/
#define append_string(xx) \
 newlen=strlen(xx); \
 if((stringused+newlen)>=stringlen) \
    string=(char*)realloc((void*)string,stringlen=(stringused+newlen+16)); \
 strcpy(string+stringused,xx); \
 stringused+=newlen;

#define YY_NO_INPUT


/* Lexer functions and variables */

extern int yylex(void);

static char *yylval=NULL;

static int xmlparse_options;

%}

%option 8bit
%option pointer
%option batch
%option yylineno

%option nodefault
%option perf-report
%option fast
%option nounput


 /* Grammar based on http://www.w3.org/TR/2004/REC-xml-20040204/ but for ASCII not Unicode. */

S               [ \t\r\n]

letter          [a-zA-Z]
digit           [0-9]
xdigit          [a-fA-F0-9]

namechar        ({letter}|{digit}|[-._:])
name            ({letter}|[_:]){namechar}*

entityref       &{name};
charref         &#({digit}+|x{xdigit}+);


%x COMMENT
%x CDATA
%x DOCTYPE
%x XML_DECL_START XML_DECL
%x TAG_START TAG
%x ATTR_KEY ATTR_VAL
%x END_TAG1 END_TAG2
%x DQUOTED SQUOTED

%%
 /* Must use static variables since the parser returns often. */
 static char *string=NULL;
 static int stringlen=0,stringused=0;
 static int after_attr=0;
 int newlen;
 int doctype_depth=0;

 /* Handle top level entities */

"<!--"                      { BEGIN(COMMENT); }
"<![CDATA["                 { BEGIN(CDATA); }
"<!DOCTYPE"                 { BEGIN(DOCTYPE); doctype_depth=0; }
"</"                        { BEGIN(END_TAG1); }
"<?"                        { BEGIN(XML_DECL_START); }
"<"                         { BEGIN(TAG_START); }
">"                         { return(LEX_ERROR_CLOSE); }
[^<>]+                      { }

 /* Comments */

<COMMENT>"--->"             { return(LEX_ERROR_COMMENT); }
<COMMENT>"-->"              { BEGIN(INITIAL); }
<COMMENT>"--"[^->]+         { }
<COMMENT>[^-]+              { }
<COMMENT>"-"                { }

 /* CDATA */

<CDATA>"]]>"                { BEGIN(INITIAL); }
<CDATA>"]"                  { }
<CDATA>[^]]+                { }

 /* CDATA */

<DOCTYPE>"<"                { doctype_depth++; }
<DOCTYPE>">"                { if(doctype_depth==0) BEGIN(INITIAL); else doctype_depth--; }
<DOCTYPE>[^<>]+             { }

 /* XML Declaration start */

<XML_DECL_START>{name}      { BEGIN(XML_DECL); yylval=yytext; return(LEX_XML_DECL_BEGIN); }
<XML_DECL_START>.|\n        { return(LEX_ERROR_XML_DECL_START); }

 /* Tag middle */

<XML_DECL>"?>"              { BEGIN(INITIAL); return(LEX_XML_DECL_FINISH); }
<XML_DECL>{S}+              { }
<XML_DECL>{name}            { after_attr=XML_DECL; BEGIN(ATTR_KEY); yylval=yytext; return(LEX_ATTR_KEY); }
<XML_DECL>.|\n              { return(LEX_ERROR_XML_DECL); }

 /* Any tag start */

<TAG_START>{name}           { BEGIN(TAG); yylval=yytext; return(LEX_TAG_BEGIN); }
<TAG_START>.|\n             { return(LEX_ERROR_TAG_START); }

 /* End-tag start */

<END_TAG1>{name}            { BEGIN(END_TAG2); yylval=yytext; return(LEX_TAG_POP); }
<END_TAG1>.|\n              { return(LEX_ERROR_END_TAG); }

<END_TAG2>">"               { BEGIN(INITIAL); }
<END_TAG2>.|\n              { return(LEX_ERROR_END_TAG); }

 /* Any tag middle */

<TAG>"/>"                   { BEGIN(INITIAL); return(LEX_TAG_FINISH); }
<TAG>">"                    { BEGIN(INITIAL); return(LEX_TAG_PUSH); }
<TAG>{S}+                   { }
<TAG>{name}                 { after_attr=TAG; BEGIN(ATTR_KEY); yylval=yytext; return(LEX_ATTR_KEY); }
<TAG>.|\n                   { return(LEX_ERROR_TAG); }

 /* Attributes */

<ATTR_KEY>=                 { BEGIN(ATTR_VAL); }
<ATTR_KEY>.|\n              { return(LEX_ERROR_ATTR); }

<ATTR_VAL>\"                { BEGIN(DQUOTED); reset_string; }
<ATTR_VAL>\'                { BEGIN(SQUOTED); reset_string; }
<ATTR_VAL>.|\n              { return(LEX_ERROR_ATTR); }

 /* Quoted strings */

<DQUOTED>\"                 { BEGIN(after_attr); yylval=string; return(LEX_ATTR_VAL); }
<DQUOTED>{entityref}        { if(xmlparse_options&XMLPARSE_RETURN_ATTR_ENCODED) {append_string(yytext);}
                              else { const char *str=ParseXML_Decode_Entity_Ref(yytext); if(str) {append_string(str);} else {yylval=yytext; return(LEX_ERROR_ENTITY_REF);} } }
<DQUOTED>{charref}          { if(xmlparse_options&XMLPARSE_RETURN_ATTR_ENCODED) {append_string(yytext);}
                              else { const char *str=ParseXML_Decode_Char_Ref(yytext);   if(str) {append_string(str);} else {yylval=yytext; return(LEX_ERROR_CHAR_REF);} } }
<DQUOTED>[<>&]              { yylval=yytext; return(LEX_ERROR_ATTR_VAL); }
<DQUOTED>[^<>&\"]+          { append_string(yytext); }

<SQUOTED>\'                 { BEGIN(after_attr); yylval=string; return(LEX_ATTR_VAL); }
<SQUOTED>{entityref}        { if(xmlparse_options&XMLPARSE_RETURN_ATTR_ENCODED) {append_string(yytext);}
                              else { const char *str=ParseXML_Decode_Entity_Ref(yytext); if(str) {append_string(str);} else {yylval=yytext; return(LEX_ERROR_ENTITY_REF);} } }
<SQUOTED>{charref}          { if(xmlparse_options&XMLPARSE_RETURN_ATTR_ENCODED) {append_string(yytext);}
                              else { const char *str=ParseXML_Decode_Char_Ref(yytext);   if(str) {append_string(str);} else {yylval=yytext; return(LEX_ERROR_CHAR_REF);} } }
<SQUOTED>[<>&]              { yylval=yytext; return(LEX_ERROR_ATTR_VAL); }
<SQUOTED>[^<>&\']+          { append_string(yytext); }

 /* End of file */

<<EOF>>                     { free(string); string=NULL; stringlen=stringused=0; BEGIN(INITIAL); return(LEX_EOF); }

%%


/*++++++++++++++++++++++++++++++++++++++
  A function to call the callback function with the parameters needed.

  int call_callback Returns 1 if the callback returned with an error.

  const char *name The name of the tag.

  int (*callback)() The callback function.

  int type The type of tag (start and/or end).

  int nattributes The number of attributes collected.

  char *attributes[XMLPARSE_MAX_ATTRS] The list of attributes.
  ++++++++++++++++++++++++++++++++++++++*/

static inline int call_callback(const char *name,int (*callback)(),int type,int nattributes,char *attributes[XMLPARSE_MAX_ATTRS])
{
 switch(nattributes)
   {
   case  0: return (*callback)(name,type);
   case  1: return (*callback)(name,type,attributes[0]);
   case  2: return (*callback)(name,type,attributes[0],attributes[1]);
   case  3: return (*callback)(name,type,attributes[0],attributes[1],attributes[2]);
   case  4: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3]);
   case  5: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4]);
   case  6: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5]);
   case  7: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6]);
   case  8: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7]);
   case  9: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8]);
   case 10: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9]);
   case 11: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10]);
   case 12: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11]);
   case 13: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12]);
   case 14: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12],attributes[13]);
   case 15: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12],attributes[13],attributes[14]);
   case 16: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12],attributes[13],attributes[14],attributes[15]);

   default:
    fprintf(stderr,"XML Parser: Error on line %d: too many attributes for tag '%s' source code needs changing.\n",yylineno,name);
    exit(1);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Parse the XML and call the functions for each tag as seen.

  int ParseXML Returns 0 if OK or something else in case of an error.

  FILE *file The file to parse.

  xmltag **tags The array of pointers to tags for the top level.

  int options A list of XML Parser options OR-ed together.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXML(FILE *file,xmltag **tags,int options)
{
 int yychar,i;

 char *attributes[XMLPARSE_MAX_ATTRS]={NULL};
 int attribute=0;

 int stackdepth=0,stackused=0;
 xmltag ***tags_stack=NULL;
 xmltag **tag_stack=NULL;
 xmltag *tag=NULL;

 /* The actual parser. */

 xmlparse_options=options;

 yyin=file;

 yyrestart(yyin);

 yylineno=1;

 BEGIN(INITIAL);

 do
   {
    yychar=yylex();

    switch(yychar)
      {
       /* The start of a tag for an XML declaration */

      case LEX_XML_DECL_BEGIN:

       if(tag_stack)
         {
          fprintf(stderr,"XML Parser: Error on line %d: XML declaration not before all other tags.\n",yylineno);
          yychar=LEX_ERROR_XML_NOT_FIRST;
          break;
         }

       /* The start of a tag for an element */

      case LEX_TAG_BEGIN:

       tag=NULL;

       for(i=0;tags[i];i++)
          if(!strcasecmp(yylval,tags[i]->name))
            {
             tag=tags[i];

             for(i=0;i<tag->nattributes;i++)
                if(attributes[i])
                  {
                   free(attributes[i]);
                   attributes[i]=NULL;
                  }

             break;
            }

       if(tag==NULL)
         {
          fprintf(stderr,"XML Parser: Error on line %d: unexpected tag '%s'.\n",yylineno,yylval);
          yychar=LEX_ERROR_UNEXP_TAG;
         }

       break;

       /* The end of the start-tag for an element */

      case LEX_TAG_PUSH:

       if(stackused==stackdepth)
         {
          tag_stack =(xmltag**) realloc((void*)tag_stack ,(stackdepth+=8)*sizeof(xmltag*));
          tags_stack=(xmltag***)realloc((void*)tags_stack,(stackdepth+=8)*sizeof(xmltag**));
         }

       tag_stack [stackused]=tag;
       tags_stack[stackused]=tags;
       stackused++;

       if(tag->callback)
          if(call_callback(tag->name,tag->callback,XMLPARSE_TAG_START,tag->nattributes,attributes))
             yychar=LEX_ERROR_CALLBACK;

       tags=tag->subtags;

       break;

       /* The end of the empty-element-tag for an XML declaration */

      case LEX_XML_DECL_FINISH:

       /* The end of the empty-element-tag for an element */

      case LEX_TAG_FINISH:

       if(tag->callback)
          if(call_callback(tag->name,tag->callback,XMLPARSE_TAG_START|XMLPARSE_TAG_END,tag->nattributes,attributes))
             yychar=LEX_ERROR_CALLBACK;

       if(stackused>0)
          tag=tag_stack[stackused-1];
       else
          tag=NULL;

       break;

       /* The end of the end-tag for an element */

      case LEX_TAG_POP:

       stackused--;
       tags=tags_stack[stackused];
       tag =tag_stack [stackused];

       if(strcmp(tag->name,yylval))
         {
          fprintf(stderr,"XML Parser: Error on line %d: end tag '</%s>' doesn't match start tag '<%s ...>'.\n",yylineno,yylval,tag->name);
          yychar=LEX_ERROR_UNBALANCED;
         }

       if(stackused<0)
         {
          fprintf(stderr,"XML Parser: Error on line %d: end tag '</%s>' seen but there was no start tag '<%s ...>'.\n",yylineno,yylval,yylval);
          yychar=LEX_ERROR_NO_START;
         }

       for(i=0;i<tag->nattributes;i++)
          if(attributes[i])
            {
             free(attributes[i]);
             attributes[i]=NULL;
            }

       if(tag->callback)
          if(call_callback(tag->name,tag->callback,XMLPARSE_TAG_END,tag->nattributes,attributes))
             yychar=LEX_ERROR_CALLBACK;

       if(stackused>0)
          tag=tag_stack[stackused-1];
       else
          tag=NULL;

       break;

       /* An attribute key */

      case LEX_ATTR_KEY:

       attribute=-1;

       for(i=0;i<tag->nattributes;i++)
          if(!strcasecmp(yylval,tag->attributes[i]))
            {
             attribute=i;

             break;
            }

       if(attribute==-1)
         {
          if((options&XMLPARSE_UNKNOWN_ATTRIBUTES)==XMLPARSE_UNKNOWN_ATTR_ERROR ||
             ((options&XMLPARSE_UNKNOWN_ATTRIBUTES)==XMLPARSE_UNKNOWN_ATTR_ERRNONAME && !strchr(yylval,':')))
            {
             fprintf(stderr,"XML Parser: Error on line %d: unexpected attribute '%s' for tag '%s'.\n",yylineno,yylval,tag->name);
             yychar=LEX_ERROR_UNEXP_ATT;
            }
          else if((options&XMLPARSE_UNKNOWN_ATTRIBUTES)==XMLPARSE_UNKNOWN_ATTR_WARN)
             fprintf(stderr,"XML Parser: Warning on line %d: unexpected attribute '%s' for tag '%s'.\n",yylineno,yylval,tag->name);
         }

       break;

       /* An attribute value */

      case LEX_ATTR_VAL:

       if(tag->callback && attribute!=-1 && yylval)
          attributes[attribute]=strcpy(malloc(strlen(yylval)+1),yylval);

       break;

       /* End of file */

      case LEX_EOF:

       if(tag)
         {
          fprintf(stderr,"XML Parser: Error on line %d: end of file seen without end tag '</%s>'.\n",yylineno,tag->name);
          yychar=LEX_ERROR_UNEXP_EOF;
         }

       break;

      case LEX_ERROR_TAG_START:
       fprintf(stderr,"XML Parser: Error on line %d: character '<' seen not at start of tag.\n",yylineno);
       break;

      case LEX_ERROR_XML_DECL_START:
       fprintf(stderr,"XML Parser: Error on line %d: characters '<?' seen not at start of XML declaration.\n",yylineno);
       break;

      case LEX_ERROR_TAG:
       fprintf(stderr,"XML Parser: Error on line %d: invalid character seen inside tag '<%s...>'.\n",yylineno,tag->name);
       break;

      case LEX_ERROR_XML_DECL:
       fprintf(stderr,"XML Parser: Error on line %d: invalid character seen inside XML declaration '<?%s...>'.\n",yylineno,tag->name);
       break;

      case LEX_ERROR_ATTR:
       fprintf(stderr,"XML Parser: Error on line %d: invalid attribute definition seen in tag.\n",yylineno);
       break;

      case LEX_ERROR_END_TAG:
       fprintf(stderr,"XML Parser: Error on line %d: invalid character seen in end-tag.\n",yylineno);
       break;

      case LEX_ERROR_COMMENT:
       fprintf(stderr,"XML Parser: Error on line %d: invalid comment seen.\n",yylineno);
       break;

      case LEX_ERROR_CLOSE:
       fprintf(stderr,"XML Parser: Error on line %d: character '>' seen not at end of tag.\n",yylineno);
       break;

      case LEX_ERROR_ATTR_VAL:
       fprintf(stderr,"XML Parser: Error on line %d: invalid character '%s' seen in attribute value.\n",yylineno,yylval);
       break;

      case LEX_ERROR_ENTITY_REF:
       fprintf(stderr,"XML Parser: Error on line %d: invalid entity reference '%s' seen in attribute value.\n",yylineno,yylval);
       break;

      case LEX_ERROR_CHAR_REF:
       fprintf(stderr,"XML Parser: Error on line %d: invalid character reference '%s' seen in attribute value.\n",yylineno,yylval);
       break;
      }
   }
 while(yychar>LEX_EOF && yychar<LEX_ERROR);

 /* Delete the tagdata */

 for(i=0;i<XMLPARSE_MAX_ATTRS;i++)
    if(attributes[i])
       free(attributes[i]);

 if(stackdepth)
   {
    free(tag_stack);
    free(tags_stack);
   }

 return(yychar);
}


/*++++++++++++++++++++++++++++++++++++++
  Return the current parser line number.

  unsigned long ParseXML_LineNumber Returns the line number.
  ++++++++++++++++++++++++++++++++++++++*/

unsigned long ParseXML_LineNumber(void)
{
 return(yylineno);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert an XML entity reference into an ASCII string.

  char *ParseXML_Decode_Entity_Ref Returns a pointer to the replacement decoded string.

  const char *string The entity reference string.
  ++++++++++++++++++++++++++++++++++++++*/

char *ParseXML_Decode_Entity_Ref(const char *string)
{
 if(!strcmp(string,"&amp;"))  return("&");
 if(!strcmp(string,"&lt;"))   return("<");
 if(!strcmp(string,"&gt;"))   return(">");
 if(!strcmp(string,"&apos;")) return("'");
 if(!strcmp(string,"&quot;")) return("\"");
 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert an XML character reference into an ASCII string.

  char *ParseXML_Decode_Char_Ref Returns a pointer to the replacement decoded string.

  const char *string The character reference string.
  ++++++++++++++++++++++++++++++++++++++*/

char *ParseXML_Decode_Char_Ref(const char *string)
{
 static char result[2]=" ";
 long int val;

 if(string[2]=='x') val=strtol(string+3,NULL,16);
 else               val=strtol(string+2,NULL,10);

 if(val<0 || val>255)
    return(NULL);

 result[0]=val&0xff;

 return(result);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string into something that is safe to output in an XML file.

  char *ParseXML_Encode_Safe_XML Returns a pointer to the replacement encoded string (or the original if no change needed).

  const char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

char *ParseXML_Encode_Safe_XML(const char *string)
{
 static const char hexstring[17]="0123456789ABCDEF";
 int i=0,j=0,len;
 char *result;

 for(i=0;string[i];i++)
    if(string[i]=='<' || string[i]=='>' || string[i]=='&' || string[i]=='\'' || string[i]=='"' || string[i]<32 || string[i]>126)
       break;

 if(!string[i])
    return((char*)string);

 len=i+256-6;

 result=(char*)malloc(len+7);
 strncpy(result,string,j=i);

 do
   {
    for(;j<len && string[i];i++)
       if(string[i]=='<')
         {
          result[j++]='&';
          result[j++]='l';
          result[j++]='t';
          result[j++]=';';
         }
       else if(string[i]=='>')
         {
          result[j++]='&';
          result[j++]='g';
          result[j++]='t';
          result[j++]=';';
         }
       else if(string[i]=='&')
         {
          result[j++]='&';
          result[j++]='a';
          result[j++]='m';
          result[j++]='p';
          result[j++]=';';
         }
       else if(string[i]=='\'')
         {
          result[j++]='&';
          result[j++]='a';
          result[j++]='p';
          result[j++]='o';
          result[j++]='s';
          result[j++]=';';
         }
       else if(string[i]=='"')
         {
          result[j++]='&';
          result[j++]='q';
          result[j++]='u';
          result[j++]='o';
          result[j++]='t';
          result[j++]=';';
         }
       else if(string[i]<32 || string[i]>126)
         {
          result[j++]='&';
          result[j++]='#';
          result[j++]='x';
          result[j++]=hexstring[(string[i]&0xf0)>>4];
          result[j++]=hexstring[ string[i]&0x0f    ];
          result[j++]=';';
         }
       else
          result[j++]=string[i];

    if(string[i])                  /* Not finished */
      {
       len+=256;
       result=(char*)realloc((void*)result,len+7);
      }
   }
 while(string[i]);

 result[j]=0;

 return(result);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a integer (checking that it really is a integer).

  int ParseXML_GetInteger Returns 1 if a integer could be found or 0 otherwise.

  const char *string The string to be parsed.

  int *number Returns the number.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXML_GetInteger(const char *string,int *number)
{
 const char *p=string;

 if(*p=='-' || *p=='+')
    p++;

 while(isdigit(*p))
    p++;

 if(*p)
    return(0);

 *number=atoi(string);

 return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string to a floating point number (checking that it really is a number).

  int ParseXML_GetFloating Returns 1 if a number could be found or 0 otherwise.

  const char *string The string to be parsed.

  int *number Returns the number.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXML_GetFloating(const char *string,double *number)
{
 const char *p=string;

 if(*p=='-' || *p=='+')
    p++;

 while(isdigit(*p) || *p=='.')
    p++;

 if(*p=='e' || *p=='E')
   {
    p++;

    if(*p=='-' || *p=='+')
       p++;

    while(isdigit(*p))
       p++;
   }

 if(*p)
    return(0);

 *number=atof(string);

 return(1);
}
