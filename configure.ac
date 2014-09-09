dnl configure.ac for fs2_open
dnl
dnl $Id: configure.ac,v 1.31.2.4 2007-05-28 19:36:12 taylor Exp $

AC_INIT(fs2_open, 3.7.1)
AC_CONFIG_SRCDIR(code/freespace2/freespace.cpp)

dnl we need at least version 2.54 of autoconf
AC_PREREQ(2.54)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl we need at least version 1.6.1 of automake, well it's as far down as has been tested anyway
AM_INIT_AUTOMAKE([ 1.6.1 no-define dist-bzip2 subdir-objects ])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_DEFINE([NO_CD_CHECK])

AC_CONFIG_MACRO_DIR([m4])

dnl Check for programs
dnl save current vars first
CXXFLAGS_save="$CXXFLAGS"
CFLAGS_save="$CFLAGS"
LDFLAGS_save="$LDFLAGS"

AC_PROG_CC
AC_PROG_CXX
LT_INIT

dnl ok we want to replace any defaults with our own since it's probably less freaky that way
CXXFLAGS="$CXXFLAGS_save"
CFLAGS="$CFLAGS_save"
LDFLAGS="$LDFLAGS_save"
AC_SUBST(CXXFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)

PKG_PROG_PKG_CONFIG()


dnl The configuration options
dnl -------------------------

AC_ARG_ENABLE(debug,
	AC_HELP_STRING([--enable-debug],
		[Build in DEBUG mode (default = no)]),
		[fs2_debug=$enableval],
		[fs2_debug=no],
)

AC_ARG_ENABLE(wxfred2,
	AC_HELP_STRING([--enable-wxfred2],
		[Build wxFRED2 rather than the normal game (default = no)]),
		[fs2_fred=$enableval],
		[fs2_fred=no],
)
AM_CONDITIONAL([BUILD_WXFRED], [test x$fs2_fred = xyes])

AC_ARG_ENABLE(speech,
	AC_HELP_STRING([--enable-speech],
		[Enable text-to-speech]),
		[fs2_speech=$enableval],
		[fs2_speech=no],
)

AC_ARG_ENABLE(nowarn,
	AC_HELP_STRING([--enable-nowarn],
		[Make a Warning Suppression Build (NOT Recommended)]),
		[fs2_nowarn=$enableval],
		[fs2_nowarn=no],
)

AC_ARG_ENABLE(gprof,
	AC_HELP_STRING([--enable-gprof],
		[Build with GProf support]),
		[fs2_gprof=$enableval],
		[fs2_gprof=no],
)

AC_ARG_WITH(static-ogg,
	AC_HELP_STRING([--with-static-ogg=DIR],
		[Link staticly with OGG, Vorbis and Theora libs from this directory]),
		[fs2_static_ogg="$withval"],
		[fs2_static_ogg=""],
)

AC_ARG_WITH(static-jpeg,
	AC_HELP_STRING([--with-static-jpeg=DIR],
		[Link staticly with JPEG libs from this directory]),
		[fs2_static_jpeg="$withval"],
		[fs2_static_jpeg=""],
)


dnl check OS type -- right now, we support
dnl windows OS (via mingw) and unix (linux, etc.)
dnl
dnl the following block will need to be updated as additional
dnl environments are supported
fs2_os_win32="no"
fs2_os_linux="no"
fs2_os_osx="no"
fs2_os_solaris="no"
fs2_os_bsd="no"

