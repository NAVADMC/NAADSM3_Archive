dnl Configure path for SPRNG pseudo-random number generator library
dnl Neil Harvey <neilharvey@canada.com>, July 2003

dnl AM_PATH_SPRNG([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for SPRNG, and define SPRNG_CFLAGS and SPRNG_LIBS.
dnl
AC_DEFUN(AM_PATH_SPRNG,[
  AC_ARG_WITH([sprng],
    AC_HELP_STRING([--with-sprng], [SPRNG installation prefix]),
    [ac_cv_with_sprng_root=${with_sprng}],
    AC_CACHE_CHECK([whether with-sprng was specified], ac_cv_with_sprng_root,
      ac_cv_with_sprng_root=no)
  ) # end of AC_ARG_WITH sprng

  AC_ARG_WITH([sprng-cflags],
    AC_HELP_STRING([--with-sprng-cflags], [SPRNG compile flags]),
    [ac_cv_with_sprng_cflags=${with_sprng_cflags}],
    AC_CACHE_CHECK([whether with-sprng-cflags was specified], ac_cv_with_sprng_cflags,
      ac_cv_with_sprng_cflags=no)
  ) # end of AC_ARG_WITH sprng-cflags

  AC_ARG_WITH([sprng-libs],
    AC_HELP_STRING([--with-sprng-libs], [SPRNG link command arguments]),
    [ac_cv_with_sprng_libs=${with_sprng_libs}],
    AC_CACHE_CHECK([whether with-sprng-libs was specified], ac_cv_with_sprng_libs,
      ac_cv_with_sprng_libs=no)
  ) # end of AC_ARG_WITH sprng-libs

  case "X${ac_cv_with_sprng_cflags}" in Xyes|Xno|X )
    case "X${ac_cv_with_sprng_root}" in Xyes|Xno|X )
      ac_cv_with_sprng_cflags=no ;;
    * )
      ac_cv_with_sprng_cflags="-I${ac_cv_with_sprng_root}/include" ;;
    esac
  esac
  case "X${ac_cv_with_sprng_libs}" in Xyes|Xno|X )
    case "X${ac_cv_with_sprng_root}" in Xyes|Xno|X )
      ac_cv_with_sprng_libs=no ;;
    * )
      ac_cv_with_sprng_libs="-L${ac_cv_with_sprng_root}/lib -lsprng";;
    esac
  esac

  AC_ARG_ENABLE([sprng-test],
    AC_HELP_STRING([--disable-sprngtest], [Do not try to compile and run a test SPRNG program]),
    [ac_cv_with_sprng_test=${with_sprng_test}],
    AC_CACHE_CHECK([whether disable-sprngtest was specified], ac_cv_with_sprng_test,
      ac_cv_with_sprng_test=no)
  ) # end of AC_ARG_WITH libsprng

  ac_save_CPPFLAGS="${CPPFLAGS}"
  ac_save_LIBS="${LIBS}"
  ac_save_DEFS="${DEFS}"
  case "X${ac_cv_with_sprng_cflags}" in Xyes|Xno|X )
    ac_cv_with_sprng_cflags="" ;;
  esac
  SPRNG_CFLAGS="${ac_cv_with_sprng_cflags}"
  CPPFLAGS="${CPPFLAGS} ${SPRNG_CFLAGS}"
  DEFS="${DEFS} -DSIMPLE_SPRNG"
  case "X${ac_cv_with_sprng_libs}" in Xyes|Xno|X )
    ac_cv_with_sprng_libs="-lsprng" ;;
  esac
  SPRNG_LIBS="${ac_cv_with_sprng_libs}"
  LIBS="${LIBS} ${SPRNG_LIBS}"

  dnl
  dnl At this point the SPRNG header file and library information have been
  dnl appended to the CPPFLAGS and LIBS variables.
  dnl 
  dnl Now try to compile and link a program that uses SPRNG.
  dnl

  AC_CACHE_CHECK([for SPRNG compiled without pmlcg generator],[ac_cv_with_sprng],[
  AC_LANG_PUSH(C)
  AC_LINK_IFELSE([[
#include <stdio.h>

#define SIMPLE_SPRNG	
#include <sprng.h>



int
main (int argc, char *argv[])
{
  int *dummy;

  dummy = init_sprng (SPRNG_LFG, make_sprng_seed (), SPRNG_DEFAULT);
  return 0;
}
]],
    [ac_cv_with_sprng=yes],
    [ac_cv_with_sprng=no]) # end of AC_LINK_IFELSE 
  AC_LANG_POP(C)
  ]) # end of AC_CACHE_CHECK for ac_cv_with_sprng

  if test "X${ac_cv_with_sprng}" != Xno
  then
    AC_DEFINE([WITH_SPRNG],[1],
        [Define this if the link test succeeded])
    AC_DEFINE([SIMPLE_SPRNG],[1],
        [Use the simple SPRNG interface])
    ifelse([$1], , :, [$1])
  else
    dnl
    dnl The link test failed, but the user may have compiled SPRNG
    dnl with the pmlcg generator, which needs libgmp, so try again with
    dnl libgmp.
    dnl

    AC_LIB_GMP()

    if test "X${ac_cv_using_lib_gmp}" != Xno
    then
      SPRNG_LIBS="${SPRNG_LIBS} -lgmp"
      LIBS="${LIBS} -lgmp"
      ac_cv_with_sprng_libs="${ac_cv_with_sprng_libs} -lgmp"

      AC_MSG_CHECKING(for SPRNG compiled with pmlcg generator)
      AC_LANG_PUSH(C)
      AC_LINK_IFELSE([[
#include <stdio.h>

#define SIMPLE_SPRNG	
#include <sprng.h>



int
main (int argc, char *argv[])
{
  int *dummy;

  dummy = init_sprng (SPRNG_LFG, make_sprng_seed (), SPRNG_DEFAULT);
  return 0;
}
]],
        [ac_cv_with_sprng=yes],
        [ac_cv_with_sprng=no]) # end of AC_LINK_IFELSE 
      AC_LANG_POP(C)
      AC_MSG_RESULT(${ac_cv_with_sprng})

      if test "X${ac_cv_with_sprng}" != Xno
      then
        AC_DEFINE([WITH_SPRNG],[1],
            [Define to 1 if you have the SPRNG random number generator.])
        AC_DEFINE([SIMPLE_SPRNG],[1],
            [Define to use the simple SPRNG interface.])
        ifelse([$1], , :, [$1])
      else
        SPRNG_CFLAGS=""
        SPRNG_LIBS=""
        AC_DEFINE([WITHOUT_SPRNG],[1],
            [Define this if the link test failed.])
        ifelse([$2], , :, [$2])
      fi

    else  
      SPRNG_CFLAGS=""
      SPRNG_LIBS=""
      AC_DEFINE([WITHOUT_SPRNG],[1],
          [Define this if the link test failed.])
      ifelse([$2], , :, [$2])
    fi
  fi
  CPPFLAGS="${ac_save_CPPFLAGS}"
  LIBS="${ac_save_LIBS}"
  AC_SUBST([SPRNG_CFLAGS])
  AC_SUBST([SPRNG_LIBS])
  
]) # end of AC_DEFUN of AC_WITHLIB_SPRNG
dnl
dnl sprng.m4 ends here
