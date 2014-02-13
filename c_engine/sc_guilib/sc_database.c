#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef USE_SC_GUILIB

#if STDC_HEADERS
#  include <string.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#if HAVE_MATH_H
#  include <math.h>
#endif

#if defined( USE_MPI ) && !CANCEL_MPI
  #include <mpi.h>
  #include <mpix.h>
#endif

#include <time.h>
#include <glib.h>

#include <general.h>
#include <sc_database.h>

extern const char HRD_APPARENT_STATE_CHAR[7];
extern const char HRD_STATE_CHAR[HRD_NSTATES];

void write_production_type_list_results_SQL( GPtrArray *_production_type_list, unsigned int _run )
{
  int i;
  guint run_val;
  HRD_production_type_data_t *p_data;
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_production_type_list_results_SQL");
#endif
#if defined( USE_MPI ) && !CANCEL_MPI 
  run_val = (me.rank * _scenario.nruns) + _run + 1;
#else
  run_val = _run + 1;
#endif
 
  if ( _production_type_list != NULL )
  {
    for( i = 0; i < _production_type_list->len; i++ )
    {
      p_data = (HRD_production_type_data_t*)(g_ptr_array_index (_production_type_list, i ));
      if ( p_data != NULL )
      {
        g_print( "INSERT INTO outIterationByProductionType ( jobID, iteration, productiontypeID, tscUSusc, tscASusc, tscULat, tscALat, tscUSubc, tscASubc, tscUClin, tscAClin, tscUNImm, tscANImm, tscUVImm, tscAVImm, tscUDest, tscADest, infcUIni, infcAIni, infcUAir, infcAAir, infcUDir, infcADir, infcUInd, infcAInd, expcUDir, expcADir, expcUInd, expcAInd, trcUDir, trcADir, trcUInd, trcAInd, trcUDirp, trcADirp, trcUIndp, trcAIndp, detcUClin, detcAClin, firstDetection, descUIni, descAIni, descUDet, descADet, descUDir, descADir, descUInd, descAInd, descURing, descARing, firstDestruction, vaccUIni, vaccAIni, vaccURing, vaccARing, firstVaccination, zoncFoci) VALUES( %s, %i, %i, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %i, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %i, %lu, %lu, %lu, %lu, %i, %lu );\n", _scenario.scenarioId, run_val, p_data->id, p_data->data.tscUSusc, p_data->data.tscASusc, p_data->data.tscULat, p_data->data.tscALat, p_data->data.tscUSubc, p_data->data.tscASubc, p_data->data.tscUClin, p_data->data.tscAClin, p_data->data.tscUNImm, p_data->data.tscANImm, p_data->data.tscUVImm, p_data->data.tscAVImm, p_data->data.tscUDest, p_data->data.tscADest, p_data->data.infcUIni, p_data->data.infcAIni, p_data->data.infcUAir, p_data->data.infcAAir, p_data->data.infcUDir, p_data->data.infcADir, p_data->data.infcUInd, p_data->data.infcAInd, p_data->data.expcUDir, p_data->data.expcADir, p_data->data.expcUInd, p_data->data.expcAInd, p_data->data.trcUDir, p_data->data.trcADir, p_data->data.trcUInd, p_data->data.trcAInd, p_data->data.trcUDirp, p_data->data.trcADirp, p_data->data.trcUIndp, p_data->data.trcAIndp, p_data->data.detcUClin, p_data->data.detcAClin, p_data->data.firstDetection, p_data->data.descUIni, p_data->data.descAIni, p_data->data.descUDet, p_data->data.descADet, p_data->data.descUDir, p_data->data.descADir, p_data->data.descUInd, p_data->data.descAInd, p_data->data.descURing, p_data->data.descARing, p_data->data.firstDestruction, p_data->data.vaccUIni, p_data->data.vaccAIni, p_data->data.vaccURing, p_data->data.vaccARing, p_data->data.firstVaccination, p_data->data.zoncFoci );
      };
    };
  };
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_production_type_list_results_SQL");
#endif
}