dnl case $host_os in
case "$target" in
	*-*-mingw*)
		# windows
		echo "using Win32 defines (for $host_os)"
		fs2_os_win32="yes"
		AC_DEFINE([_MINGW])
		FS2_LDFLAGS="$FS2_LDFLAGS -lgdi32 -lole32 -lwsock32 -lwinmm -lmsacm32 \
			-lcomctl32 -lstrmiids -ldxguid -lddraw -ldinput"
		;;
	x86_64-*-linux*)
		# linux
		echo "Using 64-bit Unix defines (for $host_os)"
		fs2_os_linux="yes"
		D_CFLAGS="$D_CFLAGS -m64 -mtune=generic -msse -msse2 -ansi -pipe"
		D_CFLAGS="$D_CFLAGS -DLUA_USE_LINUX"
		;;
	*-*-linux*)
		# linux
		echo "Using Unix defines (for $host_os)"
		fs2_os_linux="yes"
		D_CFLAGS="$D_CFLAGS -mtune=generic -mfpmath=sse -msse -msse2 -ansi -pipe"
		D_CFLAGS="$D_CFLAGS -DLUA_USE_LINUX"
		;;
	*-*-darwin*)
		# OSX
		echo "Using OSX defines (for $host_os)"
		fs2_os_osx="yes"
		D_LDFLAGS="$D_LDFLAGS -lobjc -fPIC -Wl,-noprebind"
		D_CFLAGS="$D_CFLAGS -DLUA_USE_MACOSX"
		;;
	*-*-solaris*)
		# Solaris
		echo "Using Solaris 10 defines (for $host_os)"
		echo "************************************************************"
		echo "************************ NOTE ******************************"
		echo "* Support for this OS is not actively maintatined. It ******"
		echo "* should work but please provide feedback about builds and *"
		echo "* play experience on this OS at ****************************"
		echo "* http://www.hard-light.net/forums/index.php?board=113.0 ***"
		echo "************************************************************"
		fs2_os_solaris="yes"
		LDFLAGS="$LDFLAGS -L/opt/csw/lib"
		CPPFLAGS="$CPPFLAGS -I/opt/csw/include"
		;;
	*freebsd*)
		# FreeBSD 9.1
		echo "Using Unix defines (for $host_os $host_cpu)"
		echo "************************************************************"
		echo "************************ NOTE ******************************"
		echo "* Support for this OS is not actively maintatined. It ******"
		echo "* should work but please provide feedback about builds and *"
		echo "* play experience on this OS at ****************************"
		echo "* http://www.hard-light.net/forums/index.php?board=113.0 ***"
		echo "************************************************************"
		fs2_os_bsd="yes"
		if test "$host_cpu" = "amd64"
		then
			D_CFLAGS="$D_CFLAGS -m64 -march=athlon64 -ansi"
		fi
		D_CFLAGS="$D_CFLAGS -DLUA_USE_LINUX"
		LDFLAGS="$LDFLAGS -L/usr/local/lib"
		CPPFLAGS="$CPPFLAGS -I/usr/local/include"
		;;
	*)
		# Unknown
		echo "fs2_open does not support $host_cpu-$host_os"
		echo "check the configure.ac file"
		echo "host_cpu:    " $host_cpu
		echo "host_vendor: " $host_vendor
		echo "host_os      " $host_os
		echo "************************************************************"
		echo "* Please let us know about successful builds on this OS at *"
		echo "* http://www.hard-light.net/forums/index.php?board=113.0 ***"
		echo "* Create a new thread about your build experience and what *"
		echo "* was required to make the build successful. ***************"
		echo "************************************************************"
		exit 1;
		;;
esac


dnl define compiler switches

if test "$fs2_nowarn" = "yes" ; then
	AC_DEFINE([Allow_NoWarn])
fi

if test "$fs2_speech" = "yes" ; then
	AC_DEFINE([FS2_SPEECH])
fi

if test "$fs2_fred" = "yes" ; then
	WX_CONFIG_OPTIONS
	reqwx=2.9.4
	WX_CONFIG_CHECK([$reqwx], [wxWin=1], [wxWin=0], [all])

	if test "$wxWin" != 1; then
		AC_MSG_ERROR([
			wxWindows must be installed on your system
			but wx-config script couldn't be found.

			Please check that wx-config is in path, the directory
			where wxWindows libraries are installed (returned by
			'wx-config --libs' command) is in LD_LIBRARY_PATH or
			equivalent variable and wxWindows version is $reqwx or above.
		])
	fi

	D_CFLAGS="$D_CFLAGS $WX_CFLAGS -Iwxfred2"
	FS2_LIBS="$FS2_LIBS $WX_LIBS"
