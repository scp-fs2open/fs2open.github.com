AC_DEFUN([AC_C_COMPILE_FLAGS],[
NEW_CFLAGS="$D_CFLAGS"
for ac_flag in $1
do
 AC_MSG_CHECKING(whether compiler supports $ac_flag)
 D_CFLAGS="$NEW_CFLAGS $ac_flag"
 AC_TRY_COMPILE(,[
  void f() {};
 ],[
  NEW_CFLAGS="$D_CFLAGS"
  AC_MSG_RESULT(yes)
 ],AC_MSG_RESULT(no))
done
D_CFLAGS="$NEW_CFLAGS"
])
