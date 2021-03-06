--    This SQL DDL script was generated by Microsoft Visual Studio (Release Date: LOCAL BUILD).

--    Driver Used : Microsoft Visual Studio - Microsoft Access Driver.
--    Document    : C:\Documents and Settings\apreeves\My Documents\NAADSM\DatabaseSchemas\DBSchema3_1_1.vsd.
--    Time Created: January 12, 2007 2:44 PM.
--    Operation   : From Visio Update Database Wizard.
--    Connected data source : naadsm-3_1_x-temp
--    Connected server      : ACCESS
--    Connected database    : C:\Documents and Settings\apreeves\My Documents\NAADSM\DatabaseSchemas\naadsm3_1_x-temp.mdb


-- Create new table `dynBlob`.
-- `dynBlob` : Table of dynBlob
-- 	`dynBlobID` : dynBlobID identifies dynBlob
-- 	`zonePerimeters` : zonePerimeters is of dynBlob  
create table `dynBlob` ( 
	`dynBlobID` CHAR(10),
	`zonePerimeters` LONGBINARY, constraint `dynBlob_PK` primary key (`dynBlobID`) ); 


-- Update existing table `inZoneProductionTypePair`.
create table `inZoneProductionTypePair_IM0` ( 
	`zoneID` INTEGER,
	`productionTypeID` INTEGER,
	`zoneDirectMovementRelID` INTEGER,
	`zoneIndirectMovementRelID` INTEGER,
	`zoneDetectionMultiplier` DOUBLE); 

insert into `inZoneProductionTypePair_IM0` (`zoneID`, `productionTypeID`, `zoneDirectMovementRelID`, `zoneIndirectMovementRelID`, `zoneDetectionMultiplier`) 
    select `inZoneProductionTypePair`.`zoneID`, 
           `inZoneProductionTypePair`.`productionTypeID`, 
           `inZoneProductionTypePair`.`zoneDirectMovementRelID`, 
           `inZoneProductionTypePair`.`zoneIndirectMovementRelID`, 
           `inZoneProductionTypePair`.`zoneDetectionMultiplier`  
    from `inZoneProductionTypePair`;


alter table `inZoneProductionTypePair`
	drop constraint `inChart_inZoneProductionTypePair_FK1`;


alter table `inZoneProductionTypePair`
	drop constraint `inChart_inZoneProductionTypePair_FK2`;


alter table `inZoneProductionTypePair`
	drop constraint `inProductionType_inZoneProductionTypePair_FK1`;


alter table `inZoneProductionTypePair`
	drop constraint `inSurveillanceZone_inZoneProductionTypePair_FK1`;


drop table `inZoneProductionTypePair`; 


-- `inZoneProductionTypePair` : Table of inZoneProductionTypePair
-- 	`zoneID` : zoneID identifies inZoneProductionTypePair
-- 	`productionTypeID` : productionTypeID partly identifies inZoneProductionTypePair
-- 	`useDirectMovementControl` : useDirectMovementControl is of inZoneProductionTypePair
-- 	`zoneDirectMovementRelID` : zoneDirectMovementRelID is of inZoneProductionTypePair
-- 	`useIndirectMovementControl` : useIndirectMovementControl is of inZoneProductionTypePair
-- 	`zoneIndirectMovementRelID` : zoneIndirectMovementRelID is of inZoneProductionTypePair
-- 	`useDetectionMultiplier` : useDetectionMultiplier is of inZoneProductionTypePair
-- 	`zoneDetectionMultiplier` : zoneDetectionMultiplier is of inZoneProductionTypePair  
create table `inZoneProductionTypePair` ( 
	`zoneID` INTEGER,
	`productionTypeID` INTEGER,
	`useDirectMovementControl` BIT,
	`zoneDirectMovementRelID` INTEGER,
	`useIndirectMovementControl` BIT,
	`zoneIndirectMovementRelID` INTEGER,
	`useDetectionMultiplier` BIT,
	`zoneDetectionMultiplier` DOUBLE); 

-- Note: Access does not support the mandatory (not null) constraint in SQL DDL.
-- The mandatory constraints for table '`inZoneProductionTypePair`' will be created in the database using DAO
-- only if there is a connection to the Access MDB file during DDL generation.
-- Not nullable columns for table 'inZoneProductionTypePair' :
-- 	`zoneID`
-- 	`productionTypeID`


insert into `inZoneProductionTypePair` (`zoneID`, `productionTypeID`, `zoneDirectMovementRelID`, `zoneIndirectMovementRelID`, `zoneDetectionMultiplier`) 
    select `inZoneProductionTypePair_IM0`.`zoneID`, 
           `inZoneProductionTypePair_IM0`.`productionTypeID`, 
           `inZoneProductionTypePair_IM0`.`zoneDirectMovementRelID`, 
           `inZoneProductionTypePair_IM0`.`zoneIndirectMovementRelID`, 
           `inZoneProductionTypePair_IM0`.`zoneDetectionMultiplier`  
    from `inZoneProductionTypePair_IM0`;


drop table `inZoneProductionTypePair_IM0`; 


alter table `inZoneProductionTypePair`
	add constraint `inZoneProductionTypePair_PK` primary key (`zoneID`, `productionTypeID`) ; 


-- Restore foreign key constraints for existing tables
alter table `inZoneProductionTypePair`
	add constraint `inChart_inZoneProductionTypePair_FK1` foreign key (
		`zoneDirectMovementRelID`)
	 references `inChart` (
		`chartID`); 

alter table `inZoneProductionTypePair`
	add constraint `inChart_inZoneProductionTypePair_FK2` foreign key (
		`zoneIndirectMovementRelID`)
	 references `inChart` (
		`chartID`); 

alter table `inZoneProductionTypePair`
	add constraint `inProductionType_inZoneProductionTypePair_FK1` foreign key (
		`productionTypeID`)
	 references `inProductionType` (
		`productionTypeID`); 

alter table `inZoneProductionTypePair`
	add constraint `inSurveillanceZone_inZoneProductionTypePair_FK1` foreign key (
		`zoneID`)
	 references `inSurveillanceZone` (
		`zoneID`); 


-- This is the end of the Microsoft Visual Studio generated SQL DDL script.
