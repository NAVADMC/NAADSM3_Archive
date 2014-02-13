unit ReadXMLInput;
(*
ReadXMLInput.pas
-----------------
Begin: 2006/09/28
Last revision: $Date: 2009-08-25 01:46:43 $ $Author: areeves $
Version number: $Revision: 1.39 $
Project: NAADSM
Website: http://www.naadsm.org
Author: Shaun Case <Shaun.Case@colostate.edu>
Author: Aaron Reeves <Aaron.Reeves@colostate.edu>
--------------------------------------------------
Copyright (C) 2006 - 2009 Animal Population Health Institute, Colorado State University

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*)

{$INCLUDE Defs.inc}

interface

  uses
    Windows,
    Messages,
    SysUtils,
    Variants,
    Classes,
    Graphics,
    Controls,
    Forms,
    Dialogs,
    StdCtrls,
    Math,

    MyDialogs,
    QStringMaps,
    FunctionPointers,

    Sdew,

    XMLReader,
    Loc,
    xmlHerd,
    TypInfo,
    ChartFunction,
    ProbDensityFunctions,
    Points,
    SMScenario,
    FunctionEnums,
    ProductionType,
    ProductionTypeList,
    FunctionDictionary,
    Herd,
    StatusEnums,
    NAADSMLibraryTypes,
    VaccinationParams,
    RelFunction,
    DetectionParams,
    DestructionParams,
    TracingParams,
    GlobalControlParams,
    ProductionTypePair,
    ProductionTypePairList,
    ContactSpreadParams,
    AirborneSpreadParams,
    RingVaccParams,
    Zone,
    ProdTypeZoneParams
  ;

  type Loc = record
    latitude: double;
    longitude: double;
  end;


  ////////////////////////////////////////////////////////////////////////////////
  //  Forward declarations of functions needed by the XML parsing routines.
  //  These are callback functions, which are called by the XMLReader object as it
  //  parses an XML file.
  //////////////////////////////////////////////////////////////////////

  function ProcessZoneModels              ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessDiseaseModels           ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessVaccineModels           ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessDetectionModels         ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessRingDestructionModel    ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessBasicDestruction        ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessTracebackDestruction    ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessGlobalControlParams     ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessContactSpreadModel      ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessAirborneModel           ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessRingVaccineModel        ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessBasicZoneFocusModel     ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  function ProcessTraceBackZoneFocusModel ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;


// FIX ME: This document block originally went with one of the PDF importers.
// Figure out where it should go now.
////////////////////////////////////////////////////////////////////////////////
//  These are the implementations of all of the callback functions seen in the
//  const declaration above of "constStatMethodTypes"
//
//  NOTE:  Anything returned by these callback fuctions is placed, by
//         the XMLReader object, into the array of records passed
//         to it prior to calling this callback.. As a result, the
//         XMLReader returns these updated arrays back to the calling
//         code for it's use.  Notice, in some of these functions, which
//         are nested, how they check this information and add it to their
//         own, creating objects to return.  See the XMLCallbacks record
//         definition in XMLReader for more information about the
//         structure of this data.
///////////////////////////////////////////////////////////////////////





  type TSMScenarioPtr = ^TSMScenario;


  type TXmlConvert = class( TObject )
    protected
      _smScenario:TSMScenarioPtr;
      _sfilename:String;
      _hfilename:String;

      _populateScenario: boolean;

      {* Function pointer to TFormProgress.setMessage() }
      _fnProgressMessage: TObjFnVoid1String;

      {* Function pointer to TFormProgress.setPrimary() }
      _fnProgressSet: TObjFnBool1Int;

      procedure convertScenario( err: pstring );
      procedure convertHerdList(  err: pstring );

    public
      constructor create( HerdsFilename: String; ScenarioFilename: String; smScenario:TSMScenarioPtr );
      destructor destroy(); override;
      
      procedure ConvertXmlToDatabase( err: pstring = nil );
      function ReadHerdXml( _hList:THerdList; err: pstring = nil ): boolean;

      property progressMessageFunction: TObjFnVoid1String write _fnProgressMessage;
      property progressValueFunction: TObjFnBool1Int write _fnProgressSet;
    end
  ;


  type TValueUnitPair = class( TObject )
    protected
      _value: string;
      _units: string;
    public
      constructor create( value: string; units: string );
      destructor destroy(); override;

      function getValue():String;
      function getUnits():String;
    end
  ;


implementation
  uses
    StrUtils,

    QLists,

    MyStrUtils,
    DebugWindow,
    I88n,

    ModelDatabase

    {$IFNDEF CONSOLEAPP}
    ,
    FormProgress
    {$ENDIF}
  ;

  const DBSHOWMSG: Boolean = false; // Set to true to enable debugging for this unit.

  var
    errorMessage: string;
    _internalDestructionPriorityList: TQStringLongIntMap;
    _destructionPriorityList: TQStringList;
    _internalVaccinationPriorityList: TQStringLongIntMap;

  type xmlProcessFunc = function( Element: Pointer; Sdew: TSdew; extData:Pointer ): TObject;

  type xmlProcessEquates = record
    name:String;
    xFunc: xmlProcessFunc;
  end;

  // Names in this array must exactly match those specified in the XML schema.
  const constNumXMLProcs = 14;
  const constXMLProcs:  array[ 0..constNumXMLProcs - 1 ] of xmlProcessEquates =
  (
    ( name: 'zone-model';                   xFunc: ProcessZoneModels           ),
    ( name: 'disease-model';                xFunc: ProcessDiseaseModels        ),
    ( name: 'vaccine-model';                xFunc: ProcessVaccineModels        ),
    ( name: 'detection-model';              xFunc: ProcessDetectionModels      ),
    ( name: 'basic-destruction-model';      xFunc: ProcessBasicDestruction     ),
    ( name: 'trace-back-destruction-model'; xFunc: ProcessTracebackDestruction ),
    ( name: 'ring-destruction-model';       xFunc: ProcessRingDestructionModel ),
    ( name: 'resources-and-implementation-of-controls-model'; xFunc: ProcessGlobalControlParams ),
    ( name: 'contact-spread-model';         xFunc: ProcessContactSpreadModel   ),
    ( name: 'airborne-spread-model';        xFunc: ProcessAirborneModel ),
    ( name: 'airborne-spread-exponential-model';        xFunc: ProcessAirborneModel ),
    ( name: 'ring-vaccination-model';       xFunc: ProcessRingVaccineModel ),
    ( name: 'basic-zone-focus-model';       xFunc: ProcessBasicZoneFocusModel ),
    ( name: 'trace-back-zone-focus-model';  xFunc: ProcessTraceBackZoneFocusModel )
  );


constructor TValueUnitPair.create( value: string; units: string );
  begin
    inherited create();
    
    _value := value;
    _units := units;
  end
;

destructor TValueUnitPair.destroy();
  begin
    inherited destroy();
  end
;


function TValueUnitPair.getValue():String;
  begin
    result := _value;
  end
;


function TValueUnitPair.getUnits():String;
  begin
    result := _units;
  end
;


