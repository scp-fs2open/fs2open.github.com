# Makefile.am for fs2_open (top level)
#
# This file uses GNU Automake to generate makefiles

SUBDIRS = libjpeg code



cfilearchiver:
	$(CXX) $(AM_CXXFLAGS) -I./code -o cfilearchiver code/cfilearchiver/cfilearchiver.cpp

cryptstring:
	$(CXX) $(AM_CXXFLAGS) -I./code -o cryptstring code/cryptstring/cryptstring.cpp

scramble:
	$(CXX) $(AM_CXXFLAGS) -I./code -c -o code/encrypt.o code/parse/encrypt.cpp
	$(CXX) $(AM_CXXFLAGS) -I./code -o scramble	code/scramble/scramble.cpp code/encrypt.o