fi

if test "$fs2_debug" = "yes" ; then
	AC_DEFINE([_DEBUG])
	D_CFLAGS="$D_CFLAGS -O0 -g -Wall -Wextra -Wno-unused-parameter -Wno-write-strings -Wshadow -funroll-loops"
	D_LDFLAGS="$D_LDFLAGS -g"

	if test "$fs2_fred" = "yes" ; then
		AC_SUBST(FS2_BINARY, ["wxFRED2_${PACKAGE_VERSION}_DEBUG"])
	else
		AC_SUBST(FS2_BINARY, ["fs2_open_${PACKAGE_VERSION}_DEBUG"])
	fi
else
	AC_DEFINE([NDEBUG])
	D_CFLAGS="$D_CFLAGS -O2 -Wall -Wno-write-strings -funroll-loops"
	D_LDFLAGS="$D_LDFLAGS "

	if test "$fs2_fred" = "yes" ; then
		AC_SUBST(FS2_BINARY, ["wxFRED2_${PACKAGE_VERSION}"])
	else
		AC_SUBST(FS2_BINARY, ["fs2_open_${PACKAGE_VERSION}"])
	fi
fi

if test "$fs2_gprof" = "yes" ; then
	FS2_CXXFLAGS="$FS2_CXXFLAGS -pg"
fi


dnl generate appropriate conditionals for Makefile.am
AM_CONDITIONAL(FS2_OS_WIN32,   test "$fs2_os_win32" = "yes")
AM_CONDITIONAL(FS2_OS_UNIX,    test "$fs2_os_linux" = "yes" || test "$fs2_os_osx" = "yes" || test "$fs2_os_solaris" = "yes" || test "$fs2_os_bsd" = "yes")
AM_CONDITIONAL(FS2_OS_LINUX,   test "$fs2_os_linux" = "yes")
AM_CONDITIONAL(FS2_OS_OSX,     test "$fs2_os_osx"   = "yes")
AM_CONDITIONAL(FS2_OS_SOLARIS, test "$fs2_os_solaris"   = "yes")
AM_CONDITIONAL(FS2_OS_BSD,     test "$fs2_os_bsd"   = "yes")

AM_CONDITIONAL(FS2_DEVIL,      test "$fs2_devil"      = "yes")
AM_CONDITIONAL(FS2_SPEECH,     test "$fs2_speech"     = "yes")
AM_CONDITIONAL(FS2_DEBUG,      test "$fs2_debug"      = "yes")

AM_CONDITIONAL([am__fastdepOBJC],  test "$fs2_os_osx"  = "yes")

AC_CHECK_HEADER(stdlib.h)

dnl From licq: Copyright (c) 2000 Dirk Mueller
dnl Check if the type socklen_t is defined anywhere
AC_DEFUN([AC_C_SOCKLEN_T],
[AC_CACHE_CHECK(for socklen_t, ac_cv_c_socklen_t,
[
	AC_TRY_COMPILE([
		#include <sys/types.h>
		#include <sys/socket.h>
	],[
		socklen_t foo;
	],[
		ac_cv_c_socklen_t=yes
	],[
		ac_cv_c_socklen_t=no
	])
])
if test $ac_cv_c_socklen_t = no; then
	AC_DEFINE(socklen_t, int, [define to int if socklen_t not available])
fi
])

AC_C_SOCKLEN_T

