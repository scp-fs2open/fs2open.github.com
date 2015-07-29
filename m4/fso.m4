# Configure paths for SDL
# Sam Lantinga 9/21/99
# stolen from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_SDL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for SDL, and define SDL_CFLAGS and SDL_LIBS
dnl
AC_DEFUN([AM_PATH_SDL],
[dnl
dnl Get the cflags and libraries from the sdl-config script
dnl
AC_ARG_WITH(sdl-prefix,[  --with-sdl-prefix=PFX   Prefix where SDL is installed (optional)],
            sdl_prefix="$withval", sdl_prefix="")
AC_ARG_WITH(sdl-exec-prefix,[  --with-sdl-exec-prefix=PFX Exec prefix where SDL is installed (optional)],
            sdl_exec_prefix="$withval", sdl_exec_prefix="")
AC_ARG_ENABLE(sdltest, [  --disable-sdltest       Do not try to compile and run a test SDL program],
		    , enable_sdltest=yes)

  if test x$sdl_exec_prefix != x ; then
     sdl_args="$sdl_args --exec-prefix=$sdl_exec_prefix"
     if test x${SDL_CONFIG+set} != xset ; then
        SDL_CONFIG=$sdl_exec_prefix/bin/sdl-config
     fi
  fi
  if test x$sdl_prefix != x ; then
     sdl_args="$sdl_args --prefix=$sdl_prefix"
     if test x${SDL_CONFIG+set} != xset ; then
        SDL_CONFIG=$sdl_prefix/bin/sdl-config
     fi
  fi

  AC_REQUIRE([AC_CANONICAL_TARGET])
  PATH="$prefix/bin:$prefix/usr/bin:$PATH"
  AC_PATH_PROG(SDL_CONFIG, sdl-config, no, [$PATH])
  min_sdl_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for SDL - version >= $min_sdl_version)
  no_sdl=""
  if test "$SDL_CONFIG" = "no" ; then
    no_sdl=yes
  else
    SDL_CFLAGS=`$SDL_CONFIG $sdlconf_args --cflags`
    SDL_LIBS=`$SDL_CONFIG $sdlconf_args --libs`

    sdl_major_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sdl_minor_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sdl_micro_version=`$SDL_CONFIG $sdl_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_sdltest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $SDL_CFLAGS"
      CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
      LIBS="$LIBS $SDL_LIBS"
dnl
dnl Now check if the installed SDL is sufficiently new. (Also sanity
dnl checks the results of sdl-config to some extent
dnl
      rm -f conf.sdltest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

char*
my_strdup (char *str)
{
  char *new_str;

  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;

  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sdltest");
  */
  { FILE *fp = fopen("conf.sdltest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sdl_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sdl_version");
     exit(1);
   }

   if (($sdl_major_version > major) ||
      (($sdl_major_version == major) && ($sdl_minor_version > minor)) ||
      (($sdl_major_version == major) && ($sdl_minor_version == minor) && ($sdl_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'sdl-config --version' returned %d.%d.%d, but the minimum version\n", $sdl_major_version, $sdl_minor_version, $sdl_micro_version);
      printf("*** of SDL required is %d.%d.%d. If sdl-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If sdl-config was wrong, set the environment variable SDL_CONFIG\n");
      printf("*** to point to the correct copy of sdl-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_sdl=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_sdl" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])
  else
     AC_MSG_RESULT(no)
     if test "$SDL_CONFIG" = "no" ; then
       echo "*** The sdl-config script installed by SDL could not be found"
       echo "*** If SDL was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the SDL_CONFIG environment variable to the"
       echo "*** full path to sdl-config."
     else
       if test -f conf.sdltest ; then
        :
       else
          echo "*** Could not run SDL test program, checking why..."
          CFLAGS="$CFLAGS $SDL_CFLAGS"
          CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
          LIBS="$LIBS $SDL_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include "SDL.h"

int main(int argc, char *argv[])
{ return 0; }
#undef  main
#define main K_and_R_C_main
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding SDL or finding the wrong"
          echo "*** version of SDL. If it is not finding SDL, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means SDL was incorrectly installed"
          echo "*** or that you have moved SDL since it was installed. In the latter case, you"
          echo "*** may want to edit the sdl-config script: $SDL_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     SDL_CFLAGS=""
     SDL_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(SDL_CFLAGS)
  AC_SUBST(SDL_LIBS)
  rm -f conf.sdltest
])


# Configure paths for libogg
# Jack Moffitt <jack@icecast.org> 10-21-2000
# Shamelessly stolen from Owen Taylor and Manish Singh

dnl XIPH_PATH_OGG([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for libogg, and define OGG_CFLAGS and OGG_LIBS
dnl
AC_DEFUN([XIPH_PATH_OGG],
[dnl
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(ogg,[  --with-ogg=PFX   Prefix where libogg is installed (optional)], ogg_prefix="$withval", ogg_prefix="")
AC_ARG_WITH(ogg-libraries,[  --with-ogg-libraries=DIR   Directory where libogg library is installed (optional)], ogg_libraries="$withval", ogg_libraries="")
AC_ARG_WITH(ogg-includes,[  --with-ogg-includes=DIR   Directory where libogg header files are installed (optional)], ogg_includes="$withval", ogg_includes="")
AC_ARG_ENABLE(oggtest, [  --disable-oggtest       Do not try to compile and run a test Ogg program],, enable_oggtest=yes)

  if test "x$ogg_libraries" != "x" ; then
    OGG_LIBS="-L$ogg_libraries"
  elif test "x$ogg_prefix" != "x" ; then
    OGG_LIBS="-L$ogg_prefix/lib"
  elif test "x$prefix" != "xNONE" ; then
    OGG_LIBS="-L$libdir"
  fi

  OGG_LIBS="$OGG_LIBS -logg"

  if test "x$ogg_includes" != "x" ; then
    OGG_CFLAGS="-I$ogg_includes"
  elif test "x$ogg_prefix" != "x" ; then
    OGG_CFLAGS="-I$ogg_prefix/include"
  elif test "x$prefix" != "xNONE"; then
    OGG_CFLAGS=""
  fi

  AC_MSG_CHECKING(for Ogg)
  no_ogg=""


  if test "x$enable_oggtest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $OGG_CFLAGS"
    LIBS="$LIBS $OGG_LIBS"
dnl
dnl Now check if the installed Ogg is sufficiently new.
dnl
      rm -f conf.oggtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogg/ogg.h>

int main ()
{
  system("touch conf.oggtest");
  return 0;
}

],, no_ogg=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
  fi

  if test "x$no_ogg" = "x" ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])
  else
     AC_MSG_RESULT(no)
     if test -f conf.oggtest ; then
       :
     else
       echo "*** Could not run Ogg test program, checking why..."
       CFLAGS="$CFLAGS $OGG_CFLAGS"
       LIBS="$LIBS $OGG_LIBS"
       AC_TRY_LINK([
#include <stdio.h>
#include <ogg/ogg.h>
],     [ return 0; ],
       [ echo "*** The test program compiled, but did not run. This usually means"
       echo "*** that the run-time linker is not finding Ogg or finding the wrong"
       echo "*** version of Ogg. If it is not finding Ogg, you'll need to set your"
       echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
       echo "*** to the installed location  Also, make sure you have run ldconfig if that"
       echo "*** is required on your system"
       echo "***"
       echo "*** If you have an old version installed, it is best to remove it, although"
       echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
       [ echo "*** The test program failed to compile or link. See the file config.log for the"
       echo "*** exact error that occured. This usually means Ogg was incorrectly installed"
       echo "*** or that you have moved Ogg since it was installed." ])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
     OGG_CFLAGS=""
     OGG_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(OGG_CFLAGS)
  AC_SUBST(OGG_LIBS)
  rm -f conf.oggtest
])

# Configure paths for libvorbis
# Jack Moffitt <jack@icecast.org> 10-21-2000
# Shamelessly stolen from Owen Taylor and Manish Singh
# thomasvs added check for vorbis_bitrate_addblock which is new in rc3

dnl XIPH_PATH_VORBIS([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for libvorbis, and define VORBIS_CFLAGS and VORBIS_LIBS
dnl
AC_DEFUN([XIPH_PATH_VORBIS],
[dnl
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(vorbis,[  --with-vorbis=PFX   Prefix where libvorbis is installed (optional)], vorbis_prefix="$withval", vorbis_prefix="")
AC_ARG_WITH(vorbis-libraries,[  --with-vorbis-libraries=DIR   Directory where libvorbis library is installed (optional)], vorbis_libraries="$withval", vorbis_libraries="")
AC_ARG_WITH(vorbis-includes,[  --with-vorbis-includes=DIR   Directory where libvorbis header files are installed (optional)], vorbis_includes="$withval", vorbis_includes="")
AC_ARG_ENABLE(vorbistest, [  --disable-vorbistest       Do not try to compile and run a test Vorbis program],, enable_vorbistest=yes)

  if test "x$vorbis_libraries" != "x" ; then
    VORBIS_LIBS="-L$vorbis_libraries"
  elif test "x$vorbis_prefix" != "x" ; then
    VORBIS_LIBS="-L$vorbis_prefix/lib"
  elif test "x$prefix" != "xNONE"; then
    VORBIS_LIBS="-L$libdir"
  fi

  VORBIS_LIBS="$VORBIS_LIBS -lvorbis -lm"
  VORBISFILE_LIBS="-lvorbisfile"
  VORBISENC_LIBS="-lvorbisenc"

  if test "x$vorbis_includes" != "x" ; then
    VORBIS_CFLAGS="-I$vorbis_includes"
  elif test "x$vorbis_prefix" != "x" ; then
    VORBIS_CFLAGS="-I$vorbis_prefix/include"
  elif test "x$prefix" != "xNONE"; then
    VORBIS_CFLAGS=""
  fi


  AC_MSG_CHECKING(for Vorbis)
  no_vorbis=""


  if test "x$enable_vorbistest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $VORBIS_CFLAGS $OGG_CFLAGS"
    LIBS="$LIBS $VORBIS_LIBS $VORBISENC_LIBS $OGG_LIBS"
dnl
dnl Now check if the installed Vorbis is sufficiently new.
dnl
      rm -f conf.vorbistest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

int main ()
{
    vorbis_block 	vb;
    vorbis_dsp_state	vd;
    vorbis_info		vi;

    vorbis_info_init (&vi);
    vorbis_encode_init (&vi, 2, 44100, -1, 128000, -1);
    vorbis_analysis_init (&vd, &vi);
    vorbis_block_init (&vd, &vb);
    /* this function was added in 1.0rc3, so this is what we're testing for */
    vorbis_bitrate_addblock (&vb);

    system("touch conf.vorbistest");
    return 0;
}

],, no_vorbis=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
  fi

  if test "x$no_vorbis" = "x" ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])
  else
     AC_MSG_RESULT(no)
     if test -f conf.vorbistest ; then
       :
     else
       echo "*** Could not run Vorbis test program, checking why..."
       CFLAGS="$CFLAGS $VORBIS_CFLAGS"
       LIBS="$LIBS $VORBIS_LIBS $OGG_LIBS"
       AC_TRY_LINK([
#include <stdio.h>
#include <vorbis/codec.h>
],     [ return 0; ],
       [ echo "*** The test program compiled, but did not run. This usually means"
       echo "*** that the run-time linker is not finding Vorbis or finding the wrong"
       echo "*** version of Vorbis. If it is not finding Vorbis, you'll need to set your"
       echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
       echo "*** to the installed location  Also, make sure you have run ldconfig if that"
       echo "*** is required on your system"
       echo "***"
       echo "*** If you have an old version installed, it is best to remove it, although"
       echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
       [ echo "*** The test program failed to compile or link. See the file config.log for the"
       echo "*** exact error that occured. This usually means Vorbis was incorrectly installed"
       echo "*** or that you have moved Vorbis since it was installed." ])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
     VORBIS_CFLAGS=""
     VORBIS_LIBS=""
     VORBISFILE_LIBS=""
     VORBISENC_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(VORBIS_CFLAGS)
  AC_SUBST(VORBIS_LIBS)
  AC_SUBST(VORBISFILE_LIBS)
  AC_SUBST(VORBISENC_LIBS)
  rm -f conf.vorbistest
])

dnl XIPH_PATH_THEORA([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for libtheora, and define THEORA_CFLAGS and THEORA_LIBS
dnl
AC_DEFUN([XIPH_PATH_THEORA],
[dnl
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(theora,[  --with-theora=PFX   Prefix where libtheora is installed (optional)], theora_prefix="$withval", theora_prefix="")
AC_ARG_WITH(theora-libraries,[  --with-theora-libraries=DIR   Directory where libtheora library is installed (optional)], theora_libraries="$withval", theora_libraries="")
AC_ARG_WITH(theora-includes,[  --with-theora-includes=DIR   Directory where libtheora header files are installed (optional)], theora_includes="$withval", theora_includes="")
AC_ARG_ENABLE(theoratest, [  --disable-theoratest       Do not try to compile and run a test Theora program],, enable_theoratest=yes)

  if test "x$theora_libraries" != "x" ; then
    THEORA_LIBS="-L$theora_libraries"
  elif test "x$theora_prefix" != "x" ; then
    THEORA_LIBS="-L$theora_prefix/lib"
  elif test "x$prefix" != "xNONE"; then
    THEORA_LIBS="-L$libdir"
  fi

  THEORA_LIBS="$THEORA_LIBS -ltheora"

  if test "x$theora_includes" != "x" ; then
    THEORA_CFLAGS="-I$theora_includes"
  elif test "x$theora_prefix" != "x" ; then
    THEORA_CFLAGS="-I$theora_prefix/include"
  elif test "x$prefix" != "xNONE"; then
    THEORA_CFLAGS=""
  fi


  AC_MSG_CHECKING(for Theora)
  no_theora=""


  if test "x$enable_theoratest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $THEORA_CFLAGS $OGG_CFLAGS"
    LIBS="$LIBS $THEORA_LIBS $OGG_LIBS"
dnl
dnl Now check if the installed Theora is sufficiently new.
dnl
      rm -f conf.theoratest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <theora/theora.h>
#include <vorbis/codec.h>

int main ()
{
    theora_info		ti;
    theora_state	ts;

    theora_info_init(&ti);

    system("touch conf.theoratest");
    return 0;
}

],, no_theora=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
  fi

  if test "x$no_theora" = "x" ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])
  else
     AC_MSG_RESULT(no)
     if test -f conf.theoratest ; then
       :
     else
       echo "*** Could not run Theora test program, checking why..."
       CFLAGS="$CFLAGS $THEORA_CFLAGS"
       LIBS="$LIBS $THEORA_LIBS $OGG_LIBS"
       AC_TRY_LINK([
#include <stdio.h>
#include <vorbis/codec.h>
],     [ return 0; ],
       [ echo "*** The test program compiled, but did not run. This usually means"
       echo "*** that the run-time linker is not finding Theora or finding the wrong"
       echo "*** version of Theora. If it is not finding Theora, you'll need to set your"
       echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
       echo "*** to the installed location  Also, make sure you have run ldconfig if that"
       echo "*** is required on your system"
       echo "***"
       echo "*** If you have an old version installed, it is best to remove it, although"
       echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
       [ echo "*** The test program failed to compile or link. See the file config.log for the"
       echo "*** exact error that occured. This usually means Theora was incorrectly installed"
       echo "*** or that you have moved Theora since it was installed." ])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
     THEORA_CFLAGS=""
     THEORA_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(THEORA_CFLAGS)
  AC_SUBST(THEORA_LIBS)
  rm -f conf.theoratest
])


dnl AM_PATH_OPENAL([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for OpenAL, and define OPENAL_CFLAGS and OPENAL_LIBS
dnl
AC_DEFUN([AM_PATH_OPENAL],
[dnl
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(openal,[  --with-openal=PFX   Prefix where OpenAL is installed (optional)], openal_prefix="$withval", openal_prefix="")
AC_ARG_WITH(openal-libraries,[  --with-openal-libraries=DIR   Directory where OpenAL library is installed (optional)], openal_libraries="$withval", openal_libraries="")
AC_ARG_WITH(openal-includes,[  --with-openal-includes=DIR   Directory where OpenAL header files are installed (optional)], openal_includes="$withval", openal_includes="")
AC_ARG_ENABLE(openaltest, [  --disable-openaltest       Do not try to compile and run a test OpenAL program],, enable_openaltest=yes)

  if test "x$openal_libraries" != "x" ; then
    OPENAL_LIBS="-L$openal_libraries"
  elif test "x$openal_prefix" != "x" ; then
    OPENAL_LIBS="-L$openal_prefix/lib"
  elif test "x$prefix" != "xNONE"; then
    OPENAL_LIBS="-L$libdir"
  fi

  OPENAL_LIBS="$OPENAL_LIBS -lopenal"

  if test "x$openal_includes" != "x" ; then
    OPENAL_CFLAGS="-I$openal_includes"
  elif test "x$openal_prefix" != "x" ; then
    OPENAL_CFLAGS="-I$openal_prefix/include"
  elif test "x$prefix" != "xNONE"; then
    OPENAL_CFLAGS=""
  fi


  AC_MSG_CHECKING(for OpenAL)
  no_openal=""


  if test "x$enable_openaltest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $OPENAL_CFLAGS"
    LIBS="$LIBS $OPENAL_LIBS"
dnl
dnl Now check if the installed OpenAL is sufficiently new.
dnl
      rm -f conf.openaltest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AL/al.h>
#include <AL/alc.h>

int main ()
{
#ifdef AL_VERSION_1_1
	alcIsExtensionPresent(NULL, (const ALCchar*)"ALC_ENUMERATION_EXT");

	system("touch conf.openaltest");
	return 0;
#else
	printf("\n*** OpenAL version 1.1 or greater is required.\n");
	return 1;
#endif
}

],, no_openal=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
     CFLAGS="$ac_save_CFLAGS"
     LIBS="$ac_save_LIBS"
  fi

  if test "x$no_openal" = "x" ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])
  else
     AC_MSG_RESULT(no)
     if test -f conf.openaltest ; then
       :
     else
       echo "*** Could not run OpenAL test program, checking why..."
       CFLAGS="$CFLAGS $OPENAL_CFLAGS"
       LIBS="$LIBS $OPENAL_LIBS"
       AC_TRY_LINK([
#include <stdio.h>
#include <AL/al.h>
],     [ return 0; ],
       [ echo "*** The test program compiled, but did not run. This usually means"
       echo "*** that the run-time linker is not finding OpenAL or finding the wrong"
       echo "*** version of OpenAL. If it is not finding OpenAL, you'll need to set your"
       echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
       echo "*** to the installed location  Also, make sure you have run ldconfig if that"
       echo "*** is required on your system"
       echo "***"
       echo "*** If you have an old version installed, it is best to remove it, although"
       echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
       [ echo "*** The test program failed to compile or link. See the file config.log for the"
       echo "*** exact error that occured. This usually means OpenAL was incorrectly installed"
       echo "*** or that you have moved OpenAL since it was installed." ])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
     OPENAL_CFLAGS=""
     OPENAL_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(OPENAL_CFLAGS)
  AC_SUBST(OPENAL_LIBS)
  rm -f conf.openaltest
])

dnl AM_PATH_OPENGL([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for OpenGL, and define OPENGL_CFLAGS and OPENGL_LIBS
dnl
AC_DEFUN([AM_PATH_OPENGL],
[dnl
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(opengl,[  --with-opengl=PFX   Prefix where OpenGL is installed (optional)], opengl_prefix="$withval", opengl_prefix="")
AC_ARG_WITH(opengl-libraries,[  --with-opengl-libraries=DIR   Directory where OpenGL library is installed (optional)], opengl_libraries="$withval", opengl_libraries="")
AC_ARG_WITH(opengl-includes,[  --with-opengl-includes=DIR   Directory where OpenGL header files are installed (optional)], opengl_includes="$withval", opengl_includes="")
AC_ARG_ENABLE(opengltest, [  --disable-opengltest       Do not try to compile and run a test OpenGL program],, enable_opengltest=yes)

  if test "x$opengl_libraries" != "x" ; then
    OPENGL_LIBS="-L$opengl_libraries"
  elif test "x$opengl_prefix" != "x" ; then
    OPENGL_LIBS="-L$opengl_prefix/lib"
  elif test "x$prefix" != "xNONE"; then
    OPENGL_LIBS="-L$libdir"
  fi

  OPENGL_LIBS="$OPENGL_LIBS -lGL -lGLU"

  if test "x$opengl_includes" != "x" ; then
    OPENGL_CFLAGS="-I$opengl_includes"
  elif test "x$opengl_prefix" != "x" ; then
    OPENGL_CFLAGS="-I$opengl_prefix/include"
  elif test "x$prefix" != "xNONE"; then
    OPENGL_CFLAGS=""
  fi


  no_opengl=""


  if test "x$enable_opengltest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $OPENGL_CFLAGS"
    LIBS="$LIBS $OPENGL_LIBS"

    AC_CHECK_LIB(GL, glPushMatrix,
      [AC_CHECK_HEADER(GL/gl.h,
        [AC_CHECK_LIB(GLU, gluPerspective,
          [AC_CHECK_HEADER(GL/glu.h, ,
            dnl NOTE: this is a failure
            no_opengl=yes
          )]
        )]
      )]
    )

    CFLAGS="$ac_save_CFLAGS"
    LIBS="$ac_save_LIBS"
  fi

  if test "x$no_opengl" = "x" ; then
     ifelse([$1], , :, [$1])
  else
     OPENGL_CFLAGS=""
     OPENGL_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(OPENGL_CFLAGS)
  AC_SUBST(OPENGL_LIBS)
])
