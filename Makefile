#
#	Makefile
#	This file is part of riss.
#	
#	26.08.2010
#	Copyright 2010 Norbert Manthey
#
DATE="`date +%y%m%d%H%M%S`"

#commandline
CMD = -O3 -DSILENT
ARGS = 

#compiler
CPP = g++ $(ARGS)
#	icpc -fast -Wall -DCOMPILE_INTEL

#compiler flags
LIBS		=	
CPPINCLUDES 	= -I./include -I../libPD/src/runtime/include
COMMON_CPPFLAGS = $(CPPINCLUDES)
CPPFLAGS 	= $(COMMON_CPPFLAGS) -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -g3 -fno-exceptions -fno-strict-aliasing #-DNDEBUG #
LDFLAGS 	= -L. -ldl -lpthread -static

# add some parameter for specific targets
riss: CPPFLAGS += -DNDEBUG
priss: CPPFLAGS += -DPARALLEL -DNDEBUG
rissExp: ARGS += -DBLOCKING_LIT -DNDEBUG
	

#
# section below includes all the necessay rules for building riss
#

#build variables
SRCS = $(wildcard src/*/*.cc src/*/*/*.cc src/*.cc)
OBJS = $(SRCS:.cc=.o)
DEPS = $(SRCS:.cc=.d)
LIBSRCS = $(wildcard src/clauseactivity/*.cc src/analyze/*.cc src/preprocessor/*.cc src/unitpropagation/*.cc src/decisionheuristic/*.cc src/eventheuristic/*.cc)
LIBOBJS = $(LIBSRCS:.cc=.lo)
LIBFILES = $(LIBSRCS:.cc=.so)

all: riss

#build solver
riss: $(OBJS)
	$(CPP) -o $@ $(OBJS) $(LIBS) $(LDFLAGS) $(CMD) $(CPPFLAGS)
	strip riss

# for competition
rissExp: riss
	mv riss rissExp

riss-libpd: $(OBJS)
	$(CPP) -static -o $@ $(OBJS) $(LIBS) -lpd $(LIBPD_CPPFLAGS) $(LDFLAGS) -lpthread -D _LIBPD_ACTIVATED $(CMD) $(CPPFLAGS)

priss: $(OBJS)
	$(CPP) -o $@ $(OBJS) $(LIBS) $(LDFLAGS) $(CMD) $(CPPFLAGS)
	strip priss

src/info.cc: always

# build lib objects (no supported at the moment)
%.lo:%.cc
	$(CPP) -MD $(CMD) $(CPPFLAGS) -DCOMPILE_LIBRARY -fno-stack-protector -fPIC -c -o $@ $<

# build libs (no supported at the moment)
%.so:%.lo
	$(CPP) -lc -lm -shared -Wl,-soname,$@ -o $@ $<

# special handling for multithreading
src/utils/waitfunc.o: src/utils/waitfunc.cc
	$(CPP) -MD $(CMD) $(CPPFLAGS) -c -o $@ $< -O0
	
#build object file and dependencies files
.cc.o:
	$(CPP) $(CMD) -MD $(CPPFLAGS) -c -o $@ $<

# create a tar from the source
tar: clean
	tar --exclude=".svn" -czvf riss.tar.gz src include Makefile COPYING README *.sh

always:
#	echo "#include \"info.h\"" > src/info.cc
	echo "const char* builttime = \"`date +%y%m%d%H%M%S`\";" > src/info.cc
	echo "const char* kernel=\"`uname -r`\";" >> src/info.cc
	echo "const char* revision=\"`svn info | grep evision`\";" >> src/info.cc
	echo "const char* buildflags=\"$(CPP) $(CPPFLAGS) $(LDFLAGS) $(CMD)\";" >> src/info.cc
	echo "const char* hostname=\"`hostname`\";" >> src/info.cc

# clean up backups and old files
clean:
	rm -f *~ */*~ */*/*~
	rm -f $(OBJS) $(DEPS) $(LIBOBJS) $(LIBFILES)
	rm -f riss priss riss-libpd CrissSP jlib/libjavariss.so
	rm -f include/java/sat/Solver.class include/java/sat_Solver.h
	rm -f log.txt
	echo "" > src/info.cc
	cd docu; rm -fR html

# build the whole solver from scratch
new: clean all

# include headerfile dependencies for sources
-include $(DEPS)