dnl Check if std::ios_base is available. Old compilers (e.g. gcc 2.9x)
dnl may have to use std::ios instead
AC_DEFUN([AC_CXX_IOS_BASE],
[AC_CACHE_CHECK(for std::ios_base, ac_cv_cxx_class_ios_base,
[
	AC_LANG_PUSH([C++])
	AC_TRY_COMPILE([
		#include <iostream>
	],[
		std::ios_base::out;
	],[
		ac_cv_cxx_class_ios_base=yes
	],[
		ac_cv_cxx_class_ios_base=no
	])
	AC_LANG_POP([C++])
])
if test "x$ac_cv_cxx_class_ios_base" = "xno"; then
	AC_DEFINE(ios_base, ios, [define std::ios_base as std::ios if not available])
fi
])

AC_CXX_IOS_BASE


dnl Setup paths for lib and include dirs

dnl SDL check
if test "$fs2_os_linux" = "yes" || test "$fs2_os_bsd" = "yes" ; then
	dnl Check for SDL 1.2.6+
	SDL_VERSION=1.2.6
	AM_PATH_SDL($SDL_VERSION,:,AC_MSG_ERROR([*** SDL >= $SDL_VERSION not found!]))
elif test "$fs2_os_osx" = "yes" ; then
	SDL_LIBS="-framework SDL"
	dnl Check for SDL 1.2.6+
	SDL_VERSION=1.2.6
	AM_PATH_SDL($SDL_VERSION,:,[AC_CHECK_HEADER(/Library/Frameworks/SDL.framework/Headers/SDL.h,
		SDL_CFLAGS="-I/Library/Frameworks/SDL.framework/Headers",
		[AC_CHECK_HEADER($HOME/Library/Frameworks/SDL.framework/Headers/SDL.h,
			SDL_CFLAGS="-I$HOME/Library/Frameworks/SDL.framework/Headers",
		[AC_CHECK_HEADER(/System/Library/Frameworks/SDL.framework/Headers/SDL.h,
			SDL_CFLAGS="-I/System/Library/Frameworks/SDL.framework/Headers",
			AC_MSG_ERROR([*** Cannot find SDL which is required! ***]))])])])
elif test "$fs2_os_solaris" = "yes" ; then
	FS2_CXXFLAGS="$FS2_CXXFLAGS -I/usr/include/SDL"
	FS2_LDFLAGS="$FS2_LDFLAGS -lpng -lpng12 -ljpeg -lpthread -logg -lSDL -ltheora -lopenal -lsocket -lvorbisfile -lvorbis -lnsl -lGL -lGLU"
	## don't need the CFLAGS here if recent SDL is used
fi

dnl add SDL settings to our flags, only CFLAGS are default, libs are game specific
D_CFLAGS=$D_CFLAGS" $SDL_CFLAGS"
FS2_LIBS=$FS2_LIBS" $SDL_LIBS"

dnl OGG/Vorbis/Theora check
if test "$fs2_os_linux" = "yes" || test "$fs2_os_bsd" = "yes" ; then
	dnl Check that we have libogg, libvorbis, libvorbisfile, & libtheora
	XIPH_PATH_OGG(:,AC_MSG_ERROR([*** OGG installation not found!]))
	XIPH_PATH_VORBIS(:,AC_MSG_ERROR([*** Vorbis installation not found!]))
	XIPH_PATH_THEORA(:,AC_MSG_ERROR([*** Theora installation not found!]))
fi

dnl add OGG settings to our flags, only CFLAGS are default, libs are game specific
D_CFLAGS=$D_CFLAGS" $VORBIS_CFLAGS $VORBISFILE_CFLAGS"
FS2_LIBS=$FS2_LIBS" $VORBIS_LIBS $VORBISFILE_LIBS"
D_CFLAGS=$D_CFLAGS" $THEORA_CFLAGS"
FS2_LIBS=$FS2_LIBS" $THEORA_LIBS"
D_CFLAGS=$D_CFLAGS" $OGG_CFLAGS"
FS2_LIBS=$FS2_LIBS" $OGG_LIBS"