void write_herds_ever_infected_SQL( HRD_herd_list_t *_herds )
{
  int i;
  guint length;
  HRD_herd_t * _herd;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_herds_ever_infected_SQL");
#endif

  if ( _herds != NULL )
  {
    length = HRD_herd_list_length( _herds );
    for( i = 0; i < length; i++ )
    {
      _herd = HRD_herd_list_get( _herds, i );
      if ( _herd != NULL )
      {
        if ( _herd->ever_infected )
        {
          char _status[50];
          switch ( _herd->status )
          {
            case Susceptible:
              sprintf( _status, "Susceptible" );
              break;

            case Latent:
              sprintf( _status, "Latent" );
              break;

            case InfectiousSubclinical:
              sprintf( _status, "InfectiousSubclinical" );
              break;

            case InfectiousClinical:
              sprintf( _status, "InfectiousClinical" );
              break;

            case NaturallyImmune:
              sprintf( _status, "NaturallyImmune" );
              break;

            case VaccineImmune:
              sprintf( _status, "VaccineImmune" );
              break;

            case Destroyed:
              sprintf( _status, "Destroyed" );
              break;
          };

          g_print( "Herd: %s was first infected on day %i, and is currently %s and has been in that status for %i days\n",
                     _herd->official_id, _herd->day_first_infected, _status, _herd->days_in_status );
        }
      };
    };
  };

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_herds_ever_infected_SQL");
#endif
}

void write_outIteration_SQL( guint _run )
{
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_outIteration_SQL");
#endif

#if defined( USE_MPI ) && !CANCEL_MPI
  g_print( "INSERT INTO outIteration ( jobID, iteration ) VALUES ( %s, %i );\n",
            _scenario.scenarioId,  (me.rank * _scenario.nruns) + _run + 1
		 );
#else
  g_print( "INSERT INTO outIteration ( jobID, iteration ) VALUES ( %s, %i );\n",
            _scenario.scenarioId,  _run + 1
		 );
#endif

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_outIteration_SQL");
#endif
}

void update_outIteration_SQL( guint _run )
{
  guint iteration;
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_outIteration_SQL");
#endif

#if defined( USE_MPI ) && !CANCEL_MPI
  iteration = (me.rank * _scenario.nruns) + _run + 1;
#else
  iteration = _run + 1;
#endif
  
  g_print( "UPDATE outIteration set diseaseEnded=%s, diseaseEndDay=%i, outbreakEnded=%s, outbreakEndDay=%i, zoneFociCreated=%i  WHERE jobID=%s AND iteration=%i;\n",
             (( _iteration.diseaseEndDay != -1 )?  "TRUE":"FALSE" ), _iteration.diseaseEndDay, ((_iteration.outbreakEndDay != -1 )? "TRUE":"FALSE"),
            _iteration.outbreakEndDay, ((_iteration.zoneFociCreated)? -1: 0 ), _scenario.scenarioId,  iteration );

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_outIteration_SQL");
#endif
}

void write_outIterationByZone_SQL( guint _run, ZON_zone_list_t *_zones )
{
  guint i, list_len;
  ZON_zone_t *_zone;
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_outIterationByZone_SQL");
#endif

  list_len =  ZON_zone_list_length( _zones );

  for ( i = 0; i < list_len; i++ )
  {
    if ( (_zone = ZON_zone_list_get( _zones, i )) != NULL )
    {
#if defined( USE_MPI ) && !CANCEL_MPI
      g_print( "INSERT INTO outIterationByZone ( jobID, iteration, zoneID, maxArea, maxAreaDay, finalArea ) VALUES ( %s, %i, %i, %g, %i, %g );\n",
             _scenario.scenarioId,  (me.rank * _scenario.nruns) + _run + 1, _zone->level, _zone->max_area, _zone->max_day, _zone->area );
#else
      g_print( "INSERT INTO outIterationByZone ( jobID, iteration, zoneID, maxArea, maxAreaDay, finalArea ) VALUES ( %s, %i, %i, %g, %i, %g );\n",
              _scenario.scenarioId,  _run + 1, _zone->level, _zone->max_area, _zone->max_day, _zone->area );
#endif
    };
  };
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_outIterationByZone_SQL");
#endif
}