///////////////////////////////////////////////////////////////////////////////
//  These two functions are the callback functions used when loading a herd
//  XML file.  This process is started by clicking button 1. (See below)
//////////////////////////////////////////////////////////////////////////
  function ProcessHerdLocation( Element: Pointer; Sdew: TSdew; extData:Pointer): TObject;
    var
      latitude: double;
      longitude: double;
      location: TLoc;
    begin
      latitude := usStrToFloat(Sdew.GetElementContents(Sdew.GetElementByName(Element, PChar('latitude'))));
      longitude := usStrToFloat(Sdew.GetElementContents(Sdew.GetElementByName(Element, PChar('longitude'))));
      location := TLoc.create( latitude, longitude );
      result := location;
    end
  ;


  function ProcessHerd( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
    var
      id: String;
      productionType: String;
      location: TLoc;
      size: Integer;
      status: String;
      daysInInitialState, daysLeftInInitialState: integer;

      tempXMLReader: TXMLReader;
      locationCallbacks : CallbackArray;
      Herd: TxmlHerd;
      e: pointer;
    begin
      SetLength(locationCallbacks, 1);
      locationCallbacks[0].Tag := 'location';
      locationCallbacks[0].Callback := ProcessHerdLocation;
      SetLength(locationCallbacks[0].ObjectList,0);

      tempXMLReader := TXMLReader.create( @locationCallbacks, Sdew, Element, @location );
      tempXMLReader.Run();
      tempXMLReader.free();

      id := Sdew.GetElementContents(Sdew.GetElementByName(Element, PChar('id')));
      productionType := Sdew.GetElementContents(Sdew.GetElementByName(Element, PChar('production-type')));
      size := StrToInt(Sdew.GetElementContents(Sdew.GetElementByName(Element, PChar('size'))));
      status := Sdew.GetElementContents(Sdew.GetElementByName(Element, PChar('status')));

      e := Sdew.GetElementByName( Element, PChar( 'days-in-status' ) );
      if( nil <> e ) then
        daysInInitialState := strToInt( Sdew.GetElementContents( e ) )
      else
        daysInInitialState := -1
      ;

      e := Sdew.GetElementByName( Element, PChar( 'days-left-in-status' ) );
      if( nil <> e ) then
        daysLeftInInitialState := strToInt( Sdew.GetElementContents( e ) )
      else
        daysLeftInInitialState := -1
      ;

      {*** Instantiate a TxmlHerd object here and load the above values to save to the database ***}
      if ( length(locationCallbacks[0].ObjectList) > 0 ) then
        begin
          Herd := TxmlHerd.create(
            id,
            productionType,
            size,
            TLoc(locationCallbacks[0].ObjectList[0]),
            status,
            daysInInitialState,
            daysLeftInInitialState
          );
        end
      else
        begin
          Herd := nil;
          Application.MessageBox('This herd file is not valid.  A herd is missing a location.', 'Error');
        end
      ;

      result := Herd;
    end
  ;



function ProcessVaccineDelay( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    delay:String;
    units:String;
    VDelay: TValueUnitPair;
  begin
    delay := Sdew.GetElementContents(Sdew.GetElementByName( Element, 'value' ));
    units := Sdew.GetElementContents( Sdew.GetElementByName( Sdew.GetElementByName(Element, 'units'), 'xdf:unit') );

    VDelay := TValueUnitPair.create( delay, units );
    result := VDelay;
  end
;




function ProcessBasicDestruction( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    _smScenarioPtr:TSMScenarioPtr;
    _prodTypeList:TProductionTypeList;
    _prodType:TProductionType;
    prodType:String;
    prodTypeID: integer;
    Priority:String;
    DParms:TDestructionParams;
  begin
    result := nil;

    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;

    prodType := Sdew.GetElementAttribute( Element, 'production-type' );
    prodTypeID := myStrToInt( Sdew.GetElementAttribute( Element, 'production-type-id' ), -1 );

    _prodType := _prodTypeList.findProdType( prodType );

    if( nil = _prodType ) then
      begin
        _prodType := TProductionType.create( prodTypeID, prodType, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodType );
      end
    ;

    if ( Assigned(_prodType.destructionParams) ) then
      DParms := _prodType.destructionParams
    else
      begin
        DParms := TDestructionParams.create( _smScenarioPtr.simInput, prodType );
        _prodType.destructionParams := DParms;
      end
    ;

    _smScenarioPtr^.simInput.controlParams.useDestructionGlobal := true;
    DParms.destroyDetectedUnits := true;
    Priority := Sdew.GetElementContents( Sdew.GetElementByName(Element, 'priority') );

    if ( _internalDestructionPriorityList.contains( 'basic' ) ) then
      begin
        if ( _internalDestructionPriorityList.Item['basic'] > StrToInt( Priority ) ) then
           _internalDestructionPriorityList.Item['basic'] := StrToInt( Priority );
       end
     else
       begin
           _internalDestructionPriorityList.Item['basic'] := StrToInt( Priority );
       end;

    DParms.destrPriority := StrToInt( Priority );   
    _smScenarioPtr.simInput.controlParams.ssDestrPriorities.Add( prodType + '+' + 'basic', StrToInt( Priority ));
    _destructionPriorityList.insert( StrToInt (Priority), prodType + '+' + 'basic' );
    _smScenarioPtr^.simInput.controlParams.useDestructionGlobal := true;
  end
;


function ProcessTracebackDestruction( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    _smScenarioPtr:TSMScenarioPtr;
    _prodTypeList:TProductionTypeList;
    _prodType:TProductionType;
    prodType:String;
    prodTypeID: integer;
    Priority:Integer;
    DParms:TDestructionParams;
    TrParms: TTracingParams;
    contactType:String;
    Period:Integer;
    Success:Double;
    QuarantineOnly:Boolean;
  begin
    result := nil; // until established otherwise

    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;

    prodType := Sdew.GetElementAttribute( Element, 'production-type' );
    prodTypeID := myStrToInt( Sdew.GetElementAttribute( Element, 'production-type-id' ), -1 );
    contactType := Sdew.GetElementAttribute( Element, 'contact-type' );

    _prodType := _prodTypeList.findProdType( prodType );

    if( nil = _prodType ) then
      begin
        _prodType := TProductionType.create( prodTypeID, prodType, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodType );
      end
    ;

    if ( Assigned(_prodType.destructionParams) ) then
      DParms := _prodType.destructionParams
    else
      begin
        DParms := TDestructionParams.create( _smScenarioPtr.simInput, prodType );
        _prodType.destructionParams := DParms;
      end
    ;

    if ( Assigned(_prodType.tracingParams) ) then
      TrParms := _prodType.tracingParams
    else
      begin
        TrParms := TTracingParams.create( _smScenarioPtr.simInput, prodType );
        _prodType.tracingParams := TrParms;
      end
    ;

    //??? Is this correct to set this for other types of destruction other than basic????
    _smScenarioPtr^.simInput.controlParams.useDestructionGlobal := true;

    Success := usStrToFloat( Sdew.GetElementContents(Sdew.GetElementByName( Element, 'trace-success' )) );
    //NOTE:  Ignoring the units in the period element because they are always set to "day" by the xml output routines for DestructionParams.
    Period := StrToInt( Sdew.GetElementContents( Sdew.GetElementByName( Sdew.GetElementByName( Element, 'trace-period'), 'value')));
    Priority := StrToInt( Sdew.GetElementContents(Sdew.GetElementByName(Element, 'priority')));
    QuarantineOnly := StrToBool( Sdew.GetElementContents( Sdew.GetElementByName( Element, 'quarantine-only') ) );


    //NOTE:  This is a placeholder in case the tracing element is found before the basic one, or if there
    //       is no basic one.....Notice that we will not set the "includeDestructionGlobal" until/unless we
    //       find an actual basic destruction element.
    _smScenarioPtr.simInput.controlParams.ssDestrPriorities.Add( prodType + '+' + 'basic', 0 );

    if ( contactType = 'direct' ) then
      begin
        TrParms.traceDirectForward := true;
        TrParms.directTracePeriod := Period;
        TrParms.directTraceSuccess := Success;
        if ( not QuarantineOnly ) then
          DParms.destroyDirectForwardTraces := true
        ;

        if ( _internalDestructionPriorityList.contains( 'direct' ) ) then
          begin
            if ( _internalDestructionPriorityList.Item['direct'] > Priority ) then
               _internalDestructionPriorityList.Item['direct'] := Priority;
           end
         else
           begin
               _internalDestructionPriorityList.Item['direct'] := Priority;
           end;

        _smScenarioPtr.simInput.controlParams.ssDestrPriorities.Add( prodType + '+' + 'direct', Priority );
        _destructionPriorityList.insert( Priority, prodType + '+' + 'direct' );
      end
    else
      begin
        TrParms.traceIndirectForward := true;
        TrParms.indirectTracePeriod := Period;
        TrParms.indirectTraceSuccess := Success;
        if ( not QuarantineOnly ) then
          DParms.destroyIndirectForwardTraces := true
        ;
        if ( _internalDestructionPriorityList.contains( 'indirect' ) ) then
          begin
            if ( _internalDestructionPriorityList.Item['indirect'] > Priority ) then
               _internalDestructionPriorityList.Item['indirect'] := Priority;
           end
         else
           begin
               _internalDestructionPriorityList.Item['indirect'] := Priority;
           end;
        _smScenarioPtr.simInput.controlParams.ssDestrPriorities.Add( prodType + '+' + 'indirect', Priority );
        _destructionPriorityList.insert( Priority, prodType + '+' + 'indirect' );
      end
    ;
    _smScenarioPtr^.simInput.controlParams.useDestructionGlobal := true;
  end
;


function ProcessRingDestructionModel( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    _smScenarioPtr:TSMScenarioPtr;
    _prodTypeList:TProductionTypeList;
    prodTypeSrc, prodTypeDest:String;
    _prodTypeSrc, _prodTypeDest:TProductionType;
    value:Double;
    DParms:TDestructionParams;
    ret_val:TObject;
    subElement:Pointer;
    priority: Integer;
    destrPriorityList: TQStringLongIntMap;
  begin
    ret_val := nil;

    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;
     destrPriorityList := _smScenarioPtr^.simInput.controlParams.ssDestrPriorities;

    prodTypeSrc :=  Sdew.GetElementAttribute( Element, 'from-production-type' );
    prodTypeDest := Sdew.GetElementAttribute( Element, 'to-production-type');

    //Get or create the production-type-pair here....
    _prodTypeSrc := _prodTypeList.findProdType( prodTypeSrc );
    _prodTypeDest := _prodTypeList.findProdType( prodTypeDest );

    if( nil = _prodTypeSrc ) then // The production type should be created
      begin
        _prodTypeSrc := TProductionType.create( -1, prodTypeSrc, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodTypeSrc );
      end
    ;

    if ( nil = _prodTypeDest ) then // The production type should be created
      begin
        _prodTypeDest := TProductionType.create( -1, prodTypeDest, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodTypeDest );
      end
    ;

    subElement := Sdew.GetElementByName( Element, 'radius' );
    if ( subElement <> nil ) then
      begin
        if ( Sdew.GetElementByName( subElement, 'value' ) <> nil ) then
          begin
            value := usStrToFloat( Sdew.GetElementContents(Sdew.GetElementByName( subElement, 'value' )) );
            DParms := _prodTypeSrc.destructionParams;

            if ( DParms = nil ) then
              begin
                DParms := TDestructionParams.create( _smScenarioPtr.simInput, prodTypeSrc );
                _prodTypeSrc.destructionParams := DParms;
              end
            ;

            DParms.isRingTrigger := true;
            DParms.ringRadius := value; //Note:  Units are not used, tho they are in the xml file...assume Km.

            DParms := _prodTypeDest.destructionParams;
            if ( DParms = nil ) then
              begin
                DParms := TDestructionParams.create( _smScenarioPtr.simInput, prodTypeDest );
                _prodTypeDest.destructionParams := DParms;
              end
            ;
            DParms.isRingTarget := true;

            //Priorities go with the "to type", i.e. destination....
            //NOTE:  These destruction ring priorities are NOT currently used
            //       in the system, but may be in the future.  Currently,
            //       The priorities are calculated based on other destruction
            //       priorities, since destruction priorities, overall, are actually
            //       a two-dimensional mapping of priorities.  Consequently, these
            //       values, as imported from XML, per se, are not directly used.             
            if ( Sdew.GetElementByName( Element, 'priority') <> nil ) then
              begin
                priority := StrToInt( Sdew.GetElementContents( Sdew.GetElementByName( Element, 'priority')) );

                if ( _internalDestructionPriorityList.contains( 'ring' ) ) then
                  begin
                    if ( _internalDestructionPriorityList.Item['ring'] > priority ) then
                      _internalDestructionPriorityList.Item['ring'] := priority;
                  end
                else
                  begin
                      _internalDestructionPriorityList.Item['ring'] := priority;
                  end;

                destrPriorityList.Add( prodTypeDest + '+' + 'ring', priority );
                _destructionPriorityList.insert( priority, prodTypeDest + '+' + 'ring' );
                //Set to bogus value so that the populateDatabase() function won't
                // throw an exception...because from-type doesn't need a priority here...but the populate function doesn't know that....
                destrPriorityList.Add( prodTypeSrc + '+' + 'ring', -1 );
              end
            ;

            _smScenarioPtr^.simInput.controlParams.useDestructionGlobal := true;
          end
        ;
      end
    ;

    result := ret_val;
  end
;


function ProcessRingVaccineModel( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    _smScenarioPtr:TSMScenarioPtr;
    _prodTypeList:TProductionTypeList;
    prodTypeSrc, prodTypeDest:String;
    _prodTypeSrc, _prodTypeDest:TProductionType;
    value:Double;
    ret_val:TObject;
    subElement:Pointer;
    priority:Integer;
    minTimeBetweenVacc:Integer;
    ringVaccSrc, ringVaccDest:TRingVaccParams;
  begin
    ret_val := nil;

    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;

    prodTypeSrc :=  Sdew.GetElementAttribute( Element, 'from-production-type' );
    prodTypeDest := Sdew.GetElementAttribute( Element, 'to-production-type');

    //Get or create the production-type-pair here....
    _prodTypeSrc := _prodTypeList.findProdType( prodTypeSrc );
    _prodTypeDest := _prodTypeList.findProdType( prodTypeDest );

    if( nil = _prodTypeSrc ) then // The production type should be created
      begin
        _prodTypeSrc := TProductionType.create( -1, prodTypeSrc, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodTypeSrc );
      end
    ;

    if( nil = _prodTypeDest ) then // The production type should be created
      begin
        _prodTypeDest := TProductionType.create( -1, prodTypeDest, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodTypeDest );
      end
    ;


    ringVaccSrc := _prodTypeSrc.ringVaccParams;
    if ( ringVaccSrc = nil ) then
      begin
        ringVaccSrc := TRingVaccParams.create( _smScenarioPtr^.simInput, prodTypeSrc );
        _prodTypeSrc.ringVaccParams := ringVaccSrc;
      end
    ;

    ringVaccDest := _prodTypeDest.ringVaccParams;
    if ( ringVaccDest = nil ) then
      begin
        ringVaccDest := TRingVaccParams.create( _smScenarioPtr^.simInput, prodTypeDest );
        _prodTypeDest.ringVaccParams := ringVaccDest;
      end
    ;

    ringVaccSrc.useRing := true;

    //Priorities go with the "to type", i.e. destination.
    if ( Sdew.GetElementByName( Element, 'priority') <> nil ) then
      begin
        priority := StrToInt( Sdew.GetElementContents( Sdew.GetElementByName( Element, 'priority')) );
        _prodTypeDest.ringVaccParams.vaccPriority := priority;

        if ( _smScenarioPtr^.simInput.controlParams.ssVaccPriorities.contains( prodTypeDest + '+' + 'ring' ) ) then
          begin
            if ( _smScenarioPtr^.simInput.controlParams.ssVaccPriorities.Item[prodTypeDest + '+' + 'ring'] > priority ) then
              _smScenarioPtr^.simInput.controlParams.ssVaccPriorities.Item[prodTypeDest + '+' + 'ring'] := priority;
          end
        else
          begin
              _smScenarioPtr^.simInput.controlParams.ssVaccPriorities.Item[prodTypeDest + '+' + 'ring'] := priority;
          end
        ;

//        _smScenarioPtr^.simInput.controlParams.ssVaccPriorities.Add( prodTypeDest + '+' + 'ring', priority );
        //Set to bogus value so that the populateDatabase() function won't
        // throw an exception...because from-type doesn't need a priority here...but the populate function doesn't know that....

        if  not ( _smScenarioPtr^.simInput.controlParams.ssVaccPriorities.contains( prodTypeSrc + '+' + 'ring' ) ) then
           _smScenarioPtr^.simInput.controlParams.ssVaccPriorities.Add( prodTypeSrc + '+' + 'ring', -1 )
        ;
      end
    ;

    subElement := Sdew.GetElementByName( Element, 'radius' );
    if ( subElement <> nil ) then
      begin
        if ( Sdew.GetElementByName( subElement, 'value' ) <> nil ) then
          begin
            value := usStrToFloat( Sdew.GetElementContents( Sdew.GetElementByName( subElement, 'value' ) ) );
            //  Units are present in the xml file, but not stored anywhere in the current schema.
            ringVaccSrc.ringRadius := value;

            subElement := Sdew.GetElementByName( Element, 'min-time-between-vaccinations');
            if ( subElement <> nil ) then
              begin
                minTimeBetweenVacc := StrToInt( Sdew.GetElementContents( Sdew.GetElementByName( subElement, 'value' ) ) );
                //  Units are present in the xml file, but not stored in anywhere in the current schema.
                ringVaccSrc.minTimeBetweenVacc := minTimeBetweenVacc;

                _smScenarioPtr^.simInput.controlParams.useVaccGlobal := true;
              end
            ;
          end
        ;
      end
    ;

    result := ret_val;
  end
;


function ProcessAirborneModel( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    ret_val:TObject;
    _smScenarioPtr:TSMScenarioPtr;
    _prodTypeList:TProductionTypeList;
    _ptpList:TProductionTypePairList;
    _ptp:TProductionTypePair;
    _prodTypeSrc, _prodTypeDest:TProductionType;
    prodTypeSrc, prodTypeDest:String;
    _airModel:TAirborneSpreadParams;
    subElement:Pointer;
    ssubElement:Pointer;
    unitElement:Pointer;
    statMethod:TChartFunction;
    probSpread:Double;
    Index:Integer;
    windDirStart, windDirEnd:Integer;
    maxSpread:Double;
    exponential:Boolean;
  begin
    ret_val := nil;
    exponential := false;
    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;
    _ptpList := _smScenarioPtr^.simInput.ptpList;

    if ( Sdew.GetElementName( Element ) = 'airborne-spread-exponential-model' ) then
      exponential := true;
      
    prodTypeSrc :=  Sdew.GetElementAttribute( Element, 'from-production-type' );
    prodTypeDest := Sdew.GetElementAttribute( Element, 'to-production-type');

    //Get or create the production-type-pair here....
    _prodTypeSrc := _prodTypeList.findProdType( prodTypeSrc );
    _prodTypeDest := _prodTypeList.findProdType( prodTypeDest );

    if( nil = _prodTypeSrc ) then // The production type should be created
      begin
        _prodTypeSrc := TProductionType.create( -1, prodTypeSrc, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodTypeSrc );
      end
    ;

    if( nil = _prodTypeDest ) then // The production type should be created
      begin
        _prodTypeDest := TProductionType.create( -1, prodTypeDest, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodTypeDest );
      end
    ;

    Index := _ptpList.pairPosition( _prodTypeSrc, _prodTypeDest );
    if ( Index <> -1 ) then
      begin
        _ptp := _ptpList.at( Index );
      end
    else
      begin
        _ptp := TProductionTypePair.create( _prodTypeSrc, _prodTypeDest, _smScenarioPtr^.simInput );
        _smScenarioPtr^.simInput.database.makeProductionTypePair( _prodTypeSrc.productionTypeID, _prodTypeDest.productionTypeID );
        _ptpList.Add( _ptp );
      end;
    _airModel := TAirborneSpreadParams.create(  _smScenarioPtr^.simInput.database,
                    -1, _smScenarioPtr^.simInput, _prodTypeSrc.productionTypeID,
                     _prodTypeDest.productionTypeID, false );

    subElement := Sdew.GetElementByName( Element, 'prob-spread-1km' );
    if ( subElement <> nil ) then
      begin
        probSpread := usStrToFloat( Sdew.GetElementContents( subElement ) );
        _airModel.probSpread1km := probSpread;
      end;

    subElement := Sdew.GetElementByName( Element, 'delay' );
    if ( subElement <> nil ) then
      begin
        statMethod := TChartFunction( createPdfFromXml( subElement, Sdew, nil ) );
        statMethod.name := 'Day airborne delay';
        statMethod.dbField := word( AIRDelay );
        _smScenarioPtr^.simInput.functionDictionary.checkAndInsert( statMethod );
        _airModel.pdfDelayName := statMethod.name;
      end;

    subElement := Sdew.GetElementByName( Element, 'wind-direction-start' );
    if ( subElement <> nil ) then
      begin
        ssubElement := Sdew.GetElementByName( subElement, 'value' );
        if ( ssubElement <> nil ) then
          begin
            windDirStart := StrToInt( Sdew.GetElementContents( ssubElement ) );
            _airModel.windStart := windDirStart;
          end;
      end;

    subElement := Sdew.GetElementByName( Element, 'wind-direction-end' );
    if ( subElement <> nil ) then
      begin
        ssubElement := Sdew.GetElementByName( subElement, 'value' );
        if ( ssubElement <> nil ) then
          begin
            windDirEnd := StrToInt( Sdew.GetElementContents( ssubElement ) );
            _airModel.windEnd := windDirEnd;
          end;
      end;

    if ( not exponential ) then
      begin
        subElement := Sdew.GetElementByName( Element, 'max-spread' );
        if ( subElement <> nil ) then
          begin
            ssubElement := Sdew.GetElementByName( subElement, 'value' );
            if ( ssubElement <> nil ) then
              begin
                maxSpread := usStrToFloat( Sdew.GetElementContents( ssubElement ) );
                _airModel.maxSpread := maxSpread;

                unitElement := Sdew.GetElementByName( subElement, 'units' );
                if ( unitElement <> nil ) then
                  begin
                    if ( Sdew.GetElementByName( unitElement, 'xdf:unit') <> nil ) then
                      _airModel.maxSpreadUnits := Sdew.GetElementContents( Sdew.GetElementByName( unitElement, 'xdf:unit'))
                    else
                      if (Sdew.GetElementByName( unitElement, 'xdf:unitless') <> nil ) then
                        _airModel.maxSpreadUnits := 'unitless';
                  end;
              end;
          end;
      end;

    _ptp.airborne := _airModel;
    _airModel.useAirborne := true;

    _smScenarioPtr^.simInput.includeAirborneSpreadGlobal := true;

    if ( exponential ) then
      _smScenarioPtr^.simInput.useAirborneExponentialDecay := true
    ;

    result := ret_val;
  end
;


function ProcessContactSpreadZoneModel( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    _smScenarioPtr:TSMScenarioPtr;
    _prodTypeList:TProductionTypeList;
    _prodTypeSrc:TProductionType;
    prodTypeSrc:String;
    contactType:String;
    ZoneName:String;
    subElement:Pointer;
    Chart:TRelFunction;
    _zoneList:TZoneList;
    _zoneParams: TProdTypeZoneParams;
    _zoneId:Integer;
  begin
    result := nil;

    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;
    _zoneList := _smScenarioPtr^.simInput.zoneList;

    prodTypeSrc :=  Sdew.GetElementAttribute( Element, 'from-production-type' );
    contactType :=  Sdew.GetElementAttribute( Element, 'contact-type' );
    ZoneName := Sdew.GetElementAttribute( Element, 'zone' );

    if ( nil = _zoneList.find( ZoneName ) ) then
      raise exception.Create( 'Zone does not exist in ProcessContactSpreadZoneModel()' )
    ;

    _prodTypeSrc := _prodTypeList.findProdType( prodTypeSrc );

    if ( _prodTypeSrc = nil ) then // Create a production type.
      begin
        _prodTypeSrc := TProductionType.create( -1, prodTypeSrc, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodTypeSrc );
      end
    ;

    if ( nil = _prodTypeSrc.zoneParams ) then
      raise exception.Create( 'prodTypeSrc.zoneParams is nil in ProcessContactSpreadZoneModel()' )
    ;

    //Create the movement control chart here
    subElement := Sdew.GetElementByName( Element, 'movement-control' );
    if ( subElement <> nil ) then
      begin
        Chart := TRelFunction( createRelFromXml( subElement, sdew, nil ) );
        Chart.convertToPercentages();

        Chart.name := contactType  + ' Zone Movement control effect - ' + _prodTypeSrc.productionTypeDescr + ' - ' + ZoneName;
        if ( contactType = 'direct' ) then
          Chart.dbField := word( ZONMovementDirect )
        else
          Chart.dbField := word( ZONMovementIndirect )
        ;
        _smScenarioPtr^.simInput.functionDictionary.checkAndInsert( chart );

        // Got a chart, now set the name in the zoneParams...
        _zoneParams := _prodTypeSrc.zoneParams;

        if ( _zoneParams.zonePtParamsList = nil ) then
          begin
            _zoneParams := TProdTypeZoneParams.create( _smScenarioPtr^.simInput.database, _prodTypeSrc.productionTypeID, _prodTypeSrc.productionTypeDescr, _zoneList );
            _zoneParams.sim := _prodTypeSrc.sim;
            _prodTypeSrc.zoneParams := _zoneParams;
          end
        ;

         _zoneId := _zoneList.find( ZoneName ).id;
         _zoneParams.setChart( TSMChart(Chart.dbField), Chart, _zoneId );
         _zoneParams.prodTypeDescr := prodTypeSrc;

         if ( Chart.dbField = word( ZONMovementDirect ) ) then
           _zoneParams.zonePtParamsList.paramsForZone(_zoneId).useDirectMovementControl := true
         else if ( Chart.dbField = word( ZONMovementIndirect ) ) then
           _zoneParams.zonePtParamsList.paramsForZone(_zoneId).useIndirectMovementControl := true
          ;
      end
    ;
  end
;


function ProcessContactSpreadModel( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    _smScenarioPtr:TSMScenarioPtr;
    _prodTypeList:TProductionTypeList;
    _ptpList:TProductionTypePairList;
    _ptp:TProductionTypePair;
    _prodTypeSrc, _prodTypeDest:TProductionType;
    prodTypeSrc, prodTypeDest:String;
    contactType:String;
    _contactType:TContactSpreadParams;
    subElement:Pointer;
    ssubElement:Pointer;
    statMethod:TChartFunction;
    probInfect:Double;
    Chart:TRelFunction;
    Index:Integer;
    contactRate:Double;
  begin
    result := nil;

    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;
    _ptpList := _smScenarioPtr^.simInput.ptpList;

    // Test to see if this is actually a zone contact model....
    if ( length( Sdew.GetElementAttribute( Element, 'zone')) > 0 ) then
      result := ProcessContactSpreadZoneModel( Element, Sdew, extData )
    else
      begin
        prodTypeSrc :=  Sdew.GetElementAttribute( Element, 'from-production-type' );
        prodTypeDest := Sdew.GetElementAttribute( Element, 'to-production-type');
        contactType :=  Sdew.GetElementAttribute( Element, 'contact-type' );

        //Get or create the production-type-pair here....
        _prodTypeSrc := _prodTypeList.findProdType( prodTypeSrc );
        _prodTypeDest := _prodTypeList.findProdType( prodTypeDest );

        if ( _prodTypeSrc = nil ) then // The production type should be created
          begin
            _prodTypeSrc := TProductionType.create( -1, prodTypeSrc, false, _smScenarioPtr^.simInput );
            _prodTypeList.append( _prodTypeSrc );
          end
        ;

        if ( _prodTypeDest = nil ) then // The production type should be created
          begin
            _prodTypeList.append( _prodTypeDest );
          end
        ;

        Index := _ptpList.pairPosition( _prodTypeSrc, _prodTypeDest );
        if ( Index <> -1 ) then
          _ptp := _ptpList.at( Index )
        else
          begin
            _ptp := TProductionTypePair.create( _prodTypeSrc, _prodTypeDest, _smScenarioPtr^.simInput );
            _ptpList.Add( _ptp );
          end
        ;

         _contactType := TContactSpreadParams.create();
         _contactType.sim := _smScenarioPtr^.simInput;
         _contactType.fromProdType := prodTypeSrc;
         _contactType.fromProdTypeID := _prodTypeSrc.productionTypeID;
         _contactType.toProdType := prodTypeDest;
         _contactType.toProdTypeID := _prodTypeDest.productionTypeID;

        if ( contactType = 'direct' ) then
          begin
            _ptp.includeDirect := true;
            _contactType.contactType := TContactType( CMDirect );
            _ptp.direct := _contactType;
          end
        else
          begin
            _ptp.includeIndirect := true;
            _contactType.contactType := TContactType( CMIndirect );
            _ptp.indirect := _contactType;
          end
        ;

        subElement := Sdew.GetElementByName( Element, 'distance' );
        if ( subElement <> nil ) then
          begin
            statMethod := TChartFunction( createPdfFromXml( subElement, Sdew, nil ) );
            statMethod.name := 'Dist ' + contactType + ' movement';

            if _contactType.contactType = CMDirect then
              statMethod.dbField := word( CMDistanceDirect )
            else
              statMethod.dbField := word( CMDistanceIndirect )
            ;
            _smScenarioPtr^.simInput.functionDictionary.checkAndInsert( statMethod );
            _contactType.pdfDistanceName := statMethod.name;
          end
        ;

        subElement := Sdew.GetElementByName( Element, 'delay' );
        if ( subElement <> nil ) then
          begin
            statMethod := TChartFunction( createPdfFromXml( subElement, Sdew, nil ) );
            statMethod.name := 'Delay ' + contactType + ' shipping';

            if ( _contactType.contactType = CMDirect ) then
              statMethod.dbField := word( CMDelayDirect )
            else
              statMethod.dbField := word( CMDelayIndirect )
            ;
            _smScenarioPtr^.simInput.functionDictionary.checkAndInsert( statMethod );
            _contactType.pdfDelayName := statMethod.name;
          end
        ;

        subElement := Sdew.GetElementByName( Element, 'prob-infect' );
        if ( subElement <> nil ) then
          begin
            probInfect := usStrToFloat( Sdew.GetElementContents( subElement ) );
            _contactType.probInfect := probInfect;
          end
        ;

        subElement := Sdew.GetElementByName( Element, 'movement-control' );
        if ( subElement <> nil ) then
          begin
            Chart := TRelFunction( createRelFromXml( subElement, sdew, nil ) );
            chart.convertToPercentages();

            Chart.name := 'Movement control effect ' + contactType;
            if ( _contactType.contactType = CMDirect ) then
              Chart.dbField := word( CMMovementControlDirect )
            else
              Chart.dbField := word( CMMovementControlIndirect )
            ;
            _smScenarioPtr^.simInput.functionDictionary.checkAndInsert( Chart );
            _contactType.relMovementControlName := Chart.name;
          end
        ;

        subElement := Sdew.GetElementByName( Element, 'latent-units-can-infect' );
        if ( subElement <> nil ) then
          _contactType.latentCanInfect :=  StrToBool( Sdew.GetElementContents( subElement ) )
        ;

        subElement := Sdew.GetElementByName( Element, 'subclinical-units-can-infect' );
        if ( subElement <> nil ) then
          _contactType.subClinicalCanInfect :=  StrToBool( Sdew.GetElementContents( subElement ) );
        ;

        subElement := Sdew.GetElementByName( Element, 'movement-rate' );
        if ( subElement <> nil ) then
          begin
            ssubElement := Sdew.GetElementByName( subElement, 'value' );
            if ( ssubElement <> nil ) then
              begin
                contactRate := usStrToFloat( Sdew.GetElementContents( ssubElement ) );
                _contactType.meanContactRate := contactRate;
                _contactType.useFixedContactRate := false;
              end
            ;
          end
        ;

        subElement := Sdew.GetElementByName( Element, 'fixed-movement-rate' );
        if ( subElement <> nil ) then
          begin
            ssubElement := Sdew.GetElementByName( subElement, 'value' );
            if ( ssubElement <> nil ) then
              begin
                contactRate := usStrToFloat( Sdew.GetElementContents( ssubElement ) );
                _contactType.fixedContactRate := contactRate;
                _contactType.useFixedContactRate := true;
              end
            ;
          end
        ;

        _smScenarioPtr^.simInput.includeContactSpreadGlobal := true;
      end
    ;
  end
;


function ProcessGlobalControlParams ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    _smScenarioPtr:TSMScenarioPtr;
    gcParms:TGlobalControlParams;
    Delay:Integer;
    PriorityOrder:String;
    Chart:TRelFunction;
    subElement:Pointer;
    ssubElement:Pointer;
  begin
    result := nil;

    _smScenarioPtr := TSMScenarioPtr(extData);
    gcParms := _smScenarioPtr^.simInput.controlParams;

    //  Get Destruction global settings
    subElement := Sdew.GetElementByName( Element, 'destruction-program-delay' );
    if ( nil <> subElement ) then
      begin
        // Ignoring Units here....always days.
        ssubElement := Sdew.GetElementByName( SubElement, 'value' );
        if ( nil <> ssubElement ) then
          begin
            Delay := StrToInt( Sdew.GetElementContents( ssubElement ) );
            gcParms.destrProgramDelay := Delay;
          end
      else
        errorMessage := errorMessage + 'Warning: No destruction-program-delay value found in this xml file' + endl
      ;
      end
    else
      errorMessage := errorMessage + 'Warning: No destruction-program-delay element found in this xml file' + endl
    ;

    subElement := Sdew.GetElementByName( Element, 'destruction-priority-order');
    if ( nil <> subElement ) then
      begin
        PriorityOrder := Sdew.GetElementContents( subElement );
        gcParms.destrPriorityOrder := PriorityOrder;
      end
    else
      errorMessage := errorMessage + 'Warning: No destruction-priority-order element found in this xml file' + endl
    ;


    subElement := Sdew.GetElementByName( Element, 'destruction-capacity');
    if ( nil <> subElement ) then
      begin
        Chart := TRelFunction( createRelFromXml( subElement, sdew, nil ) );
        Chart.name := 'Destruction capacity';
        Chart.xUnits :=  chartUnitTypeFromXml( 'days' );
        Chart.yUnits := chartUnitTypeFromXml( 'units per day' );
        Chart.dbField := word( DestrCapacityGlobal );
        _smScenarioPtr^.simInput.functionDictionary.checkAndInsert( Chart );
        gcParms.relDestrCapacityName := Chart.name;
      end
    else
      errorMessage := errorMessage + 'Warning: No destruction-capacity element found in this xml file' + endl
    ;

    // Get Vaccination global settings...
    subElement := Sdew.GetElementByName( Element, 'vaccination-program-delay' );
    if ( subElement <> nil ) then
      begin
        Delay := StrToInt( Sdew.GetElementContents( subElement ) );
        gcParms.vaccDetectedUnitsBeforeStart := Delay;
      end
    else
      errorMessage := errorMessage + 'Warning: No vaccination-program-delay element found in this xml file' + endl
    ;

    subElement := Sdew.GetElementByName( Element, 'vaccination-priority-order');
    if ( subElement <> nil ) then
      begin
        PriorityOrder := Sdew.GetElementContents( subElement );
        gcParms.vaccPriorityOrder := PriorityOrder;
      end
    else
      errorMessage := errorMessage + 'Warning: No vaccination-priority-order element found in this xml file' + endl
    ;

    subElement := Sdew.GetElementByName( Element, 'vaccination-capacity');
    if ( subElement <> nil ) then
      begin
        Chart := TRelFunction( createRelFromXml( subElement, sdew, nil ) );
        chart.name := 'Vaccination capacity';
        Chart.xUnits :=  chartUnitTypeFromXml( 'days' );
        Chart.yUnits := chartUnitTypeFromXml( 'units per day' );
        Chart.dbField := word( VaccCapacityGlobal );
        _smScenarioPtr^.simInput.functionDictionary.checkAndInsert( Chart );
        gcParms.relVaccCapacityName := Chart.name;
      end
    else
      errorMessage := errorMessage + 'Warning: No vaccination-capacity element found in this xml file' + endl
    ;
  end
;


function ProcessDetectionZoneModels( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    _smScenarioPtr: TSMScenarioPtr;
    _prodTypeList: TProductionTypeList;
    _prodType: TProductionType;
    prodType: String;
    prodTypeID: integer;
    zoneName: String;
    ret_val: TObject;
    _zoneList: TZoneList;
    _zoneParams: TProdTypeZoneParams;
    _zoneId: Integer;
    detectionMultiplier: double;
  begin
    ret_val := nil;
    _smScenarioPtr := TSMScenarioPtr(extData);
    _zoneList := _smScenarioPtr^.simInput.zoneList;
    _prodTypeList := _smScenarioPtr^.simInput.ptList;

    detectionMultiplier := 1.0; // This default value may be changed below.

    prodType := Sdew.GetElementAttribute( Element, 'production-type' );
    prodTypeID := myStrToInt( Sdew.GetElementAttribute( Element, 'production-type-id' ), -1 );
    zoneName := Sdew.GetElementAttribute( Element, 'zone' );

    if ( Sdew.GetElementByName( Element, 'zone-prob-multiplier') <> nil ) then
      detectionMultiplier := usStrToFloat( Sdew.GetElementContents( Sdew.GetElementByName( Element, 'zone-prob-multiplier') ) )
    ;

    _prodType := _prodTypeList.findProdType( prodType );

    if( nil = _prodType ) then
      begin
        _prodType := TProductionType.create( prodTypeID, prodType, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodType );
      end
    ;

    if( ( length(zoneName) > 0 ) AND (_zoneList <> nil ) )  then
      begin
        _zoneParams := _prodType.zoneParams;

        if ( _zoneParams <> nil ) then
          begin
            _zoneId := _zoneList.find( ZoneName ).id;

            //  This function also sets the useDetectionMultiplier boolean in the TZoneProdTypeComboParams object...so no need to set it also, here.
            _zoneParams.zonePtParamsList.value( _zoneId ).detectionMultiplier := detectionMultiplier;

            _zoneParams.prodTypeDescr := prodType;
          end
        ;
      end
    ;

    result := ret_val;
  end
;


function ProcessDetectionModels( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    DetectionSections:CallbackArray;
    tempXMLReader: TXMLReader;
    DParms:TDetectionParams;
    _smScenarioPtr:TSMScenarioPtr;
    _prodTypeList:TProductionTypeList;
    _prodType:TProductionType;
    _fnDictionary:TFunctionDictionary;
    statMethod:TRelFunction;
    prodType:String;
    prodTypeID: integer;
    ret_val:TObject;
  begin
    ret_val := nil;
    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;
    _fnDictionary := _smScenarioPtr^.simInput.functionDictionary;

    SetLength(DetectionSections, 2);
    DetectionSections[0].Tag := 'prob-report-vs-time-clinical';
    DetectionSections[0].Callback := createRelFromXml;
    SetLength(DetectionSections[0].ObjectList,0);

    DetectionSections[1].Tag := 'prob-report-vs-time-since-outbreak';
    DetectionSections[1].Callback := createRelFromXml;
    SetLength(DetectionSections[1].ObjectList,0);

    prodType := Sdew.GetElementAttribute( Element, 'production-type' );
    prodTypeID := myStrToInt( Sdew.GetElementAttribute( Element, 'production-type-id' ), -1 );

    if ( length(Sdew.GetElementAttribute( Element, 'zone' )) > 0 ) then
      ret_val := ProcessDetectionZoneModels( Element, Sdew, extData )
    else
      begin
        _prodType := _prodTypeList.findProdType( prodType );

        if( nil = _prodType ) then // The production type should be created
          begin
            _prodType := TProductionType.create( prodTypeID, prodType, false, _smScenarioPtr^.simInput );
            _prodTypeList.append( _prodType );
          end
        ;

        tempXMLReader := TXMLReader.create( @DetectionSections, Sdew, Element, extData );
        tempXMLReader.Run();
        tempXMLReader.free();

        if ( (length( DetectionSections[0].ObjectList ) > 0 ) and (length( DetectionSections[1].ObjectList ) > 0 ) ) then
          begin
            statMethod := TRelFunction( DetectionSections[0].ObjectList[0] );
            statMethod.convertToPercentages();
            statMethod.dbField := word( DetProbObsVsTimeClinical );
            statMethod.name := 'Probability of observing clinical signs versus days clinical' + ' ' + ansiLowerCase( prodType );
            _fnDictionary.checkAndInsert( statMethod );

            statMethod := TRelFunction( DetectionSections[1].ObjectList[0] );
            statMethod.convertToPercentages();
            statMethod.dbField := word( DetProbReportVsFirstDetection );
            statMethod.name := 'Probability of reporting versus days of outbreak' + ' ' + ansiLowerCase( prodType );
            _fnDictionary.checkAndInsert( statMethod );

            if ( not Assigned( _prodType.detectionParams ) ) then
              begin
                DParms := TDetectionParams.create( _smScenarioPtr^.simInput, prodType );
                _prodType.detectionParams := DParms;
              end
            else
              begin
                DParms := _prodType.detectionParams;
                DParms.prodTypeDescr := prodType;
              end
            ;

            DParms.relObsVsTimeClinicalName := TRelFunction(DetectionSections[0].ObjectList[0]).name;
            DParms.relReportVsFirstDetectionName := TRelFunction(DetectionSections[1].ObjectList[0]).name;

            _prodType.detectionParams.useDetection := true;
            _smScenarioPtr^.simInput.controlParams.useTracingGlobal := true;
            _smScenarioPtr^.simInput.controlParams.useDetectionGlobal := true;
            ret_val := DParms;
          end
        ;
      end
    ;

    result := ret_val;
  end
;


function ProcessVaccineModels( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    VaccineSections:CallbackArray;
    tempXMLReader: TXMLReader;
    _smScenarioPtr:TSMScenarioPtr;
    _prodTypeList:TProductionTypeList;
    _prodType:TProductionType;
    _fnDictionary:TFunctionDictionary;
    statMethod: TPdf;
    prodType:String;
    prodTypeID: integer;
    vaccParams:TVaccinationParams;
    vacDelay: TValueUnitPair;
    ret_val:TObject;

  begin
    ret_val := nil;
    vacDelay := nil;

    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;
    _fnDictionary := _smScenarioPtr^.simInput.functionDictionary;

    SetLength(VaccineSections, 2);
    VaccineSections[0].Tag := 'delay';
    VaccineSections[0].Callback := ProcessVaccineDelay;
    SetLength(VaccineSections[0].ObjectList,0);

    VaccineSections[1].Tag := 'immunity-period';
    VaccineSections[1].Callback := createPdfFromXml;
    SetLength(VaccineSections[1].ObjectList,0);

    prodType := Sdew.GetElementAttribute( Element, 'production-type' );
    prodTypeID := myStrToInt( Sdew.GetElementAttribute( Element, 'production-type-id' ), -1 );

    _prodType := _prodTypeList.findProdType( prodType );

    if( nil = _prodType ) then // The production type should be created
      begin
        _prodType := TProductionType.create( prodTypeID, prodType, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodType );
      end
    ;

    tempXMLReader := TXMLReader.create( @VaccineSections, Sdew, Element, extData );
    tempXMLReader.Run();
    tempXMLReader.free();

    if ( ( length(VaccineSections[0].ObjectList) > 0) and  (length( VaccineSections[1].ObjectList) > 0 ) ) then
      begin
        if ( ( Assigned( VaccineSections[0].ObjectList[0] ) ) and ( Assigned( VaccineSections[1].ObjectList[0] ) ) ) then
          begin
            vacDelay := TValueUnitPair( VaccineSections[0].ObjectList[0] );
            statMethod := TPdf( VaccineSections[1].ObjectList[0] );
            statMethod.name := 'Vaccine immune period' + ' - ' + prodType;
            statMethod.dbField := word( VacImmunePeriod );
            _fnDictionary.checkAndInsert( statMethod );
            vaccParams := TVaccinationParams.create( _smScenarioPtr^.simInput, prodType );
            vaccParams.pdfVaccImmuneName := statMethod.name;
            vaccParams.useVaccination := true;
            _smScenarioPtr^.simInput.controlParams.useVaccGlobal := true;
            vaccParams.daysToImmunity := StrToInt(vacDelay.getValue());
            ret_val := vaccParams;
            _prodType.vaccinationParams := vaccParams;
            _smScenarioPtr^.simInput.controlParams.useVaccGlobal := true;
          end
        ;
      end
    ;

    if( nil <> vacDelay ) then
      vacDelay.Free()
    ;

    result := ret_val;
  end
;


function ProcessBasicZoneFocusModel ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    _prodTypeList: TProductionTypeList;
    prodType: String;
    prodTypeID: integer;
    _prodType: TProductionType;
    _smScenarioPtr:TSMScenarioPtr;
  begin
    result := nil;

    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;

    prodType :=  Sdew.GetElementAttribute( Element, 'production-type' );
    prodTypeID := myStrToInt( Sdew.GetElementAttribute( Element, 'production-type-id' ), -1 );

    _prodType := _prodTypeList.findProdType( prodType );

    if( nil = _prodType ) then // The production type should be created
      begin
        _prodType := TProductionType.create( prodTypeID, prodType, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodType );
      end
    ;

    if ( _prodType.zoneParams <> nil ) then
      begin
        _prodType.zoneParams.detectionIsZoneTrigger := true;
        _prodType.zoneParams.prodTypeDescr := prodType;
      end
    ;
  end
;


function ProcessTraceBackZoneFocusModel ( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    _prodTypeList: TProductionTypeList;
    prodType: String;
    prodTypeID: integer;
    contactType: String;
    _prodType: TProductionType;
    _smScenarioPtr:TSMScenarioPtr;
  begin
    result := nil;

    _smScenarioPtr := TSMScenarioPtr(extData);
    _prodTypeList := _smScenarioPtr^.simInput.ptList;

    prodType :=  Sdew.GetElementAttribute( Element, 'production-type' );
    prodTypeID := myStrToInt( Sdew.GetElementAttribute( Element, 'production-type-id' ), -1 );

    contactType :=  Sdew.GetElementAttribute( Element, 'contact-type' );

    _prodType := _prodTypeList.findProdType( prodType );

    if( nil = _prodType ) then // The production type should be created
      begin
        _prodType := TProductionType.create( prodTypeID, prodType, false, _smScenarioPtr^.simInput );
        _prodTypeList.append( _prodType );
      end
    ;

    if ( _prodType.zoneParams <> nil ) then
      begin
        if ( contactType = 'direct' ) then
          _prodType.zoneParams.directTraceIsZoneTrigger := true
        else if ( contactType = 'indirect' ) then
            _prodType.zoneParams.indirectTraceIsZoneTrigger := true
        ;

        _prodType.zoneParams.prodTypeDescr := prodType;
      end
    ;
  end
;



function ProcessZoneModels( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    ZoneName: String;
//    Level: Integer;
    Radius: String;
    _smScenarioPtr:TSMScenarioPtr;
    _zoneList: TZoneList;
    _newZone: TZone;
    zoneID: integer;
  begin
    result := nil;
    
    _smScenarioPtr := TSMScenarioPtr(extData);
    _zoneList := _smScenarioPtr^.simInput.zoneList;

    zoneID := myStrToInt( Sdew.GetElementAttribute( Element, 'zone-id' ), -1 );

    Radius := '-1.0';
    ZoneName := '';

    //NOTE:  Since the units used in the zone-model remain constant, they
    //       are not read in here.  If this changes in the future, remember to
    //       add the code here to read them in.
    //       Additionally:
    //         The Level stored in the XML is not stored in the current database
    //       schema.  Consequently, it, also, is not read in here.  If this changes
    //       then be sure to remember to add the code to read the level here.
    if ( Sdew.GetElementByName( Element, 'name') <> nil ) then
      begin
        ZoneName := Sdew.GetElementContents( Sdew.GetElementByName( Element, 'name'));

        if (  Sdew.GetElementByName( Element, 'radius') <> nil ) then
          if ( Sdew.GetElementByName( Sdew.GetElementByName( Element, 'radius'), 'value') <> nil ) then
            begin
              Radius := Sdew.GetElementContents( Sdew.GetElementByName( Sdew.GetElementByName( Element, 'radius'), 'value'));

              if ( usStrToFloat( Radius ) > 0.0 ) then
                begin
                  if ( _zoneList.find( ZoneName ) = nil ) then
                    begin
                      _newZone := TZone.create( zoneID, ZoneName, usStrToFloat(Radius), _smScenarioPtr^.simInput );
                      _zoneList.append( _newZone );
                      _smScenarioPtr^.simInput.controlParams.useZonesGlobal := true;
                    end
                  ;
                end
              ;
            end
          ;
      end
    ;
  end
;


function ProcessDiseaseModels( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    diseaseCallbacks: CallbackArray;
    tempXMLReader: TXMLReader;

    latentPeriod: TPdf;
    infectiousSubClinicalPeriod: TPdf;
    infectiousClinicalPeriod: TPdf;
    immunityPeriod: TPdf;
    prevalenceChart: TRelFunction;

    prodTypeDescr: String;
    prodTypeID: integer;
    _smScenarioPtr: TSMScenarioPtr;
    _prodTypeList: TProductionTypeList;
    _prodType: TProductionType;
    _fnDictionary: TFunctionDictionary;
  begin
    latentPeriod := nil;
    infectiousSubClinicalPeriod := nil;
    infectiousClinicalPeriod := nil;
    immunityPeriod := nil;
    prevalenceChart := nil;

    _smScenarioPtr := TSMScenarioPtr( extData );
    _prodTypeList := _smScenarioPtr^.simInput.ptList;
    _fnDictionary := _smScenarioPtr^.simInput.functionDictionary;

    prodTypeDescr := Sdew.GetElementAttribute( Element, 'production-type' );
    prodTypeID := myStrToInt( Sdew.GetElementAttribute( Element, 'production-type-id' ), -1 );

    _prodType := _prodTypeList.findProdType( prodTypeDescr );

    if( nil = _prodType ) then
      begin
        _prodType := TProductionType.create( prodTypeID, prodTypeDescr, true, _smScenarioPtr^.simInput );
//F        _prodType.populateDatabase( _smScenarioPtr^.simInput.database, true );
        _prodTypeList.append( _prodType );
      end
    ;

    // Read the prevalence chart, if present
    //--------------------------------------
    (*
    subElement := Sdew.GetElementByName( Element, 'prevalence' );
    if ( subElement <> nil ) then
      begin
        prevalenceChart := TRelFunction( createRelFromXml( subElement, sdew, nil ) );
        prevalenceChart.convertToPercentages();

        prevalenceChart.name := prodTypeDescr + ' prevalence';
        prevalenceChart.dbField := word( DPrevalence );

        ChartID := _smScenarioPtr^.simInput.database.compareFunctions( smChartStr(TSMChart(prevalenceChart.dbField)), prevalenceChart, nFunctionsWithSameName );

        if ( -1 = ChartID ) then
          begin
            if( 0 < nFunctionsWithSameName ) then
              prevalenceChart.name := prevalenceChart.name + ' (' + intToStr( nFunctionsWithSameName + 1 ) + ')'
            ;
            _smScenarioPtr^.simInput.functionDictionary.insert( prevalenceChart.name, TFunctionDictionaryItem.create( prevalenceChart ) );
            prevalenceChart.populateDatabase( _smScenarioPtr^.simInput.database );
          end
        else
          begin
            prevalenceChart.free();
            prevalenceChart := TRelFunction( createFunctionFromDB( _smScenarioPtr^.simInput.database, ChartId ) );
          end
         ;

        _prodType.prevalenceName := prevalenceChart.name;
        _smScenarioPtr^.simInput.useWithinHerdPrevalence := true;
      end
    ;
    *)

    // Read the disease states
    //------------------------
    SetLength( diseaseCallbacks, 5 );

    diseaseCallbacks[0].Tag := 'latent-period';
    diseaseCallbacks[0].Callback := createPdfFromXml;
    SetLength( diseaseCallbacks[0].ObjectList, 0 );

    diseaseCallbacks[1].Tag := 'infectious-subclinical-period';
    diseaseCallbacks[1].Callback := createPdfFromXml;
    SetLength( diseaseCallbacks[1].ObjectList, 0 );

    diseaseCallbacks[2].Tag := 'infectious-clinical-period';
    diseaseCallbacks[2].Callback := createPdfFromXml;
    SetLength( diseaseCallbacks[2].ObjectList, 0 );

    diseaseCallbacks[3].Tag := 'immunity-period';
    diseaseCallbacks[3].Callback := createPdfFromXml;
    SetLength( diseaseCallbacks[3].ObjectList, 0 );

    diseaseCallbacks[4].Tag := 'prevalence';
    diseaseCallbacks[4].Callback := createRelFromXml;
    SetLength( diseaseCallbacks[4].ObjectList, 0 );

    tempXMLReader := TXMLReader.create( @diseaseCallbacks, Sdew, Element, nil );
    tempXMLReader.Run();
    tempXMLReader.free();

    if( 0 < length( diseaseCallbacks[0].ObjectList ) ) then
      latentPeriod := TPdf( diseaseCallbacks[0].ObjectList[0] )
    ;
    if( 0 < length( diseaseCallbacks[1].ObjectList ) ) then
      infectiousSubClinicalPeriod := TPdf( diseaseCallbacks[1].ObjectList[0] )
    ;
    if( 0 < length( diseaseCallbacks[2].ObjectList ) ) then
      infectiousClinicalPeriod := TPdf( diseaseCallbacks[2].ObjectList[0] )
    ;
    if( 0 < length( diseaseCallbacks[3].ObjectList ) ) then
      immunityPeriod := TPdf( diseaseCallbacks[3].ObjectList[0] )
    ;
    if( 0 < length( diseaseCallbacks[4].ObjectList ) ) then
      prevalenceChart := TRelFunction( diseaseCallbacks[4].ObjectList[0] )
    ;

    if( nil <> latentPeriod ) then
      begin
        latentPeriod.name := prodTypeDescr + ' latent period';
        _fnDictionary.checkAndInsert( latentPeriod );
        _prodType.pdfLatentName := latentPeriod.name;
        dbcout( latentPeriod.name, DBSHOWMSG );
        latentPeriod.dbField := word( DLatent );
      end
    ;

    if( nil <> infectiousSubClinicalPeriod ) then
      begin
        infectiousSubClinicalPeriod.name := prodTypeDescr + ' subclinical period';
        _fnDictionary.checkAndInsert( infectiousSubClinicalPeriod );
        _prodType.pdfSubclinicalName := infectiousSubClinicalPeriod.name;
        dbcout( infectiousSubClinicalPeriod.name, DBSHOWMSG );
        infectiousSubClinicalPeriod.dbField := word( DSubclinical );
      end
    ;

    if( nil <> infectiousClinicalPeriod ) then
      begin
        infectiousClinicalPeriod.name := prodTypeDescr + ' clinical period';
        _fnDictionary.checkAndInsert( infectiousClinicalPeriod );
        _prodType.pdfClinicalName := infectiousClinicalPeriod.name;
        dbcout( infectiousClinicalPeriod.name, DBSHOWMSG );
        infectiousClinicalPeriod.dbField := word( DClinical );
      end
    ;

    if( nil <> immunityPeriod ) then
      begin
        immunityPeriod.name := prodTypeDescr + ' immune period';
        _fnDictionary.checkAndInsert( immunityPeriod );
        _prodType.pdfImmuneName := immunityPeriod.name;
        dbcout( immunityPeriod.name, DBSHOWMSG );
        immunityPeriod.dbField := word( DImmune );
      end
    ;

    if( nil <> prevalenceChart ) then
      begin
        prevalenceChart.name := prodTypeDescr + ' prevalence';
        _fnDictionary.checkAndInsert( prevalenceChart );
        _prodType.relPrevalenceName := prevalenceChart.name;
        dbcout( prevalenceChart.name, DBSHOWMSG );
        prevalenceChart.dbField := word( DPrevalence );
        prevalenceChart.convertToPercentages();

        _smScenarioPtr^.simInput.useWithinHerdPrevalence := true;
      end
    ;

    result := nil; // This seems silly, but the function has to return something.
  end
;


function ProcessModels( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    ModelCallbacks : CallbackArray;
    tempXMLReader: TXMLReader;
    I:Integer;
  begin
    SetLength( ModelCallbacks, constNumXMLProcs );

    for i := 0 to constNumXMLProcs - 1 do
      begin
        ModelCallbacks[I].Tag := constXMLProcs[I].name;
        ModelCallbacks[I].Callback := constXMLProcs[I].xFunc;
        SetLength(ModelCallbacks[I].ObjectList,0);
      end
    ;

    tempXMLReader := TXMLReader.create( @ModelCallbacks, Sdew, Element, extData );
    tempXMLReader.Run();
    tempXMLReader.free();

    result := nil;
  end
;


function ProcessParamsFile( Element: Pointer; Sdew: TSdew; extData:Pointer ):TObject;
  var
    elementName:String;
    _smScenarioPtr:TSMScenarioPtr;
  begin
    _smScenarioPtr := TSMScenarioPtr(extData);

    elementName := Sdew.GetElementName( Element );

    if ( elementName = 'description' ) then
      _smScenarioPtr.simInput.scenarioDescr := decodeXml( Sdew.GetElementContents( Element ) )
    else if ( elementName = 'num-days' ) then
      _smScenarioPtr.simInput.simDays := StrToInt( Sdew.GetElementContents( Element ) )
    else if ( elementName = 'num-runs' ) then
      _smScenarioPtr.simInput.simIterations := StrToInt( Sdew.GetElementContents( Element ) )
    ;

    result := nil;
  end
;


constructor TXMLConvert.create( HerdsFilename:String; ScenarioFilename:String; smScenario:TSMScenarioPtr );
  begin
    inherited create();
    errorMessage := '';

    _populateScenario := false;

    _smScenario := smScenario;
    _sfilename := ScenarioFilename;
    _hfilename := HerdsFilename;
  end
;

destructor TXMLConvert.destroy();
  begin
    inherited destroy();
  end
;





  ///////////////////////////////////////////////////////////////////////////////
 //  This function reads a Herd XML file and fills an hlist for use elsewhere
/////////////////////////////////////////////////////////////////////////
function TXMLConvert.ReadHerdXml( _hList:THerdList; err: pstring = nil ): boolean;
  var
    XMLReader: TXMLReader;
    MyCallbacks: CallbackArray;
    I: Integer;
    J: Integer;
    _h: THerd;
    _xmlHerd: TxmlHerd;
    pt: TProductionType;
    ret_val: boolean;
  begin
    ret_val := true; // until shown otherwise.  FIX ME: This value is never used!

    SetLength( MyCallbacks, 1);
    MyCallbacks[0].Tag := 'herd';
    MyCallbacks[0].Callback := ProcessHerd;
    SetLength(MyCallbacks[0].ObjectList, 0);

    if( nil <> @_fnProgressMessage ) then
      _fnProgressMessage( tr( 'Reading units...' ) )
    ;
    if( nil <> @_fnProgressSet ) then
      _fnProgressSet( 0 )
    ;

    XMLReader := TXMLReader.create(@MyCallbacks, _hfilename, _smScenario, nil (*frmProgress*) );
    XMLReader.Run();
    XMLReader.free();

    if( nil <> @_fnProgressSet ) then
      _fnProgressSet( 0 )
    ;
    if( nil <> @_fnProgressMessage ) then
      _fnProgressMessage( tr( 'Building units...' ) )
    ;

    _hList := _smScenario^.herdList;

    if ( nil <> _hList ) then
      begin
        for I := 0 to length( MyCallbacks ) - 1 do
          begin
            if( nil <> @_fnProgressSet ) then
              _fnProgressSet( 0 )
            ;

            for J := 0 to length( MyCallbacks[I].ObjectList ) - 1 do
              begin
                if( nil <> @_fnProgressSet ) then
                  _fnProgressSet( (((J+1) * 100) div length( MyCallbacks[I].ObjectList )) )
                ;

                _xmlHerd := TxmlHerd(MyCallbacks[I].ObjectList[J]);

                if( nil <> @_fnProgressMessage ) then
                  _fnProgressMessage( ansiReplaceStr( tr( 'Building unit with ID xyz...' ), 'xyz', _xmlHerd.id ) )
                ;

                _h := THerd.create( _hList );

                if( _populateScenario ) then
                  begin
                    pt := _smScenario^.simInput.findProdType( _xmlHerd.prodType );

                    if( nil = pt ) then
                      begin
                        pt := TProductionType.create( -1, _xmlHerd.prodType, false, _smScenario^.simInput );
                        pt.populateDatabase( _smScenario^.simInput.database, MDBAAuto );
                        _smScenario^.simInput.ptList.Append( pt );
                      end
                    ;

                    _h.setProdType( pt );
                  end
                else
                  _h.prodTypeName := _xmlHerd.prodType
                ;

                _h.setLatLon( _xmlHerd.location.GetLatitude(), _xmlHerd.location.GetLongitude() );
                _h.initialSize := _xmlHerd.size;
                _h.initialStatus := naadsmDiseaseStateFromString( _xmlHerd.status );
                _h.daysInInitialState := _xmlHerd.daysInInitialState;
                _h.daysLeftInInitialState := _xmlHerd.daysLeftInInitialState;

                _hList.append( _h );

                if( _populateScenario ) then
                  _h.simParams := _smScenario.simInput
                ;
              end
            ;

            if( _populateScenario ) then
              begin
                dbcout( 'Populating database with herd list', true );
                _hList.populateDatabase( _smScenario^.simInput.database );
                dbcout( 'Recounting Units', true );
                _smScenario^.simInput.ptList.recountUnits( _hList );
              end
            ;
          end
        ;
      end
    ;

    if( nil <> err ) then
      err^ := err^ + errorMessage
    ;

    result := ret_val;
  end
;


procedure TXmlConvert.convertScenario( err: pstring );
  var
    XMLReader: TXMLReader;
    MyCallbacks: CallbackArray;
    J: Integer;
    pt: TProductionType;
    destrReasonStr: String;
    index:Integer;
    listlength: Integer;
    tempStr:String;
    maxPriority: Int64;
    prodTypeDest:TProductionType;
    prodTypeList:TProductionTypeList;
    priorityList:TQStringList;
    reasons:Array of String;
    destPriorities: Array of Array of Integer;
    totalProgressSteps, currentProgressStep: integer;
  begin
    totalProgressSteps := 8;
    currentProgressStep := 0;

    if( nil <> @_fnProgressMessage ) then
      _fnProgressMessage( tr( 'Reading scenario parameters...' ) )
    ;
    if( nil <> @_fnProgressSet ) then
      begin
        _fnProgressSet( trunc( currentProgressStep / totalProgressSteps )  );
        inc( currentProgressStep );
      end
    ;

    _internalDestructionPriorityList := TQStringLongIntMap.create();
    _destructionPriorityList := TQStringList.create();
    _internalVaccinationPriorityList := TQStringLongIntMap.create();

    //'basic,direct-forward,indirect-forward,ring,direct-back,indirect-back'  As a default order...
    _internalDestructionPriorityList.insert('basic', 1000);
    _internalDestructionPriorityList.insert('direct-forward', 1001);
    _internalDestructionPriorityList.insert('indirect-forward', 1002);
    _internalDestructionPriorityList.insert('ring', 1003);
    _internalDestructionPriorityList.insert('direct-back', 1004);
    _internalDestructionPriorityList.insert('indirect-back', 1005);

    _internalVaccinationPriorityList.insert('ring', 1003);

    SetLength( MyCallbacks, 2);
    MyCallbacks[0].Tag := '*';
    MyCallbacks[0].Callback := ProcessParamsFile;
    SetLength(MyCallbacks[0].ObjectList, 0);

    MyCallbacks[1].Tag := 'models';
    MyCallbacks[1].Callback := ProcessModels;
    SetLength(MyCallbacks[1].ObjectList, 0);

    XMLReader := TXMLReader.create( @MyCallbacks, _sfilename, _smScenario, nil (*frmProgress*) );
    XMLReader.Run();
    XMLReader.free();

    if( nil <> @_fnProgressMessage ) then
      _fnProgressMessage( tr( 'Processing scenario parameters...' ) )
    ;

    // Handle ring-vaccine priority list here...check list for -1 priorities and convert them
    // to end of list....  (check list for highest priority value, increment this value for each -1 priority
    // and assign it to that prodtype in the list, replacing its -1 value...)
    //-------------------------------------------------------------------------------------------------------
    if( nil <> @_fnProgressSet ) then
      begin
        _fnProgressSet( trunc( currentProgressStep / totalProgressSteps )  );
        inc( currentProgressStep );
      end
    ;

    priorityList := TQStringList.create();
    prodTypeList := _smScenario^.simInput.ptList;

    for index := 0 to prodTypeList.Count - 1 do
      begin
        pt := prodTypeList.at( index );
        if ( 0 < pt.ringVaccParams.vaccPriority )  then
          priorityList.insert( pt.ringVaccParams.vaccPriority, pt.productionTypeDescr )
        ;
      end
    ;

    maxPriority := 0;

    for index:= 0 to priorityList.count - 1 do
      begin
        tempStr := priorityList.at( index );

        if ( length( tempStr ) > 0 ) then
          begin
            prodTypeDest := prodTypeList.findProdType( tempStr );
            if ( assigned( prodTypeDest ) ) then
              begin
                inc( maxPriority );
                prodTypeDest.ringVaccParams.vaccPriority := maxPriority;
                prodTypeDest.vaccinationParams.useVaccination := true;
                prodTypeDest.ringVaccParams.updated := true;
                _smScenario^.simInput.controlParams.ssVaccPriorities.item[ tempStr + '+' + 'ring' ] := maxPriority;
              end
            ;
          end
        ;
      end
    ;

    for index := 0 to prodTypeList.Count - 1 do
      begin
        if ( prodTypeList.at( index ).ringVaccParams.vaccPriority <=0 ) then
          begin
            // inc( maxPriority );
            prodTypeDest := prodTypeList.at( index );
            prodTypeDest.ringVaccParams.vaccPriority := -1; //maxPriority;
            prodTypeDest.vaccinationParams.useVaccination := false;
            prodTypeDest.ringVaccParams.updated := true;
            _smScenario^.simInput.controlParams.ssVaccPriorities.item[ prodTypeDest.productionTypeDescr + '+' + 'ring' ] := -1;
          end
        ;
      end
    ;

    //  Rebuild the Reason's list for destruction in order.
    //-----------------------------------------------------
    if( nil <> @_fnProgressSet ) then
      begin
        _fnProgressSet( trunc( currentProgressStep / totalProgressSteps )  );
        inc( currentProgressStep );
      end
    ;

    listlength := 4;
    setLength( reasons, 4 );

    while( 0 < listlength ) do
      begin
        tempStr := '';
        for index := 0 to listlength - 1 do
          begin
            if ( index = 0 ) then
              tempStr := _internalDestructionPriorityList.keyAtIndex( index )
            else
              begin
                if ( _internalDestructionPriorityList.itemAtIndex( index ) < _internalDestructionPriorityList.value( tempStr ) ) then
                  tempStr := _internalDestructionPriorityList.keyAtIndex( index )
                ;
              end
            ;
          end
        ;

        reasons[ 4 - listLength ] := tempStr;
        destrReasonStr := destrReasonStr + tempStr;

        if ( listlength > 1) then
          destrReasonStr := destrReasonStr + ','
        ;

        _internalDestructionPriorityList.remove( tempStr );
        listlength := listlength - 1;
      end
    ;

    _smScenario^.simInput.controlParams.destrReasonOrder := destrReasonStr;


    // Put the reasons in this array in order and fill in missing dictionary values.
    //------------------------------------------------------------------------------
    if( nil <> @_fnProgressSet ) then
      begin
        _fnProgressSet( trunc( currentProgressStep / totalProgressSteps )  );
        inc( currentProgressStep );
      end
    ;

    setLength( destPriorities, 4, prodTypeList.Count );
    listLength := 4;


    // Verify destruction entries in dictionary for all production types and for all reasons.
    //---------------------------------------------------------------------------------------
    if( nil <> @_fnProgressSet ) then
      begin
        _fnProgressSet( trunc( currentProgressStep / totalProgressSteps )  );
        inc( currentProgressStep );
      end
    ;

    for index := 0 to prodTypeList.Count - 1 do
      begin
        for j := 0 to  listLength - 1 do
          begin
            if( not _smScenario^.simInput.controlParams.ssDestrPriorities.contains( prodTypeList.at( index ).productionTypeDescr + '+' + reasons[j]  )  ) then
              begin
                _smScenario^.simInput.controlParams.ssDestrPriorities.Add( prodTypeList.at( index ).productionTypeDescr + '+' + reasons[j], -1 );
                destPriorities[j, index] := -1;
              end
            else
              begin
                destPriorities[j, index] := _smScenario^.simInput.controlParams.ssDestrPriorities.Item[ prodTypeList.at( index ).productionTypeDescr + '+' + reasons[j]];
              end
            ;
          end
        ;
      end
    ;

    //  destPriorities matrix is probably not needed, use this code to re-align priority numbers.
    //  The above matrix code is not saved in CVS anywhere, so leave it here for reference.  If removing
    //  it, make sure to leave the portion of it that sets missing entriy priorities to (-1).
{*
    listLength := 0;
    for index := 0 to _destructionPriorityList.count - 1 do
      begin
        inc( listLength );
        _smScenario^.simInput.controlParams.ssDestrPriorities.Add( _destructionPriorityList.at( index ), listLength );
      end
    ;
*}
    //  Turns out the destPriorities matrix IS needed....use the code below
    //  from now on to be sure to re-align the priorities.
    //----------------------------------------------------
    if( nil <> @_fnProgressSet ) then
      begin
        _fnProgressSet( trunc( currentProgressStep / totalProgressSteps )  );
        inc( currentProgressStep );
      end
    ;

    for index := 0 to prodTypeList.Count - 1 do
      begin
        for j := 0 to  listLength - 1 do
          _smScenario^.simInput.controlParams.ssDestrPriorities.Add( prodTypeList.at( index ).productionTypeDescr + '+' + reasons[j], destPriorities[j, index] )
        ;
      end
    ;

    _internalDestructionPriorityList.Free();
    _destructionPriorityList.Free();
    _internalVaccinationPriorityList.Free();

    // This should now be the only time it's necessary to populate the database during an XML import!
    //-----------------------------------------------------------------------------------------------
    if( nil <> @_fnProgressMessage ) then
      _fnProgressMessage( tr( 'Populating database...' ) )
    ;
    if( nil <> @_fnProgressSet ) then
      _fnProgressSet( trunc( currentProgressStep / totalProgressSteps )  )
    ;

    _smScenario^.simInput.populateDatabase( MDBAForceInsert ); // this will force inserts rather than updates

    if( nil <> @_fnProgressSet ) then
      _fnProgressSet( 100  )
    ;
  end
;



procedure TXMLConvert.convertHerdList( err: pstring );
  begin
    _populateScenario := true;
    readHerdXml( _smScenario^.herdList, err );
    _populateScenario := false;
  end
;


  ///////////////////////////////////////////////////////////////////////////////
 //  This function begins the processing of the disease-model XML file
/////////////////////////////////////////////////////////////////////////
procedure TXMLConvert.ConvertXMLToDatabase( err: pstring = nil );
  {$IFNDEF CONSOLEAPP}
  var
    frmProgress:TFormProgress;
    {$ENDIF}
  begin
    if( ( 0 = length( _sfilename ) ) and ( 0 = length( _hfilename ) ) ) then
      exit
    ;

    {$IFNDEF CONSOLEAPP}
      frmProgress := TFormProgress.create( application.MainForm, PRSingleBar, false );
      _fnProgressMessage := frmProgress.setMessage;
      _fnProgressSet := frmProgress.setPrimary;
      frmProgress.show();
    {$ELSE}
      _fnProgressMessage := nil;
      _fnProgressSet := nil;
    {$ENDIF}

    try
      if ( 0 < length( _sfilename ) ) then
        convertScenario( err )
      ;

      if( 0 < length( _hfilename ) ) then
        begin
          convertHerdList( err );
        end
      ;
    finally
      {$IFNDEF CONSOLEAPP}
        frmProgress.Release();
      {$ENDIF}
    end;
  end
;

end.