dnl OpenAL
if test "$fs2_os_linux" = "yes" || test "$fs2_os_bsd" = "yes" ; then
	AM_PATH_OPENAL(:,AC_MSG_ERROR([*** OpenAL installation not found!]))
	D_CFLAGS=$D_CFLAGS" $OPENAL_CFLAGS"
	FS2_LIBS=$FS2_LIBS" $OPENAL_LIBS"
elif test "$fs2_os_osx" = "yes" ; then
	FS2_LDFLAGS="$FS2_LDFLAGS -framework OpenAL"
	AC_CHECK_HEADER(/Library/Frameworks/OpenAL.framework/Headers/al.h,
		FS2_CXXFLAGS="$FS2_CXXFLAGS -I/Library/Frameworks/OpenAL.framework/Headers",
		[AC_CHECK_HEADER($HOME/Library/Frameworks/OpenAL.framework/Headers/al.h,
			FS2_CXXFLAGS="$FS2_CXXFLAGS -I$HOME/Library/Frameworks/OpenAL.framework/Headers",
		[AC_CHECK_HEADER(/System/Library/Frameworks/OpenAL.framework/Headers/al.h,
			FS2_CXXFLAGS="$FS2_CXXFLAGS -I/System/Library/Frameworks/OpenAL.framework/Headers",
			AC_MSG_ERROR([*** Cannot find OpenAL which is required! ***]))])])
fi

dnl DirectX - Win32 only
if test "$fs2_os_win32" = "yes" ; then
	FS2_CXXFLAGS=$FS2_CXXFLAGS" -I/c:/dxsdk/include"
	FS2_LDFLAGS=$FS2_LDFLAGS" -L/c:/dxsdk/lib"
fi

dnl OpenGL libs and CFLAGS
if test "$fs2_os_win32" = "yes" ; then
	if test "x$GL_LIBS" != "x" ; then
		FS2_LDFLAGS="$FS2_LDFLAGS -L$GL_LIBS -lopengl32 -lglu32"
	else
		FS2_LDFLAGS="$FS2_LDFLAGS -lopengl32 -lglu32"
	fi
elif test "$fs2_os_linux" = "yes" || test "$fs2_os_bsd" = "yes" ; then
	AM_PATH_OPENGL(:,AC_MSG_ERROR([*** OpenGL installation not found!]))
	D_CFLAGS=$D_CFLAGS" $OPENGL_CFLAGS"
	FS2_LIBS=$FS2_LIBS" $OPENGL_LIBS"
	AC_DEFINE([SCP_UNIX])
	AC_DEFINE([NO_DIRECT3D])
	if test "$fs2_os_bsd" = "yes" ; then
		AC_DEFINE([SCP_BSD])
	fi
elif test "$fs2_os_osx" = "yes" ; then
	FS2_CXXFLAGS="$FS2_CXXFLAGS -I/System/Library/Frameworks/OpenGL.framework/Headers"
	FS2_LDFLAGS="$FS2_LDFLAGS -framework OpenGL"
	AC_DEFINE([SCP_UNIX])
	AC_DEFINE([NO_DIRECT3D])
	## don't need the CFLAGS here if recent SDL is used
elif test "$fs2_os_solaris" = "yes" ; then
	AC_DEFINE([SCP_UNIX])
	AC_DEFINE([SCP_SOLARIS])
	AC_DEFINE([NO_DIRECT3D])
fi

dnl JANSSON
PKG_CHECK_MODULES([JANSSON], [jansson >= 2.2])
D_CFLAGS=$D_CFLAGS" $JANSSON_CFLAGS"
FS2_LIBS=$FS2_LIBS" $JANSSON_LIBS"

dnl PNG libs (libpng, falling back to libpng12)
PKG_CHECK_MODULES(
	[PNG],
	[libpng >= 1.2.20],
	[], ## do nothing special
	[PKG_CHECK_MODULES([PNG], [libpng12 >= 1.2.20])]
	)
