# Makefile for EZ-Draw, on Unix and Windows
#
# 12/02/2016 - Edouard.Thiel@lif.univ-mrs.fr


# Default system type. Override this on command line: make SYSTYPE=WIN32 ...
SYSTYPE = UNIX

#-------------------------- C O N F I G U R A T I O N -------------------------

ifeq ($(SYSTYPE),UNIX)

    # Unix/X11 systems (linux, Mac OS X, Cygwin, etc)               # SECTION A
    CC     = gcc
    CFLAGS = -Wall -W -std=c99 -pedantic -O2 -g 
    LIBS   = -lX11 -lXext
    LIBS_I =

else ifeq ($(SYSTYPE),WIN32)

    # Windows, all versions                                         # SECTION B
    CC     = gcc
    CFLAGS = -Wall -W -std=c99 -pedantic -O2
    LIBS   = -lgdi32
    LIBS_I = -lmsimg32

endif

# Add your executables at the end of the lists;                     # SECTION C
#   EXECS     to use ez-draw.o
#   EXECS_M   to use ez-draw.o and -lm
#   EXECS_IM  to use ez-draw.o, ez-image.o and -lm

EXECS = demo-01 demo-02 demo-03 demo-04 demo-05 demo-06 demo-07 demo-08 \
        demo-09 demo-11 jeu-sudoku jeu-nim jeu-vie jeu-taquin jeu-2048 \
        jeu-tetris

EXECS_M = demo-10 jeu-laby jeu-ezen jeu-heziom jeu-tangram

EXECS_IM = demo-12 demo-13 demo-14 demo-15 demo-16 demo-17 \
           jeu-bubblet jeu-doodle

# If your program needs extra modules, add the program              # SECTION D
# name in EXECS_PRO, the modules.o in OBJS_PRO, and the 
# libraries -l... in LIBS_PRO:

EXECS_PRO =
OBJS_PRO  =
LIBS_PRO  =

#----------------- E N D   O F   C O N F I G U R A T I O N --------------------

# Other variables
ifeq ($(SYSTYPE),UNIX)
    SHELL  = /bin/bash
    RM     = rm -f
    EXT    =
else ifeq ($(SYSTYPE),WIN32)
    SHELL  = 
    RM     = del
    EXT    = .exe
else
    $(error Wrong SYSTYPE - expect UNIX or WIN32)
endif

# System dependent settings
ifeq ($(SYSTYPE),UNIX)
    UNAME_S := $(shell uname -s)
    ifneq (,$(findstring CYGWIN,$(UNAME_S)))
        # On Cygwin, replace -std=c99 by -std=gnu99 to avoid warnings 
        # on random and srandom
    	CFLAGS := $(patsubst -std=c99,-std=gnu99,$(CFLAGS))
    endif
else ifeq ($(SYSTYPE),WIN32)
    # On Windows, automaticaly append "$(EXT)" to the executables
    EXECS     := $(patsubst %,%$(EXT),$(EXECS))
    EXECS_M   := $(patsubst %,%$(EXT),$(EXECS_M))
    EXECS_IM  := $(patsubst %,%$(EXT),$(EXECS_IM))
    EXECS_PRO := $(patsubst %,%$(EXT),$(EXECS_PRO))
endif

EXECS_ALL = $(EXECS) $(EXECS_M) $(EXECS_IM) $(EXECS_PRO)

%.o : %.c
	$(CC) -c $(CFLAGS) $*.c

help ::
	@echo "Options for make : help all clean distclean"

all :: $(EXECS_ALL)


# This double dependency is GNU-make syntax; it is equivalent to
#     demo0 : demo0.o ez-draw.o
#     demo1 : demo1.o ez-draw.o
#     ...
#     $(EXECS) :
#	      $(CC) -o $@ $^ $(LIBS)

$(EXECS) : %$(EXT) : %.o ez-draw.o
	$(CC) -o $@ $^ $(LIBS)

$(EXECS_M) : %$(EXT) : %.o ez-draw.o
	$(CC) -o $@ $^ $(LIBS) -lm

$(EXECS_IM) : %$(EXT) : %.o ez-draw.o ez-image.o
	$(CC) -o $@ $^ $(LIBS) $(LIBS_I) -lm

$(EXECS_PRO) : %$(EXT) : %.o $(OBJS_PRO)
	$(CC) -o $@ $^ $(LIBS) $(LIBS_I) $(LIBS_PRO)

