# Makefile.am for fs2_open (top level)
#
# This file uses GNU Automake to generate makefiles

SUBDIRS = code


EXTRA_DIST =	\
	FS2OpenSCPReadMe.doc	\
	projects/MSVC_2005/Freespace2.sln	\
	projects/MSVC_2005/Freespace2.vcproj	\
	projects/MSVC_2005/code.vcproj	\
	projects/MSVC_6/Fred2.dsp	\
	projects/MSVC_6/Freespace2.dsp	\
	projects/MSVC_6/Freespace2.dsw	\
	projects/MSVC_6/code.dsp	\
	projects/MSVC_6/libjpeg.dsp	\
	projects/MSVC_6/liblua.dsp	\
	projects/MSVC_6/wxFRED2.dsp	\
	projects/Xcode/English.lproj/InfoPlist.strings	\
	projects/Xcode/FS2_Open.icns	\
	projects/Xcode/FS2_Open.xcodeproj/project.pbxproj	\
	projects/Xcode/Frameworks.tgz	\
	projects/Xcode/Info-debug.plist	\
	projects/Xcode/Info.plist	\
	projects/Xcode/README.Xcode	\
	projects/Xcode/SDLMain.h	\
	projects/Xcode/SDLMain.m


# D_ variants are defaults and should always be used
AM_CXXFLAGS = -I./code @D_CFLAGS@
AM_LDFLAGS = @D_LDFLAGS@

EXTRA_PROGRAMS = cfilearchiver cfileextractor

###############################################################################
##
## Begin utilitites
##

# cfilearchiver -- to create VPs
cfilearchiver_SOURCES =	\
	code/cfilearchiver/cfilearchiver.cpp

# cfileextractor -- to extract files from VPs
cfileextractor_SOURCES = \
	code/cfileextractor/cfileextractor.cpp

# catch all to build utilitites
tools: cfilearchiver cfileextractor

# cleaning
clean-tools:
	-test -z "$(EXTRA_PROGRAMS)" || rm -f $(EXTRA_PROGRAMS)

##
## End utilitites
##
###############################################################################


# make sure we get this stuff cleaned up
clean-generic: clean-tools