void for_each_write_SQL( gpointer key, gpointer value, gpointer user_data)
{
  by_zone_prod_data *temp_data;
  guint _prodId, _animalCount, _herdCount;
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER for_each_write_SQL");
#endif

  temp_data = (by_zone_prod_data*)user_data;
  _prodId = GPOINTER_TO_UINT( key );
  _herdCount = GPOINTER_TO_UINT( value );
  _animalCount = GPOINTER_TO_UINT( g_hash_table_lookup( temp_data->_animalDays, key ) );

#if defined( USE_MPI ) && !CANCEL_MPI
  g_print("INSERT INTO outIterationByZoneAndProductionType ( jobID, iteration, zoneID, productionTypeID, unitDaysInZone, animalDaysInZone ) VALUES( %s, %i, %i, %i, %i, %i );\n",
           _scenario.scenarioId,  (me.rank * _scenario.nruns) + temp_data->_run + 1, temp_data->_zone_level, _prodId, _herdCount, _animalCount );
#else
  g_print("INSERT INTO outIterationByZoneAndProductionType ( jobID, iteration, zoneID, productionTypeID, unitDaysInZone, animalDaysInZone ) VALUES( %s, %i, %i, %i, %i, %i );\n",
           _scenario.scenarioId, temp_data->_run + 1, temp_data->_zone_level, _prodId, _herdCount, _animalCount );
#endif
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT for_each_write_SQL");
#endif
}

void write_outIterationByZoneAndProductiontype_SQL( guint _run, ZON_zone_list_t *_zones )
{
  guint i, j, k, list_len;
  guint _prodId, _animalCount, _herdCount;
  GList *herdKeys;

  ZON_zone_t *_zone;
  GHashTable *_herdDays;
  GHashTable *_animalDays;
  by_zone_prod_data *temp_data;
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_outIterationByZoneAndProductiontype_SQL");
#endif
  temp_data = g_new( by_zone_prod_data, 1 );

  list_len =  ZON_zone_list_length( _zones );

  for( i = 0; i < list_len; i++ )
  {
    if ( (_zone = ZON_zone_list_get( _zones, i )) != NULL )
    {
      _herdDays = _zone->_herdDays;
      _animalDays = _zone->_animalDays;

      if ( ( _herdDays != NULL ) && ( _animalDays != NULL ) )
      {
        if ( ( g_hash_table_size( _herdDays ) ) ==  ( g_hash_table_size( _animalDays ) ) )
        {
          temp_data->_run = _run;
    	  temp_data->_zone_level = _zone->level;
	      temp_data->_animalDays = _animalDays;

    	  g_hash_table_foreach( _herdDays, &for_each_write_SQL, temp_data );
        };
      };
    };
  };

  g_free( temp_data );
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_outIterationByZoneAndProductiontype_SQL");
#endif
}

void write_scenario_SQL( void )
{
  char start_time[128];
  struct tm *local_time;
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_scenario_SQL");
#endif
  memset( start_time, 0, 128 );
  
 local_time = localtime( &_scenario.start_time );  
  sprintf( start_time, "%04d%02d%02d%02d%02d%02d", local_time->tm_year + 1900, local_time->tm_mon + 1, 
 		   local_time->tm_mday, local_time->tm_hour, local_time->tm_min, local_time->tm_sec );
  
  start_time[ strlen( start_time ) ] = '\0';

  g_print( "INSERT INTO scenario ( scenarioID, descr, nIterations, isComplete, lastUpdated ) VALUES ( %s, '%s', %i, FALSE, '%s' );\n",
	  ((_scenario.scenarioId == NULL )? "0": _scenario.scenarioId ), ((_scenario.description == NULL)? "NONE":_scenario.description), _scenario.nruns, start_time );

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_scenario_SQL");
#endif
}

void write_job_SQL()
{
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_job_SQL");
#endif
  g_print( "INSERT INTO job ( jobID, scenarioID ) VALUES ( %s, %s );\n", _scenario.scenarioId, _scenario.scenarioId );
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_job_SQL");
#endif
}

