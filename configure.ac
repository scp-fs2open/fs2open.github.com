dnl configure.ac for fs2_open
dnl
dnl $Id: configure.ac,v 1.9 2005-03-31 21:12:23 mrduckman Exp $

AC_INIT(fs2_open, 3.6.5)
AC_CONFIG_SRCDIR(code/freespace2/freespace.cpp)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([ no-define ])

AC_DEFINE([NO_CD_CHECK])

dnl Check for programs
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_CXX


dnl The configuration options
dnl -------------------------

AC_ARG_ENABLE(joystick, 
	AC_HELP_STRING([--disable-joystick],
		[TEST: disable joystick support]),
		[fs2_joystick=$enableval],
		[fs2_joystick=yes],
)

AC_ARG_ENABLE(networking, 
	AC_HELP_STRING([--disable-networking],
		[TEST: disable networking]),
		[fs2_networking=$enableval],
		[fs2_networking=yes],
)

AC_ARG_ENABLE(sound,
	AC_HELP_STRING([--disable-sound],
		[TEST: disable sound]),
		[fs2_sound=$enableval],
		[fs2_sound=yes],
)

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

AC_ARG_WITH(ogl-libs,
	AC_HELP_STRING([--with-ogl-libs=DIR],
		[Use OpenGL libraries from this directory]),
		[fs2_ogl_libs="$withval"],
		[fs2_ogl_libs=""],
)
AC_ARG_WITH(ogl-incs,
	AC_HELP_STRING([--with-ogl-incs=DIR],
		[Use OpenGL Includes from this directory]),
		[fs2_ogl_incs="$withval"],
		[fs2_ogl_incs=""],
)
AC_ARG_ENABLE(scplite,
	AC_HELP_STRING([--enable-scplite],
		[Make a SCP Lite build]),
		[fs2_lite=$enableval],
		[fs2_lite=no],
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
		[Link staticly with OGG Vorbis libs from this directory]),
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
		FS2_CXXFLAGS="$FS2_CXXFLAGS -m64"
        ;;
	*-*-linux*)
		# linux
		echo "Using Unix defines (for $host_os)"
		fs2_os_unix="yes"
		;;
	*-*-darwin*)
		# OSX
		echo "Using OSX defines (for $host_os)"
		fs2_os_osx="yes"
		FS2_LDFLAGS="$FS2_LDFLAGS -lobjc -fPIC -Wl,-noprebind"
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

if ! test "$fs2_joystick" = "yes" ; then
	AC_DEFINE([NO_JOYSTICK])
fi

if test "$fs2_inferno" = "yes" ; then
	AC_DEFINE([INF_BUILD])
	fs2_networking=no
fi

if ! test "$fs2_networking" = "yes" ; then
	AC_DEFINE([NO_NETWORK])
	AC_DEFINE([NO_STANDALONE])
fi

if ! test "$fs2_sound" = "yes" ; then
	AC_DEFINE([NO_SOUND])
fi

if test "$fs2_speech" = "yes" ; then
	AC_DEFINE([FS2_SPEECH])
fi

if test "$fs2_debug" = "yes" ; then
	AC_DEFINE([_DEBUG])
	FS2_CXXFLAGS="$FS2_CXXFLAGS -O0 -g -Wall"
	FS2_LDFLAGS="$FS2_LDFLAGS -g"

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
	AC_SUBST(FS2_BINARY, ["wxFRED2_d"])
	else
		AC_SUBST(FS2_BINARY, ["fs2_open_d"])
	fi
else
	AC_DEFINE([NDEBUG])
	FS2_CXXFLAGS="$FS2_CXXFLAGS -g -O2 -Wall"
	FS2_LDFLAGS="$FS2_LDFLAGS -g"

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
		AC_SUBST(FS2_BINARY, ["wxFRED2_r"])
	else
		AC_SUBST(FS2_BINARY, ["fs2_open_r"])
	fi
fi

dnl if test "$fs2_fred" = "yes" ; then
dnl 	AC_DEFINE([NDEBUG])
dnl 	FS2_CXXFLAGS="$FS2_CXXFLAGS -I/usr/lib64/wx/include/gtk-2.4 -DGTK_NO_CHECK_CASTS -D__WXGTK__ -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES -O2 -Wall"
dnl 	FS2_LDFLAGS="$FS2_LDFLAGS -L/usr/lib64 -pthread -lwx_gtk-2.4"
dnl 	AC_SUBST(FS2_BINARY, ["wxFRED2_r"])
dnl fi

if test "$fs2_lite" = "yes" ; then
	AC_DEFINE([GL_NO_HTL])
	AC_DEFINE([SCP_LITE])
fi

if test "$fs2_gprof" = "yes" ; then
	FS2_CXXFLAGS="$FS2_CXXFLAGS -pg"
fi


