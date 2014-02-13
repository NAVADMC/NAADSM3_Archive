## Makefile.am -- Process this file with automake to produce Makefile.in

SUBDIRS = . \
  model.disease \
  model.direct-contact-spread \
  model.indirect-contact-spread \
  model.airborne-spread \
  model.detection \
  model.control-measures \
  model.ring-destruction \
  model.ring-vaccination \
  model.zones \
  model.economic

INCLUDES = -I$(top_srcdir) \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/replace \
  -I$(top_srcdir)/herd \
  -I$(top_srcdir)/event \
  -I$(top_srcdir)/prob_dist \
  -I$(top_srcdir)/rel_chart \
  -I$(top_srcdir)/gis \
  -I$(top_srcdir)/reporting \
  -I$(top_srcdir)/spatial_search \
  -I$(top_srcdir)/zone \
  -I$(top_srcdir)/rng \
  -I$(top_srcdir)/wml \
  -I$(top_srcdir)/general \
  -I$(top_srcdir)/models
if USE_SC_GUILIB
  INCLUDES +=  -I$(top_srcdir)/sc_guilib
endif
INCLUDES += @GLIB_CFLAGS@ @GSL_CFLAGS@ @SCEW_CFLAGS@ @SPRNG_CFLAGS@

# Create a program just like the real simulator, but without MPI, for running
# small tests.

noinst_PROGRAMS = mininaadsm shell shell2 fcmp

mininaadsm_SOURCES = $(top_srcdir)/src/cli.c $(top_srcdir)/src/main.c \
  $(top_srcdir)/src/model_loader.c $(top_srcdir)/src/model_loader.h \
  $(top_srcdir)/src/model.h \
  $(top_srcdir)/src/event_manager.c $(top_srcdir)/src/event_manager.h \
  $(top_srcdir)/src/parameter.c $(top_srcdir)/src/parameter.h \
  $(top_srcdir)/src/naadsm.c $(top_srcdir)/src/naadsm.h \
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
  mininaadsm_SOURCES += $(top_srcdir)/sc_guilib/production_type_data.c \
    $(top_srcdir)/sc_guilib/sc_guilib_outputs.c \
    $(top_srcdir)/sc_guilib/sc_database.c
endif
mininaadsm_CPPFLAGS = -DCANCEL_MPI=1 $(AM_CFLAGS)
mininaadsm_LDADD = @LEXLIB@ @GLIB_LIBS@ @GSL_LIBS@ @SCEW_LIBS@ @SPRNG_LIBS@
if HAVE_MPI
  mininaadsm_LDADD += @MPI_LIBS@
endif

shell_SOURCES = shell.c $(top_builddir)/replace/replace.c
shell_LDADD = @GLIB_LIBS@

shell2_SOURCES = shell2.c $(top_builddir)/replace/replace.c
shell2_LDADD = @GLIB_LIBS@

fcmp_SOURCES = fcmp.c
fcmp_LDADD = @GLIB_LIBS@ @GSL_LIBS@

# Create a C file for Doxygen that contains documentation for the test cases.
# Include that in the distribution file, but not the files that generated it.

dist_noinst_DATA = testsuite.c
testsuite_c_SOURCES = tests_start_tag.xml \
  model.disease/all.xml \
  model.direct-contact-spread/all.xml \
  model.indirect-contact-spread/all.xml \
  model.airborne-spread/all.xml \
  model.detection/all.xml \
  model.control-measures/all.xml \
  model.ring-destruction/all.xml \
  model.ring-vaccination/all.xml \
  model.zones/all.xml \
  model.economic/all.xml \
  tests_end_tag.xml

testsuite.c: $(testsuite_c_SOURCES)
	cat $^ | python xml2dox.py > $@

dist_noinst_SCRIPTS = xml2dox.py xml2expect.py

# Include the DejaGnu test files in the distribution file.

EXTRA_DIST = config/unix.exp 1herd.xml 2herds.xml \
  2herds_pigs_cattle_pigs_infected.xml 2herds_pigs_cattle_all_infected.xml \
  2targets_same_dist_same_size.xml 2targets_same_dist_diff_size.xml \
  3herds.xml 3herds_2_at_same_loc.xml 3targets_same_dist_same_size.xml \
  3herds_pigs_2cattle_pigs_infected.xml \
  3herds_pigs_cattle_sheep_sheep_infected.xml \
  3herds_pigs_cattle_sheep_pigs_sheep_infected.xml \
  3herds_pigs_cattle_sheep_all_infected.xml \
  4herds_line.xml 4herds_all_infected.xml 4herds_line_2.xml 4herds_line_3.xml \
  4herds_line_4.xml 4herds_line_5_xy.xml 4herds_2.xml \
  5herds.xml 5herds_2.xml 5herds_very_close.xml 6herds.xml 6herds_2.xml 7herds.xml \
  8herds_all_infected.xml 8herds_2rows.xml 8herds_2rows_2.xml 8herds_2rows_3.xml \
  9herds.xml \
  9herds_hexagon.xml \
  herds_circle.xml herds_circle2.xml 16herds_square.xml \
  1herd_1dayleft.xml 1herd_3daysleft.xml \
  1herd_start_infectious.xml 1herd_start_infectious_1dayleft.xml \
  1herd_start_infectious_2daysleft.xml 1herd_start_subclinical.xml \
  1herd_start_infectious_1dayin.xml \
  1herd_start_infectious_1dayin_4daysleft.xml \
  2herds_1_start_nimmune.xml 2herds_1_start_nimmune_2daysleft.xml \
  2herds_1_start_nimmune_3daysleft.xml 2herds_1_start_vimmune.xml \
  2herds_1_start_vimmune_1dayleft.xml 2herds_1_start_vimmune_2daysleft.xml \
  2herds_1_start_destroyed.xml \
  no_rtree_lat0.xml rtree_lat0.xml no_rtree_lat45.xml rtree_lat45.xml \
  no_rtree_lat-45.xml rtree_lat-45.xml \
  no_rtree_contact_spread.xml rtree_contact_spread.xml \
  2herds_ascii.xml 2herds_iso88591.xml 2herds_utf8.xml 2herds_utf8_no_bom.xml \
  2herds_utf16_big_endian.xml 2herds_utf16_big_endian_no_bom.xml \
  2herds_utf16_little_endian.xml 2herds_utf16_little_endian_no_bom.xml \
  2herds_xy.xml 4herds_line_xy.xml no_rtree_lat45_xy.xml rtree_lat45_xy.xml \
  3herds_1_start_vimmune.xml \
  3herds_1_start_destroyed.xml \
  $(testsuite_c_SOURCES)