void write_production_types_SQL( GPtrArray *production_types )
{
  guint i;
  HRD_production_type_data_t *prod;
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_production_types_SQL");
#endif
  for( i = 0; i < production_types->len; i++ )
  {
    prod = ( HRD_production_type_data_t *) g_ptr_array_index( production_types, i );
    if ( prod != NULL )
        g_print( "INSERT INTO inProductionType ( scenarioID, productionTypeID, descr) VALUES ( %s, %i, '%s' );\n",
                 _scenario.scenarioId, prod->id, prod->name );
  }
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_production_types_SQL");
#endif
}

void write_zones_SQL( ZON_zone_list_t *zones )
{
  guint i;
  ZON_zone_t *zone;
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_zones_SQL");
#endif
  for( i = 0; i < zones->list->len; i++ )
  {
    zone = ZON_zone_list_get( zones, i );
    if ( zone != NULL )
      g_print( "INSERT INTO inZone ( zoneID, descr, scenarioID ) VALUES ( %i, '%s', %s);\n",
                zone->level, zone->name, _scenario.scenarioId );
  };
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_zones_SQL");
#endif
}

void write_epi_curve_daily_data_SQL( GPtrArray *_production_types, guint _run, guint _day )
{
  guint i;
  HRD_production_type_data_t *prod;
  guint run_val;
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_epi_curve_daily_data_SQL");
#endif

#if defined( USE_MPI ) && !CANCEL_MPI
  run_val = (me.rank * _scenario.nruns) + _run + 1;
#else
  run_val = _run + 1;
#endif

  for( i = 0; i < _production_types->len; i++ )
  {
    prod = ( HRD_production_type_data_t *) g_ptr_array_index( _production_types, i );
    if ( prod != NULL )
    {
      g_print( "INSERT INTO outEpidemicCurves ( jobID, iteration, day, productionTypeID, infectedUnits, infectedAnimals, detectedUnits, detectedAnimals, infectiousUnits, apparentInfectiousUnits ) VALUES ( %s, %i, %i, %i, %i, %i, %i, %i, %i, %i );\n",
              _scenario.scenarioId,  run_val,
	      _day, prod->id,
              prod->d_data.infnUDir + prod->d_data.infnUInd + prod->d_data.infnUAir,
              prod->d_data.infnADir + prod->d_data.infnAInd + prod->d_data.infnAAir,
              prod->d_data.detnUClin,
              prod->d_data.detnAClin,
              prod->d_data.tsdUSubc + prod->d_data.tsdUClin,
              prod->d_data.appUInfectious );
    };
  }
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_epi_curve_daily_data_SQL");
#endif
}

