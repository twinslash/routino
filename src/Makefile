# $Header: /home/amb/routino/src/RCS/Makefile,v 1.36 2010/07/09 17:43:00 amb Exp $
#
# Source code Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2010 Andrew M. Bishop
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Programs

CC=gcc
LD=gcc

LEX=flex

# Program options

CFLAGS=-Wall -Wmissing-prototypes
#CFLAGS+= -Wextra -pedantic -std=c99
LDFLAGS=-lm -lc

CFLAGS+= -O3
#CFLAGS+= -O0 -g
#CFLAGS+= -pg
#CFLAGS+= --coverage

LDFLAGS+=
#LDFLAGS+= -pg -static
#LDFLAGS+= --coverage

LEXFLAGS=

# Required to use stdio with files > 2GiB on 32-bit system.

FLAGS64=-D_FILE_OFFSET_BITS=64

# Compilation targets

C=$(wildcard *.c)
D=$(foreach f,$(C),$(addprefix .deps/,$(addsuffix .d,$(basename $f))))

EXE=planetsplitter router filedumper tagmodifier

WEBDIR=../web/bin

########

all : $(EXE)
	-@[ -d $(WEBDIR) ] && \
	  for file in $(EXE); do \
	     if [ ! -f $(WEBDIR)/$$file ] || [ $$file -nt $(WEBDIR)/$$file ]; then \
	        echo cp $$file $(WEBDIR) ;\
	        cp -f $$file $(WEBDIR) ;\
	     fi ;\
	  done
	@cd xml && $(MAKE) CC="$(CC)" LD="$(LD)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"

########

PLANETSPLITTER_OBJ=planetsplitter.o \
	           nodesx.o segmentsx.o waysx.o superx.o \
	           ways.o types.o \
	           files.o \
	           results.o queue.o sorting.o \
	           xmlparse.o tagging.o osmparser.o

planetsplitter : $(PLANETSPLITTER_OBJ)
	$(LD) $(PLANETSPLITTER_OBJ) -o $@ $(LDFLAGS)

########

ROUTER_OBJ=router.o \
	   nodes.o segments.o ways.o types.o \
	   files.o profiles.o xmlparse.o \
	   optimiser.o output.o results.o queue.o translations.o

router : $(ROUTER_OBJ)
	$(LD) $(ROUTER_OBJ) -o $@ $(LDFLAGS)

########

FILEDUMPER_OBJ=filedumper.o \
	       nodes.o segments.o ways.o types.o \
	       files.o xmlparse.o \
               visualiser.o

filedumper : $(FILEDUMPER_OBJ)
	$(LD) $(FILEDUMPER_OBJ) -o $@ $(LDFLAGS)

########

TAGMODIFIER_OBJ=tagmodifier.o \
	        files.o \
                xmlparse.o tagging.o

tagmodifier : $(TAGMODIFIER_OBJ)
	$(LD) $(TAGMODIFIER_OBJ) -o $@ $(LDFLAGS)

########

xmlparse.c : xmlparse.l
	$(LEX) $(LEXFLAGS)  $<
	-@mv lex.yy.c xmlparse.c
	@echo Created xmlparse.c

########

%.o : %.c
	$(CC) -c $(CFLAGS) $(FLAGS64) $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $<)))

########

clean:
	rm -f *.o
	rm -f *~
	rm -f xmlparse.c
	cd xml && $(MAKE) clean

########

distclean: clean
	-[ -d ../web/bin ] && cd ../web/bin/ && rm -f $(EXE)
	-rm -f $(EXE)
	-rm -f $(D)
	-rm -fr .deps
	cd xml && $(MAKE) distclean

########

.deps : .FORCE
	@[ -d .deps ] || mkdir $@

$(D) : .deps
	@touch $@

include $(D)

########

.FORCE :