clean ::
	$(RM) *.o core

distclean :: clean
	$(RM) *~ .*.swp $(EXECS_ALL)


# If you copy this Makefile in a new directory, you can cut the following
# "private" section and clear the EXECS lists in sections C and D.


#------------------------------- P R I V A T E --------------------------------

# This target creates a link $HOME/ez-draw to your home directory
ln ::
	@P=`pwd` ; N="$${HOME}/ez-draw" ;\
	if [ -h "$$N" ]; then mv -f "$$N" "$$N.old" ; fi ;\
	echo "ln -s \"$$P\" \"$$N\"" ; ln -s "$$P" "$$N" 

# Tarball creation in parent directory
tar :: distclean
	@N=`pwd` ; N=`basename "$$N"` ;\
	cd .. ; P=`pwd` ;\
	tar cvfz $$N.tgz $$N --exclude='svg*'  --exclude='rsync*' ;\
	echo "DONE: tar cvfz $$P/$$N.tgz $$N" ;\
	ls -l $$P/$$N.tgz

SAP = Edouard.Thiel@saphir2.lidil.univ-mrs.fr

# Backup on SAP
sap :: distclean
	@N=`pwd` ; N=`basename "$$N"` ;\
	D=`date "+%Y-%m-%d- %H" | awk '{ printf "%s", $$1 ; printf "%c", $$2+97 }'` ;\
	cd .. ; P=`pwd` ; F="$${N}_$${D}.tgz" ;\
	tar cvfz $$F $$N --exclude='svg*' --exclude='rsync*' ;\
	echo "DONE: tar cvfz $$F $$N" ;\
        echo "Copying $$F to $(SAP) ..." ;\
        scp "$$F" "$(SAP):PROG/EZ-DRAW" ;\

# Update web page - final version
web :: tar
	@P=`pwd` ; N=`basename "$$P"` ;	t="$${HOME}/public_html" ;\
	if [ -f "$$t/index.html" ]; then \
	  echo "Updating \"$$t ...\"" ;\
	  cp -f "../$$N.tgz" "$$t" ;\
	  ( cd "$$t" ;\
	    if [ -d "$$N" ]; then rm -rf "$$N" ; fi ;\
	    tar xvfz "$$N.tgz" ;\
	    if [ -h "ez-draw" ]; then rm -f "ez-draw" ; fi ;\
	    ln -s "$$N" "ez-draw" ;\
	    ./GenWeb.sh --gen "ez-draw/index.html" --gen "$$N/index-en.html" ;\
	  ) ;\
          echo "Updating web ..." ;\
          scp -r "$${t}/ez-draw" "$${t}/$$N.tgz" "$(SAP):public_html/" ;\
	fi ;\
	echo "done"

# Update web page - development version
pre :: tar
	@P=`pwd` ; N=`basename "$$P"` ;	t="$${HOME}/public_html" ;\
	if [ -f "$$t/index.html" ]; then \
	  echo "Updating \"$$t ...\"" ;\
	  cp -f "../$$N.tgz" "$$t" ;\
	  ( cd "$$t" ;\
	    if [ -d "$$N" ]; then rm -rf "$$N" ; fi ;\
	    tar xvfz "$$N.tgz" ;\
	    ./GenWeb.sh --gen "$$N/index.html" --gen "$$N/index-en.html" ;\
	  ) ;\
          echo "Updating web ..." ;\
          scp -r "$${t}/$$N" "$${t}/$$N.tgz" "$(SAP):public_html/" ;\
	fi ;\
	echo "done"

# Check ez-image.c using valgrind
test-vg-image ::
	@for i in ../images_test/* ; do \
	  valgrind --tool=memcheck --leak-check=yes --show-reachable=yes \
	           --num-callers=20 --track-fds=yes ./demo-13 "$$i" ;\
	  echo ; echo "Press [Enter] ..." ; read ;\
	done

# Check sources encoding
mime :: distclean
	file --mime-encoding *

# Check tabs absence
tabs ::
	@grep -P '\t' *.c *.h README || echo "No tab detected"

# Create documentation. Type: make docclean doc

LANGUAGES = fr en

doc ::
	@for lang in $(LANGUAGES) ; do \
	    (cd doc && make LANGUAGE=$$lang html dist) ;\
	done

docclean ::
	@for lang in $(LANGUAGES) ; do \
	    (cd doc && make LANGUAGE=$$lang clean) ;\
	done

