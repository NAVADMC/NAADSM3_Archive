## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/herd -I$(top_srcdir)/rel_chart \
  -I$(top_srcdir)/spatial_search \
  -I$(top_srcdir)/general -I$(top_srcdir)/sc_guilib -I$(top_srcdir)/zone
INCLUDES += @GLIB_CFLAGS@ @GSL_CFLAGS@

AM_YFLAGS = -d -v

bin_PROGRAMS = smherds2xml
smherds2xml_SOURCES = main.c parser.y scanner.l \
  $(top_srcdir)/herd/herd.c \
  $(top_srcdir)/rel_chart/rel_chart.c \
  $(top_srcdir)/replace/replace.c
if USE_SC_GUILIB
  smherds2xml_SOURCES += $(top_srcdir)/general/general.c \
    $(top_srcdir)/zone/zone.c \
    $(top_srcdir)/sc_guilib/production_type_data.c \
    $(top_srcdir)/sc_guilib/sc_guilib_outputs.c \
    $(top_srcdir)/sc_guilib/sc_database.c
endif
smherds2xml_LDADD = @LEXLIB@ @GLIB_LIBS@ @GSL_LIBS@
if HAVE_MPI
smherds2xml_LDADD += @MPI_LIBS@
endif
