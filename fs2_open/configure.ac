dnl configure.ac for fs2_open
dnl
dnl $Id: configure.ac,v 1.31.2.4 2007-05-28 19:36:12 taylor Exp $

AC_INIT(fs2_open, 3.6.13)
AC_CONFIG_SRCDIR(code/freespace2/freespace.cpp)

dnl we need at least version 2.54 of autoconf
AC_PREREQ(2.54)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl we need at least version 1.6.1 of automake, well it's as far down as has been tested anyway
AM_INIT_AUTOMAKE([ 1.6.1 no-define dist-bzip2 ])

AC_DEFINE([NO_CD_CHECK])

dnl Check for programs
dnl save current vars first
CXXFLAGS_save="$CXXFLAGS"
CFLAGS_save="$CFLAGS"
LDFLAGS_save="$LDFLAGS"

AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_CXX

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

AC_ARG_ENABLE(speech,
	AC_HELP_STRING([--enable-speech],
		[Enable text-to-speech]),
		[fs2_speech=$enableval],
		[fs2_speech=no],
)

AC_ARG_ENABLE(inferno,
	AC_HELP_STRING([--enable-inferno],
		[Make an Inferno build]),
		[fs2_inferno=$enableval],
		[fs2_inferno=no],
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
fs2_os_unix="no"
fs2_os_osx="no"
fs2_os_solaris="no"

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
        fs2_os_unix="yes"
		D_CFLAGS="$D_CFLAGS -m64 -march=athlon64 -ansi"
		D_CFLAGS="$D_CFLAGS -DLUA_USE_LINUX"
        ;;
	*-*-linux*)
		# linux
		echo "Using Unix defines (for $host_os)"
		fs2_os_unix="yes"
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
		fs2_os_solaris="yes"
		;;
	*)
		# Unknown
		echo "fs2_open does not support $host_cpu-$host_os"
		echo "check the configure.ac file"
		echo "host_cpu:    " $host_cpu
		echo "host_vendor: " $host_vendor
		echo "host_os      " $host_os
		exit 1;
		;;
esac


dnl define compiler switches

if test "$fs2_inferno" = "yes" ; then
	AC_DEFINE([INF_BUILD])
fi

if test "$fs2_speech" = "yes" ; then
	AC_DEFINE([FS2_SPEECH])
fi

if test "$fs2_fred" = "yes" ; then
	AM_OPTIONS_WXCONFIG
	AM_PATH_WXCONFIG(2.4.2, wxWin=1)

	if test "$wxWin" != 1; then
		AC_MSG_ERROR([
			wxWindows must be installed on your system
			but wx-config script couldn't be found.

			Please check that wx-config is in path, the directory
			where wxWindows libraries are installed (returned by
			'wx-config --libs' command) is in LD_LIBRARY_PATH or
			equivalent variable and wxWindows version is 2.3.4 or above.
		])
	fi

	FS2_CXXFLAGS="$FS2_CXXFLAGS $WX_CXXFLAGS_ONLY"
	FS2_LDFLAGS="$FS2_LDFLAGS $WX_LIBS"
fi

if test "$fs2_debug" = "yes" ; then
	AC_DEFINE([_DEBUG])
	D_CFLAGS="$D_CFLAGS -O0 -g -Wall -Wextra -Wno-unused-parameter -Wno-write-strings -Wshadow -funroll-loops"
	D_LDFLAGS="$D_LDFLAGS -g"

	if test "$fs2_fred" = "yes" ; then
		if test "$fs2_inferno" = "yes" ; then
			AC_SUBST(FS2_BINARY, ["wxFRED2_INF_d"])
		else
			AC_SUBST(FS2_BINARY, ["wxFRED2_d"])
		fi
	else
		if test "$fs2_inferno" = "yes" ; then
			AC_SUBST(FS2_BINARY, ["fs2_open_INF_d"])
		else
			AC_SUBST(FS2_BINARY, ["fs2_open_d"])
		fi
	fi
else
	AC_DEFINE([NDEBUG])
	D_CFLAGS="$D_CFLAGS -O2 -Wall -funroll-loops"
	D_LDFLAGS="$D_LDFLAGS "

	if test "$fs2_fred" = "yes" ; then
		if test "$fs2_inferno" = "yes" ; then
			AC_SUBST(FS2_BINARY, ["wxFRED2_INF_r"])
		else
			AC_SUBST(FS2_BINARY, ["wxFRED2_r"])
		fi
	else
		if test "$fs2_inferno" = "yes" ; then
			AC_SUBST(FS2_BINARY, ["fs2_open_INF_r"])
		else
			AC_SUBST(FS2_BINARY, ["fs2_open_r"])
		fi
	fi
