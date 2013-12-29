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

for AM in automake-1.11 automake-1.10 automake-1.9 automake-1.8 automake-1.7 automake-1.6 automake; do
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
  for ACL in aclocal-1.11 aclocal-1.10 aclocal-1.9 aclocal-1.8 aclocal-1.7 aclocal-1.6 aclocal; do
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
      echo "Running autoreconf..."
      autoreconf --install
    )
  fi
done

#conf_flags="--enable-maintainer-mode --enable-compile-warnings" #--enable-iso-c

# to not run configure, call autogen.sh as,
# NOCONFIGURE=1 ./autogen.sh
if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile $PKG_NAME.
else
  echo Skipping configure process.
fi
