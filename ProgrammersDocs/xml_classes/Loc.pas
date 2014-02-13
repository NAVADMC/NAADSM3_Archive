unit Loc;
(*
Loc.pas
-------
Begin: 2006/09/01
Last revision: $Date: 2007-02-06 04:39:14 $ $Author: areeves $
Version number: $Revision: 1.6 $
Project: APHI General Purpose Delphi Library, XML datafile functions
Website: http://www.naadsm.org/opensource/delphi
Author: Shaun Case <Shaun.Case@colostate.edu>
--------------------------------------------------
Copyright (C) 2006 - 2007 Animal Population Health Institute, Colorado State University

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*)

interface
  
type TLoc = class(TObject)
  public
    constructor create();overload;
    constructor create( Latitude: double; Longitude: double );overload;

    function GetLatitude():double;
    function GetLongitude():double;

    procedure SetLatitude( Latitude: double );
    procedure SetLongitude( Longitude: double );

  protected
  _latitude: double;
  _longitude: double;
end;

implementation

  constructor TLoc.create();
    begin
      _latitude := 0.0;
      _longitude := 0.0;
    end;

  constructor TLoc.create(Latitude: double; Longitude: double );
    begin
      _latitude := Latitude;
      _longitude := Longitude;
    end;

  function TLoc.GetLatitude():double;
    begin
      result := _latitude;
    end;

  function TLoc.GetLongitude():double;
    begin
      result := _longitude;
    end;

  procedure TLoc.SetLatitude( Latitude: double );
    begin
      _latitude := Latitude;
    end;

  procedure TLoc.SetLongitude( Longitude: double );
    begin
      _longitude := Longitude;
    end;

end.
