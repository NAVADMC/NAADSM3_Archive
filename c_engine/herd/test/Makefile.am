## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I.. -I$(top_srcdir)/src -I$(top_srcdir)/rel_chart -I$(top_srcdir)/general -I$(top_srcdir)/zone \
  -I$(top_srcdir)/spatial_search \
  @GLIB_CFLAGS@ @GSL_CFLAGS@
if USE_SC_GUILIB
  INCLUDES += -I$(top_srcdir)/sc_guilib
endif


AM_YFLAGS = -d -v

noinst_PROGRAMS = shell
shell_SOURCES = shell.y scanner.l ../herd.c \
  $(top_srcdir)/rel_chart/rel_chart.c \
  $(top_srcdir)/replace/replace.c
if USE_SC_GUILIB
  shell_SOURCES += $(top_srcdir)/general/general.c \
    $(top_srcdir)/zone/zone.c \
    $(top_srcdir)/sc_guilib/production_type_data.c \
    $(top_srcdir)/sc_guilib/sc_guilib_outputs.c \
    $(top_srcdir)/sc_guilib/sc_database.c
endif
shell_LDADD = @GLIB_LIBS@ @GSL_LIBS@
if HAVE_MPI
  shell_LDADD += @MPI_LIBS@
endif

# Include the DejaGnu test files in the distribution file.

EXTRA_DIST = config/unix.exp libherd.all/*.exp