dnl generate appropriate conditionals for Makefile.am
AM_CONDITIONAL(FS2_OS_WIN32,   test "$fs2_os_win32" = "yes")
AM_CONDITIONAL(FS2_OS_UNIX,    test "$fs2_os_unix"  = "yes" || test "$fs2_os_osx" = "yes")
AM_CONDITIONAL(FS2_OS_OSX,     test "$fs2_os_osx"   = "yes")

AM_CONDITIONAL(FS2_JOYSTICK,   test "$fs2_joystick"   = "yes")
AM_CONDITIONAL(FS2_SOUND,      test "$fs2_sound"      = "yes")
AM_CONDITIONAL(FS2_DEVIL,      test "$fs2_devil"      = "yes")
AM_CONDITIONAL(FS2_SPEECH,     test "$fs2_speech"     = "yes")
AM_CONDITIONAL(FS2_DEBUG,      test "$fs2_debug"      = "yes")

AM_CONDITIONAL([am__fastdepOBJC],  test "$fs2_os_osx"  = "yes")

AC_CHECK_HEADER(malloc.h)
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


dnl Setup paths for lib and include dirs

dnl DirectX - Win32 only
if test "$fs2_os_win32" = "yes" ; then
	FS2_CXXFLAGS=$FS2_CXXFLAGS" -I/c:/dxsdk/include"
	FS2_LDFLAGS=$FS2_LDFLAGS" -L/c:/dxsdk/lib"
fi

dnl OpenGL libs and CFLAGS
if test "$fs2_os_win32" = "yes" ; then
	if test "x$fs2_ogl_libs" != "x" ; then
		FS2_LDFLAGS="$FS2_LDFLAGS -L$fs2_ogl_libs -lopengl32 -lglu32"
	else
		FS2_LDFLAGS="$FS2_LDFLAGS -lopengl32 -lglu32"
	fi
	AC_CHECK_LIB(opengl32, glBindBufferARB, ,
		[AC_DEFINE([GL_NO_HTL])
			echo ""
			echo "******************************************************"
			echo "* Warning: Your OpenGL libraries do not support      *"
			echo "* glBindBufferARB(). HT&L will be disabled. Rebuild  *"
			echo "* with newer OpenGL libraries to enable HT&L.        *"
			echo "******************************************************"
			echo ""])
elif test "$fs2_os_unix" = "yes" ; then
	if test "x$fs2_ogl_libs" != "x" ; then
		FS2_LDFLAGS="$FS2_LDFLAGS -L$fs2_ogl_libs -lGL -lGLU"
	else
		FS2_LDFLAGS="$FS2_LDFLAGS -lGL -lGLU"
	fi
	if test "x$fs2_ogl_incs" != "x" ; then
		FS2_CXXFLAGS="$FS2_CXXFLAGS -I$fs2_ogl_incs"
	fi
	AC_CHECK_LIB(GL, glBindBufferARB, ,
		[AC_DEFINE(GL_NO_HTL)
			echo ""
			echo "******************************************************"
			echo "* Warning: Your OpenGL libraries do not support      *"
			echo "* glBindBufferARB(). HT&L will be disabled. Rebuild  *"
			echo "* with newer OpenGL libraries to enable HT&L.        *"
			echo "******************************************************"
			echo ""])
	AC_DEFINE([SCP_UNIX])
	AC_DEFINE([NO_DIRECT3D])
elif test "$fs2_os_osx" = "yes" ; then
	FS2_LDFLAGS="$FS2_LDFLAGS -framework OpenGL"
	AC_CHECK_LIB(/System/Library/Frameworks/OpenGL.framework/OpenGL, glBindBufferARB, ,
		[AC_DEFINE([GL_NO_HTL])
			echo ""
			echo "******************************************************"
			echo "* Warning: Your OpenGL libraries do not support      *"
			echo "* glBindBufferARB(). HT&L will be disabled. Rebuild  *"
			echo "* with newer OpenGL libraries to enable HT&L.        *"
			echo "******************************************************"
			echo ""])
	AC_DEFINE([SCP_UNIX])
	AC_DEFINE([NO_DIRECT3D])
	## don't need the CFLAGS here if recent SDL is used
fi

dnl OpenAL
if test "$fs2_os_unix" = "yes" ; then
	AC_DEFINE([USE_OPENAL])
	FS2_LDFLAGS="$FS2_LDFLAGS -lopenal"
elif test "$fs2_os_osx" = "yes" ; then
	AC_DEFINE([USE_OPENAL])
	FS2_LDFLAGS="$FS2_LDFLAGS -framework OpenAL"
	AC_CHECK_HEADER(/Library/Frameworks/OpenAL.framework/Headers/al.h,
		FS2_CXXFLAGS="$FS2_CXXFLAGS -I/Library/Frameworks/OpenAL.framework/Headers",
		[AC_CHECK_HEADER($HOME/Library/Frameworks/OpenAL.framework/Headers/al.h,
			FS2_CXXFLAGS="$FS2_CXXFLAGS -I$HOME/Library/Frameworks/OpenAL.framework/Headers",
		[AC_CHECK_HEADER(/System/Library/Frameworks/OpenAL.framework/Headers/al.h,
			FS2_CXXFLAGS="$FS2_CXXFLAGS -I/System/Library/Frameworks/OpenAL.framework/Headers",
			AC_MSG_ERROR([*** Cannot find OpenAL which is required! ***]))])])
