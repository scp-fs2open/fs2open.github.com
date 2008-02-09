dnl configure.ac for fs2_open
dnl
dnl $Id: configure.ac,v 1.1 2004-05-24 04:10:47 taylor Exp $

AC_INIT(fs2_open, 3.6)
AC_CONFIG_SRCDIR(code/freespace2/freespace.cpp)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([ no-define ])

AC_DEFINE([NO_CD_CHECK])

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
                       [TEST: disable networking (multi-player)]),
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
					[build in DEBUG mode (default is release mode)]),
		[fs2_debug=$enableval],
		[fs2_debug=no],
)

AC_ARG_ENABLE(devil,
        AC_HELP_STRING([--enable-devil],
						[use DevIL for image loading (needed for jpg,tga files)]),
        [fs2_devil=$enableval],
        [fs2_devil=no],
)

AC_ARG_ENABLE(speech,
		AC_HELP_STRING([--enable-speech],
						[enable use text-to-speech]),
		[fs2_speech=$enableval],
		[fs2_speech=no],
)

AC_ARG_WITH(ogl-libs,
		AC_HELP_STRING([--with-ogl-libs=PFX],
						[use OpenGL libraries in this prefix]),
		[fs2_ogl_libs="$withval"],
		[fs2_ogl_libs=""],
)



dnl check OS type -- right now, we support
dnl windows OS (via mingw) and unix (linux, etc.)
dnl
dnl the following block will need to be updated as additional
dnl environments are supported
fs2_os_win32="no"
fs2_os_unix="no"
fs2_os_osx="no"

case $host_os in
    mingw*)
        # windows
        echo "using Win32 defines (for $host_os)"
        fs2_os_win32="yes"
		AC_DEFINE([_MINGW])
        ;;
    linux*)
        # linux
        echo "Using Unix defines (for $host_os)"
        fs2_os_unix="yes"
        ;;
	darwin*)
		# OSX
		echo "Using OSX defines (for $host_os)"
		fs2_os_osx="yes"
		;;
esac

if test "$fs2_os_win32" = "no" && test "$fs2_os_unix" = "no" && test "$fs2_os_osx" = "no"; then
        echo "fs2_open does not support $host_os"
        echo "check the configure.ac file"
        echo "host_cpu:    " $host_cpu
        echo "host_vendor: " $host_vendor
        echo "host_os:     " $host_os
        exit 1;
fi


## define compiler switches
if ! test "$fs2_joystick" = "yes" ; then
        AC_DEFINE([NO_JOYSTICK])
fi
if ! test "$fs2_networking" = "yes" ; then
        AC_DEFINE([NO_NETWORK])
        AC_DEFINE([NO_STANDALONE])
fi
if ! test "$fs2_sound" = "yes" ; then
        AC_DEFINE([NO_SOUND])
fi

if test "$fs2_devil" = "yes" ; then
        AC_DEFINE([USE_DEVIL])
fi

if test "$fs2_speech" = "yes" ; then
		AC_DEFINE([FS2_SPEECH])
fi

if test "$fs2_debug" = "yes" ; then
        AC_DEFINE([_DEBUG])
        AC_SUBST(CXXFLAGS, ["$CFLAGS $CXXFLAGS -O0 -g"])
        AC_SUBST(LDFLAGS, ["-g"])
else
        AC_DEFINE([NDEBUG])
        AC_SUBST(CXXFLAGS, ["$CFLAGS $CXXFLAGS -O2"])
        AC_SUBST(LDFLAGS, [])
fi

AC_DEFINE([MORE_SPECIES])

# generate appropriate conditionals for Makefile.am
AM_CONDITIONAL(FS2_OS_WIN32,   test "$fs2_os_win32" = "yes")
AM_CONDITIONAL(FS2_OS_UNIX,    test "$fs2_os_unix"  = "yes")
AM_CONDITIONAL(FS2_OS_OSX,     test "$fs2_os_osx"   = "yes")

AM_CONDITIONAL(FS2_JOYSTICK,   test "$fs2_joystick"   = "yes")
AM_CONDITIONAL(FS2_NETWORKING, test "$fs2_networking" = "yes")
AM_CONDITIONAL(FS2_SOUND,      test "$fs2_sound"      = "yes")
AM_CONDITIONAL(FS2_DEVIL,      test "$fs2_devil"      = "yes")
AM_CONDITIONAL(FS2_SPEECH,     test "$fs2_speech"     = "yes")
AM_CONDITIONAL(FS2_DEBUG,      test "$fs2_debug"      = "yes")

dnl Check for programs
AC_PROG_CXX

dnl Setup paths for lib and include dirs

if test "$fs2_os_win32" = "yes" ; then
	AC_SUBST(DX8_INCLUDE,	[-I/c/dxsdk/include])
	AC_SUBST(DX8_LIB,		[-L/c/dxsdk/lib])
fi

if test "x$fs2_ogl_libs" != "x" ; then
	AC_SUBST(OGL_LIBDIR, ["-L"$fs2_ogl_libs""])
else
	AC_SUBST(OGL_LIBDIR, [""])
fi

if test "$fs2_os_win32" = "yes" && test "$fs2_devil" = "yes" ; then
	AC_SUBST(DEVIL_LIBS,	["-L/c/devil-sdk/lib -lIL -lILU -lILUT"])
	AC_SUBST(DEVIL_CFLAGS,	["-I/c/devil-sdk/include"])
fi
if test "$fs2_os_unix" = "yes" ; then
if test "$fs2_devil" = "yes" ; then
	AC_SUBST(DEVIL_LIBS,	["-L/usr/lib -lIL -lILU -lILUT"])
	AC_SUBST(DEVIL_CFLAGS,	["-I/usr/include"])
fi
	AC_SUBST(OPENAL_LIBS,	["-lopenal"])
	AC_SUBST(OPENAL_CFLAGS,	[""])
fi
if test "$fs2_os_osx" = "yes" ; then
if test "$fs2_devil" = "yes" ; then
	AC_SUBST(DEVIL_LIBS,	["-framework DevIL"])
	AC_SUBST(DEVIL_CFLAGS,	["-I/System/Library/Frameworks/DevIL.framework/Headers"])
fi
	AC_SUBST(OPENAL_LIBS,	["-framework OpenAL"])
	AC_SUBST(OPENAL_CFLAGS,	["-I/System/Library/Frameworks/OpenAL.framework/Headers"])
	
fi

if test "$fs2_os_unix" = "yes" ; then
	dnl Check for SDL 1.2.0+
	SDL_VERSION=1.2.0
	AM_PATH_SDL($SDL_VERSION,:,AC_MSG_ERROR([*** SDL >= $SDL_VERSION not found!]))
fi

dnl The makefiles we create
AC_OUTPUT([Makefile code/Makefile])

echo
echo Configure options:
echo
if test "x$fs2_ogl_libs" != "x" ; then
echo Using OpenGL libs from . : $fs2_ogl_libs
echo
fi
echo Joystick enabled ....... : $fs2_joystick
echo Networking enabled ..... : $fs2_networking
echo Sound enabled .......... : $fs2_sound
echo Text-to-speech enabled . : $fs2_speech
echo
echo Using DevIL ............ : $fs2_devil
echo
echo Debug build ............ : $fs2_debug
echo