fi

if test "$fs2_lite" = "yes" ; then
	AC_DEFINE([SCP_LITE])
fi

if test "$fs2_gprof" = "yes" ; then
	FS2_CXXFLAGS="$FS2_CXXFLAGS -pg"
fi


dnl generate appropriate conditionals for Makefile.am
AM_CONDITIONAL(FS2_OS_WIN32,   test "$fs2_os_win32" = "yes")
AM_CONDITIONAL(FS2_OS_UNIX,    test "$fs2_os_unix"  = "yes" || test "$fs2_os_osx" = "yes" || test "$fs2_os_solaris" = "yes")
AM_CONDITIONAL(FS2_OS_OSX,     test "$fs2_os_osx"   = "yes")
AM_CONDITIONAL(FS2_OS_SOLARIS, test "$fs2_os_solaris"   = "yes")

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
if test "$fs2_os_unix" = "yes" ; then
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
	AC_DEFINE([SCP_UNIX])
	AC_DEFINE([SCP_SOLARIS])
	AC_DEFINE([NO_DIRECT3D])
	## don't need the CFLAGS here if recent SDL is used
fi

dnl add SDL settings to our flags, only CFLAGS are default, libs are game specific
D_CFLAGS=$D_CFLAGS" $SDL_CFLAGS"
FS2_LIBS=$FS2_LIBS" $SDL_LIBS"

dnl OGG/Vorbis/Theora check
if test "$fs2_os_unix" = "yes" ; then
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
if test "$fs2_os_unix" = "yes" ; then
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

dnl libpng
FS2_LIBS="$FS2_LIBS -lpng"

dnl OpenGL libs and CFLAGS
if test "$fs2_os_win32" = "yes" ; then
	if test "x$GL_LIBS" != "x" ; then
		FS2_LDFLAGS="$FS2_LDFLAGS -L$GL_LIBS -lopengl32 -lglu32"
	else
		FS2_LDFLAGS="$FS2_LDFLAGS -lopengl32 -lglu32"
	fi
elif test "$fs2_os_unix" = "yes" ; then
	AM_PATH_OPENGL(:,AC_MSG_ERROR([*** OpenGL installation not found!]))
	D_CFLAGS=$D_CFLAGS" $OPENGL_CFLAGS"
	FS2_LIBS=$FS2_LIBS" $OPENGL_LIBS"
	AC_DEFINE([SCP_UNIX])
	AC_DEFINE([NO_DIRECT3D])
elif test "$fs2_os_osx" = "yes" ; then
	FS2_CXXFLAGS="$FS2_CXXFLAGS -I/System/Library/Frameworks/OpenGL.framework/Headers"
	FS2_LDFLAGS="$FS2_LDFLAGS -framework OpenGL"
	AC_DEFINE([SCP_UNIX])
	AC_DEFINE([NO_DIRECT3D])
	## don't need the CFLAGS here if recent SDL is used
fi

dnl PNG libs
PKG_CHECK_MODULES([PNG], [libpng >= 1.2.20])
D_CFLAGS=$D_CFLAGS" $PNG_CFLAGS"
FS2_LIBS=$FS2_LIBS" $PNG_LIBS"

dnl LUA
## Checking for lua.pc and if that fails lua5.1.pc
PKG_CHECK_MODULES(
	[LUA],
	[lua >= 5.1.3],
	[], ## do nothing special
	[PKG_CHECK_MODULES([LUA], [lua5.1 >= 5.1.3], [])])
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
if test "$fs2_os_unix" = "yes" ; then
	if test "x$fs2_static_ogg" != "x" ; then
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libtheora.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libvorbisfile.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libvorbis.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libogg.a"
	fi
elif test "$fs2_os_osx" = "yes" ; then
	if test "x$fs2_static_ogg" != "x" ; then
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libtheora.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libvorbisfile.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libvorbis.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libogg.a"
	fi
fi


dnl any extra optimization or warning flags
if test "$fs2_os_win32" = "yes" ; then
	D_CFLAGS="$D_CFLAGS -fexceptions"
elif test "$fs2_os_unix" = "yes" ; then
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
AC_OUTPUT([Makefile code/Makefile])


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
echo
echo Debug build ............ : $fs2_debug
echo Inferno build .......... : $fs2_inferno
echo GProf support .......... : $fs2_gprof
echo
