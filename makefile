#
# name: Kaylie Pham
# RedID: 828129478
#
# name: Aditya Bhagat
# RedID: 828612974

# CXX Make variable for compiler
CXX=g++
# -std=c++11  C/C++ variant to use, e.g. C++ 2011
# -Wall       show the necessary warning files
# -g3         include information for symbolic debugger e.g. gdb
CXXFLAGS=-std=c++11 -Wall -g3 -c

# object files
OBJS = log_helpers.o page_table.o wsclock.o vaddr_tracereader.o pagingwithwsclock.o


# Program name
PROGRAM = pagingwithwsclock

# Rules format:
# target : dependency1 dependency2 ... dependencyN
#     Command to make target, uses default rules if not specified

# First target is the one executed if you just type make
# make target specifies a specific target
# $^ is an example of a special variable.  It substitutes all dependencies

$(PROGRAM) : $(OBJS)
	$(CXX) -o $(PROGRAM) $^ -lpthread

log_helpers.o : log_helpers.h log_helpers.cpp
	$(CXX) $(CXXFLAGS) log_helpers.cpp

page_table.o : page_table.h page_table.cpp
	$(CXX) $(CXXFLAGS) page_table.cpp

wsclock.o : wsclock.h wsclock.cpp
	$(CXX) $(CXXFLAGS) wsclock.cpp

vaddr_tracereader.o : vaddr_tracereader.h vaddr_tracereader.cpp
	$(CXX) $(CXXFLAGS) vaddr_tracereader.cpp

pagingwithwsclock.o : pagingwithwsclock.cpp
	$(CXX) $(CXXFLAGS) pagingwithwsclock.cpp

clean :
	rm -f *.o $(PROGRAM)

