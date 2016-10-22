#
# Makefile for the RedBase project.
#
# Author: Hyunjung Park (hyunjung@cs.stanford.edu)
#


#
# Global variables
#
GTEST_DIR = ../lib/googletest/googletest
# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
CC             = g++
BUILD_DIR      = ../build/
LIB_DIR        = ../build/lib/
INC_DIRS       = -I. -I$(GTEST_DIR)/include/
AR             = ar -rc
RANLIB         = ranlib
YACC           = bison -dy
LEX            = flex

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# -g - Debugging information
# -O1 - Basic optimization
# -Wall - All warnings
# -DDEBUG_PF - This turns on the LOG file for lots of BufferMgr info
CFLAGS         = -g -Wall -pthread $(STATS_OPTION) $(INC_DIRS)

# The STATS_OPTION can be set to -DPF_STATS or to nothing to turn on and
# off buffer manager statistics.  The student should not modify this
# flag at all!
STATS_OPTION   = -DPF_STATS

#
# Students: Please modify SOURCES variables as needed.
#
PF_SOURCES     = pf_buffermgr.cc pf_error.cc pf_filehandle.cc \
                 pf_pagehandle.cc pf_hashtable.cc pf_manager.cc \
                 pf_statistics.cc statistics.cc
RM_SOURCES     = rm_manager.cc rm_filescan.cc rm_filehandle.cc \
								 rm_pagehandle.cc rm_record.cc rm_rid.cc
IX_SOURCES     =
SM_SOURCES     = #sm_stub.cc printer.cc
QL_SOURCES     = #ql_manager_stub.cc
UTILS_SOURCES  = #dbcreate.cc dbdestroy.cc redbase.cc
PARSER_SOURCES = #scan.c parse.c nodes.c interp.c
#TESTER_SOURCES = pf_test1.cc pf_test2.cc pf_test3.cc #rm_test.cc ix_test.cc parser_test.cc
TESTER_SOURCES = rm_rid_test.cc rm_record_test.cc

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
$(BUILD_DIR)gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc -o $@

$(BUILD_DIR)gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc -o $@

LIBRARY_GTEST = $(LIB_DIR)libgtest.a
$(LIBRARY_GTEST) : $(BUILD_DIR)gtest-all.o
	$(AR) $@ $^
	$(RANLIB) $@

LIBRARY_GTEST_MAIN = $(LIB_DIR)libgtest_main.a
$(LIBRARY_GTEST_MAIN) : $(BUILD_DIR)gtest-all.o $(BUILD_DIR)gtest_main.o
	$(AR) $@ $^
	$(RANLIB) $@

PF_OBJECTS     = $(addprefix $(BUILD_DIR), $(PF_SOURCES:.cc=.o))
RM_OBJECTS     = $(addprefix $(BUILD_DIR), $(RM_SOURCES:.cc=.o))
IX_OBJECTS     = $(addprefix $(BUILD_DIR), $(IX_SOURCES:.cc=.o))
SM_OBJECTS     = $(addprefix $(BUILD_DIR), $(SM_SOURCES:.cc=.o))
QL_OBJECTS     = $(addprefix $(BUILD_DIR), $(QL_SOURCES:.cc=.o))
UTILS_OBJECTS  = $(addprefix $(BUILD_DIR), $(UTILS_SOURCES:.cc=.o))
PARSER_OBJECTS = $(addprefix $(BUILD_DIR), $(PARSER_SOURCES:.c=.o))
TESTER_OBJECTS = $(addprefix $(BUILD_DIR), $(TESTER_SOURCES:.cc=.o))
OBJECTS        = $(PF_OBJECTS) $(RM_OBJECTS) $(IX_OBJECTS) \
                 $(SM_OBJECTS) $(QL_OBJECTS) $(PARSER_OBJECTS) \
                 $(TESTER_OBJECTS) $(UTILS_OBJECTS)

LIBRARY_PF     = $(LIB_DIR)libpf.a
LIBRARY_RM     = $(LIB_DIR)librm.a
LIBRARY_IX     = $(LIB_DIR)libix.a
LIBRARY_SM     = $(LIB_DIR)libsm.a
LIBRARY_QL     = $(LIB_DIR)libql.a
LIBRARY_PARSER = $(LIB_DIR)libparser.a
LIBRARIES      = $(LIBRARY_PF) $(LIBRARY_RM) $(LIBRARY_IX) \
                 $(LIBRARY_SM) $(LIBRARY_QL) $(LIBRARY_PARSER)

UTILS          = $(UTILS_SOURCES:.cc=)
TESTS          = $(TESTER_SOURCES:.cc=)
EXECUTABLES    = $(UTILS)

LIBS           = -lparser -lql -lsm -lix -lrm -lpf

#
# Build targets
#
all: $(LIBRARIES) $(UTILS)

clean:
	rm -f $(BUILD_DIR)*.o $(BUILD_DIR)*.d y.output y.tab.h parse.c $(LIBRARIES) \
	$(LIBRARY_GTEST_MAIN) $(LIBRARY_GTEST) $(EXECUTABLES) $(TESTS)

clean_test:
	rm -f $(TESTS)

testers: all $(TESTS)

#
# Libraries
#
$(LIBRARY_PF): $(PF_OBJECTS)
	$(AR) $(LIBRARY_PF) $(PF_OBJECTS)
	$(RANLIB) $(LIBRARY_PF)

$(LIBRARY_RM): $(RM_OBJECTS)
	$(AR) $(LIBRARY_RM) $(RM_OBJECTS)
	$(RANLIB) $(LIBRARY_RM)

$(LIBRARY_IX): $(IX_OBJECTS)
	$(AR) $(LIBRARY_IX) $(IX_OBJECTS)
	$(RANLIB) $(LIBRARY_IX)

$(LIBRARY_SM): $(SM_OBJECTS)
	$(AR) $(LIBRARY_SM) $(SM_OBJECTS)
	$(RANLIB) $(LIBRARY_SM)

$(LIBRARY_QL): $(QL_OBJECTS)
	$(AR) $(LIBRARY_QL) $(QL_OBJECTS)
	$(RANLIB) $(LIBRARY_QL)

$(LIBRARY_PARSER): $(PARSER_OBJECTS)
	$(AR) $(LIBRARY_PARSER) $(PARSER_OBJECTS)
	$(RANLIB) $(LIBRARY_PARSER)

#
# Parser
#
y.tab.h: parse.c

parse.c: parse.y
	$(YACC) parse.y; mv y.tab.c parse.c

scan.c: scan.l scanhelp.c y.tab.h
	$(LEX) scan.l; mv lex.yy.c scan.c

$(BUILD_DIR)parse.o: parse.c

$(BUILD_DIR)scan.o: scan.c y.tab.h

$(BUILD_DIR)nodes.o: nodes.c

$(BUILD_DIR)interp.o: interp.c

#
# Rules
#
-include $(OBJECTS:.o=.d)

$(BUILD_DIR)%.d: %.cc
	@set -e; \
	 rm -f $@; \
	 $(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@: ,g' $@.$$$$ > $@; \
	 rm -f $@.$$$$

$(OBJECTS): %.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(EXECUTABLES): %: $(BUILD_DIR)%.o $(LIBRARIES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -lpthread $< -o $@ -L$(LIB_DIR)

$(TESTS): %: $(BUILD_DIR)%.o $(LIBRARY_GTEST_MAIN) $(LIBRARIES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -lpthread $^ -o $@ -L$(LIB_DIR)