void write_out_daily_by_production_type_SQL( guint _day, guint _run, GPtrArray *_production_types )
{
  guint run_val;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_out_daily_by_production_type_SQL");
#endif
  guint _prodId, i;
  HRD_production_type_data_t *prod;

#if defined( USE_MPI ) && !CANCEL_MPI
  run_val = (me.rank * _scenario.nruns) + _run + 1;
#else
  run_val = _run + 1;
#endif

  for( i = 0; i < _production_types->len; i++ )
  {
    prod = ( HRD_production_type_data_t *) g_ptr_array_index( _production_types, i );
    if ( prod != NULL )
      g_print( "INSERT INTO outDailyByProductionType (jobID, iteration,day,productionTypeID,tsdUSusc,tsdASusc,tsdULat,tsdALat,tsdUSubc,tsdASubc,tsdUClin,tsdAClin,tsdUNImm,tsdANImm,tsdUVImm,tsdAVImm,tsdUDest,tsdADest,tscUSusc,tscASusc,tscULat,tscALat,tscUSubc,tscASubc,tscUClin,tscAClin,tscUNImm,tscANImm,tscUVImm,tscAVImm,tscUDest,tscADest,infnUAir,infnAAir,infnUDir,infnADir,infnUInd,infnAInd,infcUIni,infcAIni,infcUAir,infcAAir,infcUDir,infcADir,infcUInd,infcAInd,expcUDir,expcADir,expcUInd,expcAInd,trcUDir,trcADir,trcUInd,trcAInd,trcUDirp,trcADirp,trcUIndp,trcAIndp,trnUDir,trnADir,trnUInd,trnAInd,detnUClin,detnAClin,desnUAll,desnAAll,vaccnUAll,vaccnAAll,detcUClin,detcAClin,descUIni,descAIni,descUDet,descADet,descUDir,descADir,descUInd,descAInd,descURing,descARing,vaccUIni,vaccAIni,vaccURing,vaccARing,zonnFoci,zoncFoci,appUInfectious) VALUES( %s, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i );\n",
               _scenario.scenarioId,  run_val, _day, prod->id,
                prod->d_data.tsdUSusc,  prod->d_data.tsdASusc,  prod->d_data.tsdULat,
                prod->d_data.tsdALat,   prod->d_data.tsdUSubc,  prod->d_data.tsdASubc,
                prod->d_data.tsdUClin,  prod->d_data.tsdAClin,  prod->d_data.tsdUNImm,
                prod->d_data.tsdANImm,  prod->d_data.tsdUVImm,  prod->d_data.tsdAVImm,
                prod->d_data.tsdUDest,  prod->d_data.tsdADest,  prod->data.tscUSusc,
                prod->data.tscASusc,    prod->data.tscULat,     prod->data.tscALat,
                prod->data.tscUSubc,    prod->data.tscASubc,    prod->data.tscUClin,
                prod->data.tscAClin,    prod->data.tscUNImm,    prod->data.tscANImm,
                prod->data.tscUVImm,    prod->data.tscAVImm,    prod->data.tscUDest,
                prod->data.tscADest,    prod->d_data.infnUAir,  prod->d_data.infnAAir,
                prod->d_data.infnUDir,  prod->d_data.infnADir,  prod->d_data.infnUInd,
                prod->d_data.infnAInd,  prod->data.infcUIni,    prod->data.infcAIni,
                prod->data.infcUAir,    prod->data.infcAAir,    prod->data.infcUDir,
                prod->data.infcADir,    prod->data.infcUInd,    prod->data.infcAInd,
                prod->data.expcUDir,    prod->data.expcADir,    prod->data.expcUInd,
                prod->data.expcAInd,    prod->data.trcUDir,     prod->data.trcADir,
                prod->data.trcUInd,     prod->data.trcAInd,     prod->data.trcUDirp,
                prod->data.trcADirp,    prod->data.trcUIndp,    prod->data.trcAIndp,
                prod->d_data.trnUDir,   prod->d_data.trnADir,   prod->d_data.trnUInd,
                prod->d_data.trnAInd,   prod->d_data.detnUClin, prod->d_data.detnAClin,
                prod->d_data.desnUAll,  prod->d_data.desnAAll,  prod->d_data.vaccUAll,
                prod->d_data.vaccAAll,  prod->data.detcUClin,   prod->data.detcAClin,
                prod->data.descUIni,    prod->data.descAIni,    prod->data.descUDet,
                prod->data.descADet,    prod->data.descUDir,    prod->data.descADir,
                prod->data.descUInd,    prod->data.descAInd,    prod->data.descURing,
                prod->data.descARing,   prod->data.vaccUIni,    prod->data.vaccAIni,
                prod->data.vaccURing,   prod->data.vaccARing,   prod->d_data.zonnFoci,
                prod->data.zoncFoci,    prod->d_data.appUInfectious
             );
  };
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_out_daily_by_production_type_SQL");
#endif
}


