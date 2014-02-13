unit xmlHerd;
(*
xmlHerd.pas
-----------
Begin: 2006/09/01
Last revision: $Date: 2009-06-05 19:52:39 $ $Author: areeves $
Version number: $Revision: 1.10 $
Project: APHI General Purpose Delphi Library, XML datafile functions
Website: http://www.naadsm.org/opensource/delphi
Author: Shaun Case <Shaun.Case@colostate.edu>
--------------------------------------------------
Copyright (C) 2006 - 2009 Animal Population Health Institute, Colorado State University

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*)

interface

  uses
    Loc,
    SysUtils
  ;

  type TxmlHerd =  class(TObject)
    protected
      _id: String;
      _prodType: String;
      _size: Integer;
      _location: TLoc;
      _status: String;
      _daysInInitialState: integer;
      _daysLeftInInitialState: integer;

      function getId():String;
      function getProdType():String;
      function getSize():Integer;
      function getLocation():TLoc;
      function getStatus():String;
      function getDaysInInitialState(): integer;
      function getDaysLeftInInitialState(): integer;

      procedure setId( Id: String );
      procedure setProdType( ProdType: String );
      procedure setSize( Size: Integer );
      procedure seTLocation( Location: TLoc );
      procedure setStatus( Status: String );
      procedure setDaysInInitialState( val: integer );
      procedure setDaysLeftInInitialState( val: integer );

    public
      constructor create(
            Id: string;
            ProdType: string;
            Size: integer;
            Location: TLoc;
            Status: string;
            daysInInitialState: integer;
            daysLeftInInitialState: integer
          );
        destructor destroy(); override;

        function WriteContents():String;

        property id: string read getID write setID;
        property prodType: string read getProdType write setProdType;
        property size: integer read getSize write setSize;
        property location: TLoc read getLocation write setLocation;
        property status: string read getStatus write setStatus;
        property daysInInitialState: integer read getDaysInInitialState write setDaysInInitialState;
        property daysLeftInInitialState: integer read getDaysLeftInInitialState write setDaysLeftInInitialState;
    end
  ;

implementation

  uses
    MyStrUtils
  ;

  constructor TxmlHerd.create(
        Id: string;
        ProdType: string;
        Size: integer;
        Location: TLoc;
        Status: string;
        daysInInitialState: integer;
        daysLeftInInitialState: integer
      );
    begin
      _id := Id;
      _prodType := ProdType;
      _size := Size;
      _location := Location;
      _status := Status;
      _daysInInitialState := daysInInitialState;
      _daysLeftInInitialState := daysLeftInInitialState;
    end
  ;

  destructor TxmlHerd.destroy();
    begin
      _location.Free();
      inherited destroy();
    end
  ;

  function TxmlHerd.GetId():String;
    begin
      result := _id;
    end
  ;

  function TxmlHerd.GetProdType():String;
    begin
      result := _prodType;
    end
  ;

  function TxmlHerd.GetSize():Integer;
    begin
      result := _size;
    end
  ;

  function TxmlHerd.GeTLocation():TLoc;
    begin
      result := _location;
    end
  ;

  function TxmlHerd.GetStatus():String;
    begin
      result := _status;
    end
  ;

  function TxmlHerd.GetDaysInInitialState(): integer;
    begin
      result := _daysInInitialState;
    end
  ;

  function TxmlHerd.GetDaysLeftInInitialState(): integer;
    begin
      result := _daysLeftInInitialState;
    end
  ;

  procedure TxmlHerd.SetId(Id:String);
    begin
      _id := Id;
    end
  ;

  procedure TxmlHerd.SetProdType( ProdType:String);
    begin
      _prodType := ProdType;
    end
  ;

  procedure TxmlHerd.SetSize(Size:Integer);
    begin
      _size := Size;
    end
  ;

  procedure TxmlHerd.SeTLocation(Location:TLoc);
    begin
      _location := Location;
    end
  ;

  procedure TxmlHerd.SetStatus(Status:String);
    begin
      _status := Status;
    end
  ;

  procedure TXmlHerd.setDaysInInitialState( val: integer );
    begin
      _daysInInitialState := val;
    end
  ;

  procedure TXmlHerd.setDaysLeftInInitialState( val: integer );
    begin
      _daysLeftInInitialState := val;
    end
  ;

  function TXmlHerd.WriteContents():String;
    begin
      result := 'xmlHerd:  id = ' + _id
        + '  production-type = ' + _prodType
        + '  size = ' + IntToStr(_size)
        + '  location = [lat(' + usFloatToStr(_location.GetLatitude()) + ') lon(' + usFloatToStr(_location.GetLongitude()) + ')]'
        + '  daysInInitialState = ' + intToStr( _daysInInitialState )
        + '  daysLeftInInitialState = ' + intToStr( _daysLeftInInitialState )
      ;
    end
  ;

end.
