#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
PKG_NAME="fs2_open"

# check the version of a package
check_version() {
  COMMAND=$1
  MAJOR=$2
  MINOR=$3
  MICRO=$4

  ($COMMAND --version) </dev/null >/dev/null 2>&1 || return 1

  ver=`$COMMAND --version|head -n 1|sed 's/^.*) //'|sed 's/ (.*)//'`
  major=`echo $ver | cut -d. -f1 | sed s/[a-zA-Z\-].*//g`
  minor=`echo $ver | cut -d. -f2 | sed s/[a-zA-Z\-].*//g`
  micro=`echo $ver | cut -d. -f3 | sed s/[a-zA-Z\-].*//g`
  test -z "$major" && major=0
  test -z "$minor" && minor=0
  test -z "$micro" && micro=0

  fail=0
  if [ ! "$major" -gt "$MAJOR" ]; then
    if [ "$major" -lt "$MAJOR" ]; then
      fail=1
    elif [ ! "$minor" -gt "$MINOR" ]; then
      if [ "$minor" -lt "$MINOR" ]; then
        fail=1
      elif [ "$micro" -lt "$MICRO" ]; then
        fail=1
      fi
    fi
  fi

  return "$fail"
}

AUTOMAKE=
ACLOCAL=
DIE=0

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`autoconf' installed to."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

(grep "^AM_PROG_LIBTOOL" $srcdir/configure.ac >/dev/null) && {
  (libtool --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`libtool' installed."
    echo "Get ftp://ftp.gnu.org/pub/gnu/"
    echo "(or a newer version if it is available)"
    DIE=1
  }
}

for AM in automake-1.9 automake-1.8 automake-1.7 automake-1.6 automake; do
  (check_version "$AM" 1 6 1) && {
    AUTOMAKE="$AM"
    break
  }
done

if [ -z "$AUTOMAKE" ]; then
  echo
  echo "**Error**: You must have \`automake' version 1.6.1 or greater"
  echo "installed to compile $PKG_NAME. Download the appropriate package"
  echo "for your distribution, or get the source tarball at"
  echo "ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
fi

# if no automake, don't bother testing for aclocal
test -z "$AUTOMAKE" || {
  for ACL in aclocal-1.9 aclocal-1.8 aclocal-1.7 aclocal-1.6 aclocal; do
    (check_version "$ACL" 1 6 1) && {
      ACLOCAL="$ACL"
      break
    }
  done

  if [ -z "$ACLOCAL" ]; then
    echo
    echo "**Error**: Missing \`aclocal'. The version of \`automake'"
    echo "installed seems to be outdated. Get a recent package from"
    echo "ftp://ftp.gnu.org/pub/gnu/"
    DIE=1
  fi
}

if test "$DIE" -eq 1; then
  exit 1
fi

if test -z "$*"; then
  echo "**Warning**: I am going to run \`configure' with no arguments."
  echo "If you wish to pass any to it, please specify them on the"
  echo \`$0\'" command line."
  echo
fi

case $CC in
xlc )
  am_opt=--include-deps;;
esac

for coin in `find $srcdir -name configure.ac -print`
do 
  dr=`dirname $coin`
  if test -f $dr/NO-AUTO-GEN; then
    echo skipping $dr -- flagged as no auto-gen
  else
    echo processing $dr
    macrodirs=`sed -n -e 's,AM_ACLOCAL_INCLUDE(\(.*\)),\1,gp' < $coin`
    ( cd $dr
      aclocalinclude="$ACLOCAL_FLAGS"
      for k in $macrodirs; do
  	if test -d $k; then
          aclocalinclude="$aclocalinclude -I $k"
  	##else 
	##  echo "**Warning**: No such directory \`$k'.  Ignored."
        fi
      done
      if grep "^AM_GNU_GETTEXT" configure.ac >/dev/null; then
	if grep "sed.*POTFILES" configure.ac >/dev/null; then
	  : do nothing -- we still have an old unmodified configure.ac
	else
	  echo "Creating $dr/aclocal.m4 ..."
	  test -r $dr/aclocal.m4 || touch $dr/aclocal.m4
	  echo "Running gettextize...  Ignore non-fatal messages."
	  ./setup-gettext
	  echo "Making $dr/aclocal.m4 writable ..."
	  test -r $dr/aclocal.m4 && chmod u+w $dr/aclocal.m4
        fi
      fi
      if grep "^AM_GNOME_GETTEXT" configure.ac >/dev/null; then
	echo "Creating $dr/aclocal.m4 ..."
	test -r $dr/aclocal.m4 || touch $dr/aclocal.m4
	echo "Running gettextize...  Ignore non-fatal messages."
	./setup-gettext
	echo "Making $dr/aclocal.m4 writable ..."
	test -r $dr/aclocal.m4 && chmod u+w $dr/aclocal.m4
      fi
      if grep "^AM_PROG_LIBTOOL" configure.ac >/dev/null; then
	echo "Running libtoolize..."
	libtoolize --force --copy
      fi
      echo "Running $ACLOCAL $aclocalinclude ..."
      "$ACLOCAL" $aclocalinclude
      if grep "^AM_CONFIG_HEADER" configure.ac >/dev/null; then
	echo "Running autoheader..."
	autoheader
      fi
      echo "Running $AUTOMAKE --add-missing --copy --gnu $am_opt ..."
      "$AUTOMAKE" --add-missing --copy --gnu $am_opt
      echo "Running autoconf..."
      autoconf
    )
  fi
done

#conf_flags="--enable-maintainer-mode --enable-compile-warnings" #--enable-iso-c

if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile $PKG_NAME.
else
  echo Skipping configure process.
fi