fi

dnl SDL check
if test "$fs2_os_unix" = "yes" ; then
	dnl Check for SDL 1.2.6+
	SDL_VERSION=1.2.6
	AM_PATH_SDL($SDL_VERSION,:,AC_MSG_ERROR([*** SDL >= $SDL_VERSION not found!]))
elif test "$fs2_os_osx" = "yes" ; then
	SDL_LIBS="-framework SDL"
	AC_CHECK_HEADER(/Library/Frameworks/SDL.framework/Headers/SDL.h,
		SDL_CFLAGS="-I/Library/Frameworks/SDL.framework/Headers",
		[AC_CHECK_HEADER($HOME/Library/Frameworks/SDL.framework/Headers/SDL.h,
			SDL_CFLAGS="-I$HOME/Library/Frameworks/SDL.framework/Headers",
		[AC_CHECK_HEADER(/System/Library/Frameworks/SDL.framework/Headers/SDL.h,
			SDL_CFLAGS="-I/System/Library/Frameworks/SDL.framework/Headers",
			AC_MSG_ERROR([*** Cannot find SDL which is required! ***]))])])
fi

dnl add SDL settings to our flags
FS2_CXXFLAGS=$FS2_CXXFLAGS" $SDL_CFLAGS"
FS2_LDFLAGS=$FS2_LDFLAGS" $SDL_LIBS"

dnl libjpeg
dnl if test "$fs2_os_unix" = "yes" ; then
dnl 	if test "x$fs2_static_jpeg" != "x" ; then
dnl 		FS2_LIBS="$FS2_LIBS $fs2_static_jpeg/libjpeg.a"
dnl 	else
dnl 		FS2_LDFLAGS="$FS2_LDFLAGS -ljpeg"
dnl 	fi
dnl fi

dnl OGG Vorbis
if test "$fs2_os_unix" = "yes" ; then
	if test "x$fs2_static_ogg" != "x" ; then
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libvorbisfile.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libvorbis.a"
		FS2_LIBS="$FS2_LIBS $fs2_static_ogg/libogg.a"
	else
		FS2_LDFLAGS="$FS2_LDFLAGS -logg -lvorbis -lvorbisfile"
	fi
fi


dnl any extra optimization or warning flags
if test "$fs2_os_win32" = "yes" ; then
	FS2_CXXFLAGS="$FS2_CXXFLAGS -fexceptions"
elif test "$fs2_os_unix" = "yes" ; then
	FS2_CXXFLAGS="$FS2_CXXFLAGS -fsigned-char -Wno-deprecated -Wno-unknown-pragmas"
elif test "$fs2_os_osx" = "yes" ; then
	FS2_CXXFLAGS="$FS2_CXXFLAGS -fsigned-char"
fi

dnl extra OSX frameworks
if test "$fs2_os_osx" = "yes" ; then
	FS2_LDFLAGS="$FS2_LDFLAGS -framework AppKit -framework Foundation"
fi

dnl For SDLMain.m on OSX
OBJC="$CC"
AC_SUBST(OBJC)
OBJCFLAGS="$SDL_CFLAGS"
AC_SUBST(OBJCFLAGS)


dnl prevent automake from overwriting our stuff or using extra
CXXFLAGS="$FS2_CXXFLAGS"
LDFLAGS="$FS2_LDFLAGS"
LIBS="$FS2_LIBS"


dnl The makefiles we create
AC_OUTPUT([Makefile libjpeg/Makefile code/Makefile])


echo
echo
echo Configure options:
echo
if test "x$fs2_ogl_libs" != "x" ; then
	echo Using OpenGL libs from . : $fs2_ogl_libs
fi
if test "x$fs2_ogl_incs" != "x" ; then
	echo Using OpenGL includes from . : $fs2_ogl_incs
fi
if test "x$fs2_static_jpeg" != "x" ; then
	echo Using libjpeg.a from ... : $fs2_static_jpeg
fi
if test "x$fs2_static_ogg" != "x" ; then
	echo Static OGG Vorbis from . : $fs2_static_ogg
fi
echo
echo Joystick enabled ....... : $fs2_joystick
echo Networking enabled ..... : $fs2_networking
echo Sound enabled .......... : $fs2_sound
echo Text-to-speech enabled . : $fs2_speech
echo
echo Debug build ............ : $fs2_debug
echo SCP Lite ............... : $fs2_lite
echo Inferno build .......... : $fs2_inferno
echo GProf support .......... : $fs2_gprof
echo
