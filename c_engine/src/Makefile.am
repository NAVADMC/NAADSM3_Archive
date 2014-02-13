## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir) \
  -I$(top_srcdir)/replace \
  -I$(top_srcdir)/herd \
  -I$(top_srcdir)/event \
  -I$(top_srcdir)/prob_dist \
  -I$(top_srcdir)/rel_chart \
  -I$(top_srcdir)/gis \
  -I$(top_srcdir)/reporting \
  -I$(top_srcdir)/rng \
  -I$(top_srcdir)/spatial_search \
  -I$(top_srcdir)/zone \
  -I$(top_srcdir)/wml \
  -I$(top_srcdir)/general \
  -I$(top_srcdir)/models
if USE_SC_GUILIB
  INCLUDES +=  -I$(top_srcdir)/sc_guilib 
endif
INCLUDES += @GLIB_CFLAGS@ @GSL_CFLAGS@ @SCEW_CFLAGS@ @SPRNG_CFLAGS@ 
if HAVE_MPI
INCLUDES += @MPI_CFLAGS@
endif

bin_PROGRAMS = naadsm
naadsm_SOURCES = cli.c main.c model_loader.c model_loader.h model.h \
  event_manager.c event_manager.h \
  parameter.c parameter.h \
  naadsm.c naadsm.h \
  $(top_srcdir)/replace/replace.c $(top_srcdir)/replace/replace.h \
  $(top_srcdir)/general/general.c $(top_srcdir)/general/general.h \
  $(top_srcdir)/herd/herd.c $(top_srcdir)/herd/herd.h \
  $(top_srcdir)/event/event.c $(top_srcdir)/event/event.h \
  $(top_srcdir)/prob_dist/prob_dist.c $(top_srcdir)/prob_dist/prob_dist.h \
  $(top_srcdir)/rel_chart/rel_chart.c $(top_srcdir)/rel_chart/rel_chart.h \
  $(top_srcdir)/gis/gis.c $(top_srcdir)/gis/gis.h \
  $(top_srcdir)/reporting/reporting.c $(top_srcdir)/reporting/reporting.h \
  $(top_srcdir)/rng/rng.c $(top_srcdir)/rng/rng.h \
  $(top_srcdir)/spatial_search/spatial_search.c $(top_srcdir)/spatial_search/spatial_search.h \
  $(top_srcdir)/spatial_search/ch2d.c $(top_srcdir)/spatial_search/ch2d.h \
  $(top_srcdir)/zone/zone.c $(top_srcdir)/zone/zone.h \
  $(top_srcdir)/wml/wml.c $(top_srcdir)/wml/wml.h \
  $(top_srcdir)/models/model_util.c $(top_srcdir)/models/model_util.h \
  $(top_srcdir)/models/conflict-resolver.c $(top_srcdir)/models/conflict-resolver.h \
  $(top_srcdir)/models/disease-model.c $(top_srcdir)/models/disease-model.h \
  $(top_srcdir)/models/contact-spread-model.c $(top_srcdir)/models/contact-spread-model.h \
  $(top_srcdir)/models/contact-recorder-model.c $(top_srcdir)/models/contact-recorder-model.h \
  $(top_srcdir)/models/airborne-spread-model.c $(top_srcdir)/models/airborne-spread-model.h \
  $(top_srcdir)/models/airborne-spread-exponential-model.c $(top_srcdir)/models/airborne-spread-exponential-model.h \
  $(top_srcdir)/models/detection-model.c $(top_srcdir)/models/detection-model.h \
  $(top_srcdir)/models/vaccine-model.c $(top_srcdir)/models/vaccine-model.h \
  $(top_srcdir)/models/resources-and-implementation-of-controls-model.c $(top_srcdir)/models/resources-and-implementation-of-controls-model.h \
  $(top_srcdir)/models/quarantine-model.c $(top_srcdir)/models/quarantine-model.h \
  $(top_srcdir)/models/basic-destruction-model.c $(top_srcdir)/models/basic-destruction-model.h \
  $(top_srcdir)/models/trace-back-destruction-model.c $(top_srcdir)/models/trace-back-destruction-model.h \
  $(top_srcdir)/models/ring-destruction-model.c $(top_srcdir)/models/ring-destruction-model.h \
  $(top_srcdir)/models/ring-vaccination-model.c $(top_srcdir)/models/ring-vaccination-model.h \
  $(top_srcdir)/models/trace-model.c $(top_srcdir)/models/trace-model.h \
  $(top_srcdir)/models/trace-destruction-model.c $(top_srcdir)/models/trace-destruction-model.h \
  $(top_srcdir)/models/trace-quarantine-model.c $(top_srcdir)/models/trace-quarantine-model.h \
  $(top_srcdir)/models/trace-exam-model.c $(top_srcdir)/models/trace-exam-model.h \
  $(top_srcdir)/models/test-model.c $(top_srcdir)/models/test-model.h \
  $(top_srcdir)/models/zone-model.c $(top_srcdir)/models/zone-model.h \
  $(top_srcdir)/models/basic-zone-focus-model.c $(top_srcdir)/models/basic-zone-focus-model.h \
  $(top_srcdir)/models/trace-back-zone-focus-model.c $(top_srcdir)/models/trace-back-zone-focus-model.h \
  $(top_srcdir)/models/trace-zone-focus-model.c $(top_srcdir)/models/trace-zone-focus-model.h \
  $(top_srcdir)/models/economic-model.c $(top_srcdir)/models/economic-model.h \
  $(top_srcdir)/models/destruction-monitor.c $(top_srcdir)/models/destruction-monitor.h \
  $(top_srcdir)/models/destruction-list-monitor.c $(top_srcdir)/models/destruction-list-monitor.h \
  $(top_srcdir)/models/trace-back-monitor.c $(top_srcdir)/models/trace-back-monitor.h \
  $(top_srcdir)/models/trace-monitor.c $(top_srcdir)/models/trace-monitor.h \
  $(top_srcdir)/models/vaccination-monitor.c $(top_srcdir)/models/vaccination-monitor.h \
  $(top_srcdir)/models/vaccination-list-monitor.c $(top_srcdir)/models/vaccination-list-monitor.h \
  $(top_srcdir)/models/detection-monitor.c $(top_srcdir)/models/detection-monitor.h \
  $(top_srcdir)/models/exposure-monitor.c $(top_srcdir)/models/exposure-monitor.h \
  $(top_srcdir)/models/infection-monitor.c $(top_srcdir)/models/infection-monitor.h \
  $(top_srcdir)/models/zone-monitor.c $(top_srcdir)/models/zone-monitor.h \
  $(top_srcdir)/models/exam-monitor.c $(top_srcdir)/models/exam-monitor.h \
  $(top_srcdir)/models/test-monitor.c $(top_srcdir)/models/test-monitor.h \
  $(top_srcdir)/models/full-table-writer.c $(top_srcdir)/models/full-table-writer.h \
  $(top_srcdir)/models/apparent-events-table-writer.c $(top_srcdir)/models/apparent-events-table-writer.h
if USE_SC_GUILIB
  naadsm_SOURCES += $(top_srcdir)/sc_guilib/production_type_data.c \
    $(top_srcdir)/sc_guilib/production_type_data.h \
    $(top_srcdir)/sc_guilib/sc_guilib_outputs.c \
    $(top_srcdir)/sc_guilib/sc_guilib_outputs.h \
    $(top_srcdir)/sc_guilib/sc_database.c \
    $(top_srcdir)/sc_guilib/sc_database.h
endif
if HAVE_MPI
  naadsm_SOURCES += mpix.c mpix.h
endif
naadsm_LDADD = @LEXLIB@ @GLIB_LIBS@ @GSL_LIBS@ @SCEW_LIBS@ @SPRNG_LIBS@
if HAVE_MPI
  naadsm_LDADD += @MPI_LIBS@
endif