D_CFLAGS=$D_CFLAGS" $PNG_CFLAGS"
FS2_LIBS=$FS2_LIBS" $PNG_LIBS"

dnl LUA
## Checking for lua.pc and if that fails lua5.1.pc
PKG_CHECK_MODULES(
	[LUA],
	[lua >= 5.1.3 lua < 5.2],
	[], ## do nothing special
	[PKG_CHECK_MODULES([LUA], [lua5.1 >= 5.1.3 lua5.1 < 5.2], [], [
        PKG_CHECK_MODULES([LUA], [lua-5.1 >= 5.1.3 lua-5.1 < 5.2], [])
    ])])
D_CFLAGS=$D_CFLAGS" $LUA_CFLAGS"
FS2_LIBS=$FS2_LIBS" $LUA_LIBS"

dnl JPEG
AC_CHECK_LIB(jpeg, jpeg_start_decompress,
	[AC_CHECK_HEADER(jpeglib.h, ,
		dnl NOTE: this is failure
		AC_MSG_ERROR([*** JPEG installation missing or broken!])
	)],
	AC_MSG_ERROR([*** JPEG installation missing or broken!])
)
FS2_LIBS=$FS2_LIBS" -ljpeg"


dnl OGG Vorbis/Theora (static libs)
if test "$fs2_os_linux" = "yes" || test "$fs2_os_osx" = "yes" || test "$fs2_os_bsd" = "yes" ; then
	if test "x$fs2_static_ogg" != "x" ; then
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libtheora.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libvorbisfile.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libvorbis.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libogg.a"
	fi
fi

if test "$fs2_os_bsd" != "yes" ; then
	dnl Dynamic Linking
	AC_SEARCH_LIBS([dlsym], [dl])
	FS2_LIBS=$FS2_LIBS" -ldl"
fi

dnl Posix Threads
AC_SEARCH_LIBS([pthread_mutex_timedlock], [pthread])
FS2_LIBS=$FS2_LIBS" -lpthread"

dnl any extra optimization or warning flags
if test "$fs2_os_win32" = "yes" ; then
	D_CFLAGS="$D_CFLAGS -fexceptions"
elif test "$fs2_os_linux" = "yes" || test "$fs2_os_bsd" = "yes" ; then
	D_CFLAGS="$D_CFLAGS -fsigned-char -Wno-unknown-pragmas"
	FS2_CXXFLAGS="$FS2_CXXFLAGS -Wno-deprecated -Wno-char-subscripts"
elif test "$fs2_os_osx" = "yes" ; then
	D_CFLAGS="$D_CFLAGS -fsigned-char -Wno-unknown-pragmas"
fi

dnl extra OSX frameworks
if test "$fs2_os_osx" = "yes" ; then
	FS2_LDFLAGS="$FS2_LDFLAGS -framework AppKit -framework Foundation"
fi


dnl substitute our default flags
AC_SUBST(D_CFLAGS)
AC_SUBST(D_LDFLAGS)

dnl substitute out game specific flags
AC_SUBST(FS2_CXXFLAGS)
AC_SUBST(FS2_LDFLAGS)
LIBS="$FS2_LIBS"


dnl The makefiles we create
AC_OUTPUT([Makefile mongoose/Makefile code/Makefile])


echo
echo
echo Configure options:
echo
if test "x$GL_LIBS" != "x" ; then
	echo Using OpenGL libs from . : $GL_LIBS
fi
if test "x$fs2_static_jpeg" != "x" ; then
	echo Using libjpeg.a from ... : $fs2_static_jpeg
fi
if test "x$fs2_static_ogg" != "x" ; then
	echo Static OGG libs from ... : $fs2_static_ogg
fi
echo
echo Text-to-speech enabled . : $fs2_speech
echo NoWarn Allowed ......... : $fs2_nowarn
echo Debug build ............ : $fs2_debug
echo GProf support .......... : $fs2_gprof
echo