void write_dyn_herd_SQL( HRD_herd_list_t *_herds )
{
  int i;
  guint length;
  HRD_herd_t * _herd;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_dyn_herd_SQL");
#endif

  if ( _herds != NULL )
  {
    length = HRD_herd_list_length( _herds );
    for( i = 0; i < length; i++ )
    {
      _herd = HRD_herd_list_get( _herds, i );
      if ( _herd != NULL )
      { 
		g_print( "INSERT INTO dynHerd (herdID, scenarioID, productionTypeID, latitude, longitude, cumInfected, cumDetected, cumDestroyed, cumVaccinated) VALUES( %s, %s, %i, %g, %g, %i, %i, %i, %i);\n", 
				 _herd->official_id, _scenario.scenarioId, ((HRD_production_type_data_t*)(g_ptr_array_index (_herd->production_types, _herd->production_type )) )->id,
				 _herd->latitude, _herd->longitude, _herd->cum_infected, _herd->cum_detected,
				 _herd->cum_destroyed, _herd->cum_vaccinated
				);
	  };
	};
  };
		
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_dyn_herd_SQL");
#endif		
}

void update_dyn_herd_SQL( HRD_herd_list_t *_herds )
{
  int i;
  guint length;
  HRD_herd_t * _herd;

#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER update_dyn_herd_SQL");
#endif

  if ( _herds != NULL )
  {
    length = HRD_herd_list_length( _herds );
    for( i = 0; i < length; i++ )
    {
      _herd = HRD_herd_list_get( _herds, i );
      if ( _herd != NULL )
      { 
		g_print( "UPDATE dynHerd SET cumInfected=%i, cumDetected=%i, cumDestroyed=%i, cumVaccinated=%i WHERE herdID=%s;\n", 
				  _herd->cum_infected, _herd->cum_detected,
				  _herd->cum_destroyed, _herd->cum_vaccinated,
				  _herd->official_id
				);
	  };
	};
  };
		
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT update_dyn_herd_SQL");
#endif	  
}


void write_outDailyByZone_SQL( guint _day, guint _run, ZON_zone_list_t *_zones )
{
  guint i, list_len, iteration;
  ZON_zone_t *_zone;
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_outDailyByZone_SQL");
#endif	  

#if defined( USE_MPI ) && !CANCEL_MPI
  iteration = (me.rank * _scenario.nruns) + _run + 1;
#else
  iteration = _run + 1;
#endif
  
  if ( _zones != NULL )
  {
	  list_len =  ZON_zone_list_length( _zones );

	  for ( i = 0; i < list_len; i++ )
	  {
		if ( (_zone = ZON_zone_list_get( _zones, i )) != NULL )
		{
		  g_print( "INSERT INTO outDailyByZone ( jobID, iteration, day, zoneID, zoneArea ) VALUES ( %s, %i, %i, %i, %g );\n",
				 _scenario.scenarioId,  iteration, _day, _zone->level, _zone->area );
		};
	  };
  };
  
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_outDailyByZone_SQL");
#endif	  
}

void write_outIterationByHerd_SQL( guint _run, HRD_herd_list_t *_herds )
{
  guint i, length, iteration;
  HRD_herd_t * _herd;    
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- ENTER write_outIterationByHerd_SQL");
#endif	

#if defined( USE_MPI ) && !CANCEL_MPI
  iteration = (me.rank * _scenario.nruns) + _run + 1;
#else
  iteration = _run + 1;
#endif
  
  if ( _herds != NULL )
  {
    length = HRD_herd_list_length( _herds );
    for( i = 0; i < length; i++ )
    {
      _herd = HRD_herd_list_get( _herds, i );
      if ( _herd != NULL )
      { 
		g_print( "INSERT INTO outIterationByHerd ( jobID, iteration, herdID, lastStatusCode, lastStatusDay, lastApparentStateCode, lastApparentStateDay, firstInfectionDay ) VALUES( %s, %i, %s, '%c', %i, '%c', %i, %i );\n", 
				 _scenario.scenarioId,  iteration, _herd->official_id, HRD_STATE_CHAR[_herd->status], 
				 ((_iteration.outbreakEndDay > 0 )? (_iteration.outbreakEndDay - _herd->days_in_status):(_iteration.current_day - _herd->days_in_status)), 
				 HRD_APPARENT_STATE_CHAR[_herd->apparent_status], _herd->apparent_status_day, _herd->day_first_infected
				);
	  };
	};
  };
  
#if DEBUG
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "----- EXIT write_outIterationByHerd_SQL");
#endif	  
}


#endif
