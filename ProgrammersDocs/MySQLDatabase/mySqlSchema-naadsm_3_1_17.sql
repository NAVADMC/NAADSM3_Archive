-- MySQL dump 10.10
--
-- Host: risk07    Database: naadsm_3_1_17
-- ------------------------------------------------------
-- Server version	5.0.27-community-nt

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `DBSchemaVersion`
--

DROP TABLE IF EXISTS `DBSchemaVersion`;
CREATE TABLE `DBSchemaVersion` (
  `VersionNumber` varchar(255) default NULL,
  `VersionApplication` char(10) default NULL,
  `VersionDate` datetime default NULL,
  `VersionInfoURL` varchar(255) default NULL,
  `VersionID` int(11) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `inProductionType`
--

DROP TABLE IF EXISTS `inProductionType`;
CREATE TABLE `inProductionType` (
  `productionTypeID` int(11) NOT NULL default '0',
  `descr` varchar(255) default NULL,
  `scenarioID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`productionTypeID`,`scenarioID`),
  KEY `scenarioID` (`scenarioID`),
  CONSTRAINT `inproductiontype_ibfk_1` FOREIGN KEY (`scenarioID`) REFERENCES `scenario` (`scenarioID`),
  CONSTRAINT `inproductiontype_ibfk_2` FOREIGN KEY (`scenarioID`) REFERENCES `scenario` (`scenarioID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `inZone`
--

DROP TABLE IF EXISTS `inZone`;
CREATE TABLE `inZone` (
  `zoneID` int(11) NOT NULL default '0',
  `descr` varchar(255) default NULL,
  `scenarioID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`zoneID`,`scenarioID`),
  KEY `scenarioID` (`scenarioID`),
  CONSTRAINT `inzone_ibfk_1` FOREIGN KEY (`scenarioID`) REFERENCES `scenario` (`scenarioID`),
  CONSTRAINT `inzone_ibfk_2` FOREIGN KEY (`scenarioID`) REFERENCES `scenario` (`scenarioID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `job`
--

DROP TABLE IF EXISTS `job`;
CREATE TABLE `job` (
  `jobID` int(11) NOT NULL default '0',
  `scenarioID` int(11) default NULL,
  PRIMARY KEY  (`jobID`),
  KEY `scenarioID` (`scenarioID`),
  CONSTRAINT `job_ibfk_1` FOREIGN KEY (`scenarioID`) REFERENCES `scenario` (`scenarioID`),
  CONSTRAINT `job_ibfk_2` FOREIGN KEY (`scenarioID`) REFERENCES `scenario` (`scenarioID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `message`
--

DROP TABLE IF EXISTS `message`;
CREATE TABLE `message` (
  `messageID` int(11) NOT NULL default '0',
  `jobID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`messageID`,`jobID`),
  KEY `jobID` (`jobID`),
  CONSTRAINT `message_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `message_ibfk_2` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outCustIteration`
--

DROP TABLE IF EXISTS `outCustIteration`;
CREATE TABLE `outCustIteration` (
  `jobID` int(11) default NULL,
  `iteration` int(11) default NULL,
  KEY `jobID` (`jobID`),
  KEY `iteration` (`iteration`),
  CONSTRAINT `outcustiteration_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outcustiteration_ibfk_2` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outcustiteration_ibfk_3` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outcustiteration_ibfk_4` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outCustIterationByProductionType`
--

DROP TABLE IF EXISTS `outCustIterationByProductionType`;
CREATE TABLE `outCustIterationByProductionType` (
  `jobID` int(11) default NULL,
  `productionTypeID` int(11) default NULL,
  `iteration` int(11) default NULL,
  `vaccA10DaysPostDetection` int(11) default NULL,
  `tscLatPostDetection` int(11) default NULL,
  `vaccA2DaysPostDetection` int(11) default NULL,
  `vaccU2DaysPostDetection` int(11) default NULL,
  `vaccU7DaysPostDetection` int(11) default NULL,
  `vaccA7DaysPostDetection` int(11) default NULL,
  `unitsInfectedAtFirstDetection` int(11) default NULL,
  KEY `jobID` (`jobID`),
  KEY `productionTypeID` (`productionTypeID`),
  KEY `iteration` (`iteration`),
  CONSTRAINT `outcustiterationbyproductiontype_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outcustiterationbyproductiontype_ibfk_2` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`),
  CONSTRAINT `outcustiterationbyproductiontype_ibfk_3` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outcustiterationbyproductiontype_ibfk_4` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outcustiterationbyproductiontype_ibfk_5` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`),
  CONSTRAINT `outcustiterationbyproductiontype_ibfk_6` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outCustIterationByZone`
--

DROP TABLE IF EXISTS `outCustIterationByZone`;
CREATE TABLE `outCustIterationByZone` (
  `jobID` int(11) default NULL,
  `zoneID` int(11) default NULL,
  `iteration` int(11) default NULL,
  KEY `jobID` (`jobID`),
  KEY `zoneID` (`zoneID`),
  KEY `iteration` (`iteration`),
  CONSTRAINT `outcustiterationbyzone_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outcustiterationbyzone_ibfk_2` FOREIGN KEY (`zoneID`) REFERENCES `inzone` (`zoneID`),
  CONSTRAINT `outcustiterationbyzone_ibfk_3` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outcustiterationbyzone_ibfk_4` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outcustiterationbyzone_ibfk_5` FOREIGN KEY (`zoneID`) REFERENCES `inzone` (`zoneID`),
  CONSTRAINT `outcustiterationbyzone_ibfk_6` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outCustIterationByZoneAndProductionType`
--

DROP TABLE IF EXISTS `outCustIterationByZoneAndProductionType`;
CREATE TABLE `outCustIterationByZoneAndProductionType` (
  `jobID` int(11) default NULL,
  `zoneID` int(11) default NULL,
  `productionTypeID` int(11) default NULL,
  `iteration` int(11) default NULL,
  KEY `jobID` (`jobID`),
  KEY `zoneID` (`zoneID`),
  KEY `productionTypeID` (`productionTypeID`),
  KEY `iteration` (`iteration`),
  CONSTRAINT `outcustiterationbyzoneandproductiontype_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outcustiterationbyzoneandproductiontype_ibfk_2` FOREIGN KEY (`zoneID`) REFERENCES `inzone` (`zoneID`),
  CONSTRAINT `outcustiterationbyzoneandproductiontype_ibfk_3` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`),
  CONSTRAINT `outcustiterationbyzoneandproductiontype_ibfk_4` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outcustiterationbyzoneandproductiontype_ibfk_5` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outcustiterationbyzoneandproductiontype_ibfk_6` FOREIGN KEY (`zoneID`) REFERENCES `inzone` (`zoneID`),
  CONSTRAINT `outcustiterationbyzoneandproductiontype_ibfk_7` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`),
  CONSTRAINT `outcustiterationbyzoneandproductiontype_ibfk_8` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outDailyByProductionType`
--

DROP TABLE IF EXISTS `outDailyByProductionType`;
CREATE TABLE `outDailyByProductionType` (
  `jobID` int(11) NOT NULL default '0',
  `iteration` int(11) NOT NULL default '0',
  `day` int(11) NOT NULL default '0',
  `productionTypeID` int(11) NOT NULL default '0',
  `tsdUSusc` int(11) default NULL,
  `tsdASusc` int(11) default NULL,
  `tsdULat` int(11) default NULL,
  `tsdALat` int(11) default NULL,
  `tsdUSubc` int(11) default NULL,
  `tsdASubc` int(11) default NULL,
  `tsdUClin` int(11) default NULL,
  `tsdAClin` int(11) default NULL,
  `tsdUNImm` int(11) default NULL,
  `tsdANImm` int(11) default NULL,
  `tsdUVImm` int(11) default NULL,
  `tsdAVImm` int(11) default NULL,
  `tsdUDest` int(11) default NULL,
  `tsdADest` int(11) default NULL,
  `tscUSusc` int(11) default NULL,
  `tscASusc` int(11) default NULL,
  `tscULat` int(11) default NULL,
  `tscALat` int(11) default NULL,
  `tscUSubc` int(11) default NULL,
  `tscASubc` int(11) default NULL,
  `tscUClin` int(11) default NULL,
  `tscAClin` int(11) default NULL,
  `tscUNImm` int(11) default NULL,
  `tscANImm` int(11) default NULL,
  `tscUVImm` int(11) default NULL,
  `tscAVImm` int(11) default NULL,
  `tscUDest` int(11) default NULL,
  `tscADest` int(11) default NULL,
  `infnUAir` int(11) default NULL,
  `infnAAir` int(11) default NULL,
  `infnUDir` int(11) default NULL,
  `infnADir` int(11) default NULL,
  `infnUInd` int(11) default NULL,
  `infnAInd` int(11) default NULL,
  `infcUIni` int(11) default NULL,
  `infcAIni` int(11) default NULL,
  `infcUAir` int(11) default NULL,
  `infcAAir` int(11) default NULL,
  `infcUDir` int(11) default NULL,
  `infcADir` int(11) default NULL,
  `infcUInd` int(11) default NULL,
  `infcAInd` int(11) default NULL,
  `expcUDir` int(11) default NULL,
  `expcADir` int(11) default NULL,
  `expcUInd` int(11) default NULL,
  `expcAInd` int(11) default NULL,
  `trcUDir` int(11) default NULL,
  `trcADir` int(11) default NULL,
  `trcUInd` int(11) default NULL,
  `trcAInd` int(11) default NULL,
  `trcUDirp` int(11) default NULL,
  `trcADirp` int(11) default NULL,
  `trcUIndp` int(11) default NULL,
  `trcAIndp` int(11) default NULL,
  `detnUClin` int(11) default NULL,
  `detnAClin` int(11) default NULL,
  `desnUAll` int(11) default NULL,
  `desnAAll` int(11) default NULL,
  `vaccnUAll` int(11) default NULL,
  `vaccnAAll` int(11) default NULL,
  `detcUClin` int(11) default NULL,
  `detcAClin` int(11) default NULL,
  `descUIni` int(11) default NULL,
  `descAIni` int(11) default NULL,
  `descUDet` int(11) default NULL,
  `descADet` int(11) default NULL,
  `descUDir` int(11) default NULL,
  `descADir` int(11) default NULL,
  `descUInd` int(11) default NULL,
  `descAInd` int(11) default NULL,
  `descURing` int(11) default NULL,
  `descARing` int(11) default NULL,
  `vaccUIni` int(11) default NULL,
  `vaccAIni` int(11) default NULL,
  `vaccURing` int(11) default NULL,
  `vaccARing` int(11) default NULL,
  `zonnFoci` int(11) default NULL,
  `zoncFoci` int(11) default NULL,
  `appUInfectious` int(11) default NULL,
  PRIMARY KEY  (`iteration`,`jobID`,`day`,`productionTypeID`),
  KEY `jobID` (`jobID`),
  KEY `productionTypeID` (`productionTypeID`),
  CONSTRAINT `outdailybyproductiontype_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outdailybyproductiontype_ibfk_2` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outdailybyproductiontype_ibfk_3` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outDailyByZone`
--

DROP TABLE IF EXISTS `outDailyByZone`;
CREATE TABLE `outDailyByZone` (
  `jobID` int(11) NOT NULL default '0',
  `iteration` int(11) NOT NULL default '0',
  `day` int(11) NOT NULL default '0',
  `zoneID` int(11) NOT NULL default '0',
  `area` double default NULL,
  PRIMARY KEY  (`iteration`,`jobID`,`day`,`zoneID`),
  KEY `jobID` (`jobID`),
  KEY `zoneID` (`zoneID`),
  CONSTRAINT `outdailybyzone_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outdailybyzone_ibfk_2` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outdailybyzone_ibfk_3` FOREIGN KEY (`zoneID`) REFERENCES `inzone` (`zoneID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outDailyByZoneAndProductionType`
--

DROP TABLE IF EXISTS `outDailyByZoneAndProductionType`;
CREATE TABLE `outDailyByZoneAndProductionType` (
  `jobID` int(11) NOT NULL default '0',
  `iteration` int(11) NOT NULL default '0',
  `day` int(11) NOT NULL default '0',
  `zoneID` int(11) NOT NULL default '0',
  `productionTypeID` int(11) NOT NULL default '0',
  `unitDaysInZone` int(11) default NULL,
  `animalDaysInZone` int(11) default NULL,
  `unitsInZone` int(11) default NULL,
  `animalsInZone` int(11) default NULL,
  PRIMARY KEY  (`iteration`,`jobID`,`day`,`zoneID`,`productionTypeID`),
  KEY `jobID` (`jobID`),
  KEY `productionTypeID` (`productionTypeID`),
  KEY `zoneID` (`zoneID`),
  CONSTRAINT `outdailybyzoneandproductiontype_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outdailybyzoneandproductiontype_ibfk_2` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outdailybyzoneandproductiontype_ibfk_3` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`),
  CONSTRAINT `outdailybyzoneandproductiontype_ibfk_4` FOREIGN KEY (`zoneID`) REFERENCES `inzone` (`zoneID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outEpidemicCurves`
--

DROP TABLE IF EXISTS `outEpidemicCurves`;
CREATE TABLE `outEpidemicCurves` (
  `jobID` int(11) NOT NULL default '0',
  `iteration` int(11) NOT NULL default '0',
  `day` int(11) NOT NULL default '0',
  `productionTypeID` int(11) NOT NULL default '0',
  `infectedUnits` int(11) default NULL,
  `infectedAnimals` int(11) default NULL,
  `detectedUnits` int(11) default NULL,
  `detectedAnimals` int(11) default NULL,
  `infectiousUnits` int(11) default NULL,
  `apparentInfectiousUnits` int(11) default NULL,
  PRIMARY KEY  (`iteration`,`jobID`,`day`,`productionTypeID`),
  KEY `jobID` (`jobID`),
  KEY `productionTypeID` (`productionTypeID`),
  CONSTRAINT `outepidemiccurves_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outepidemiccurves_ibfk_2` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outepidemiccurves_ibfk_3` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outGeneral`
--

DROP TABLE IF EXISTS `outGeneral`;
CREATE TABLE `outGeneral` (
  `jobID` int(11) default NULL,
  `outGeneralID` char(10) default NULL,
  `simulationStartTime` datetime default NULL,
  `simulationEndTime` datetime default NULL,
  `completedIterations` int(11) default NULL,
  `version` varchar(50) default NULL,
  `lastUpdated` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  KEY `jobID` (`jobID`),
  CONSTRAINT `outgeneral_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outgeneral_ibfk_2` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outIteration`
--

DROP TABLE IF EXISTS `outIteration`;
CREATE TABLE `outIteration` (
  `jobID` int(11) NOT NULL default '0',
  `iteration` int(11) NOT NULL default '0',
  `diseaseEnded` tinyint(1) default NULL,
  `diseaseEndDay` int(11) default NULL,
  `outbreakEnded` tinyint(1) default NULL,
  `outbreakEndDay` int(11) default NULL,
  `zoneFociCreated` tinyint(1) default NULL,
  `lastUpdated` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`iteration`,`jobID`),
  KEY `jobID` (`jobID`),
  CONSTRAINT `outiteration_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outiteration_ibfk_2` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outIterationByProductionType`
--

DROP TABLE IF EXISTS `outIterationByProductionType`;
CREATE TABLE `outIterationByProductionType` (
  `jobID` int(11) NOT NULL default '0',
  `iteration` int(11) NOT NULL default '0',
  `productionTypeID` int(11) NOT NULL default '0',
  `tscUSusc` int(11) default NULL,
  `tscASusc` int(11) default NULL,
  `tscULat` int(11) default NULL,
  `tscALat` int(11) default NULL,
  `tscUSubc` int(11) default NULL,
  `tscASubc` int(11) default NULL,
  `tscUClin` int(11) default NULL,
  `tscAClin` int(11) default NULL,
  `tscUNImm` int(11) default NULL,
  `tscANImm` int(11) default NULL,
  `tscUVImm` int(11) default NULL,
  `tscAVImm` int(11) default NULL,
  `tscUDest` int(11) default NULL,
  `tscADest` int(11) default NULL,
  `infcUIni` int(11) default NULL,
  `infcAIni` int(11) default NULL,
  `infcUAir` int(11) default NULL,
  `infcAAir` int(11) default NULL,
  `infcUDir` int(11) default NULL,
  `infcADir` int(11) default NULL,
  `infcUInd` int(11) default NULL,
  `infcAInd` int(11) default NULL,
  `expcUDir` int(11) default NULL,
  `expcADir` int(11) default NULL,
  `expcUInd` int(11) default NULL,
  `expcAInd` int(11) default NULL,
  `trcUDir` int(11) default NULL,
  `trcADir` int(11) default NULL,
  `trcUInd` int(11) default NULL,
  `trcAInd` int(11) default NULL,
  `trcUDirp` int(11) default NULL,
  `trcADirp` int(11) default NULL,
  `trcUIndp` int(11) default NULL,
  `trcAIndp` int(11) default NULL,
  `detcUClin` int(11) default NULL,
  `detcAClin` int(11) default NULL,
  `descUIni` int(11) default NULL,
  `descAIni` int(11) default NULL,
  `descUDet` int(11) default NULL,
  `descADet` int(11) default NULL,
  `descUDir` int(11) default NULL,
  `descADir` int(11) default NULL,
  `descUInd` int(11) default NULL,
  `descAInd` int(11) default NULL,
  `descURing` int(11) default NULL,
  `descARing` int(11) default NULL,
  `vaccUIni` int(11) default NULL,
  `vaccAIni` int(11) default NULL,
  `vaccURing` int(11) default NULL,
  `vaccARing` int(11) default NULL,
  `zoncFoci` int(11) default NULL,
  `firstDetection` int(11) default NULL,
  `firstDestruction` int(11) default NULL,
  `firstVaccination` int(11) default NULL,
  PRIMARY KEY  (`iteration`,`jobID`,`productionTypeID`),
  KEY `jobID` (`jobID`),
  KEY `productionTypeID` (`productionTypeID`),
  CONSTRAINT `outiterationbyproductiontype_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outiterationbyproductiontype_ibfk_2` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outiterationbyproductiontype_ibfk_3` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outIterationByZone`
--

DROP TABLE IF EXISTS `outIterationByZone`;
CREATE TABLE `outIterationByZone` (
  `jobID` int(11) NOT NULL default '0',
  `iteration` int(11) NOT NULL default '0',
  `zoneID` int(11) NOT NULL default '0',
  `maxArea` double default NULL,
  `maxAreaDay` int(11) default NULL,
  `finalArea` double default NULL,
  PRIMARY KEY  (`iteration`,`jobID`,`zoneID`),
  KEY `jobID` (`jobID`),
  KEY `zoneID` (`zoneID`),
  CONSTRAINT `outiterationbyzone_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outiterationbyzone_ibfk_2` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outiterationbyzone_ibfk_3` FOREIGN KEY (`zoneID`) REFERENCES `inzone` (`zoneID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outIterationByZoneAndProductionType`
--

DROP TABLE IF EXISTS `outIterationByZoneAndProductionType`;
CREATE TABLE `outIterationByZoneAndProductionType` (
  `jobID` int(11) NOT NULL default '0',
  `iteration` int(11) NOT NULL default '0',
  `zoneID` int(11) NOT NULL default '0',
  `productionTypeID` int(11) NOT NULL default '0',
  `unitDaysInZone` int(11) default NULL,
  `animalDaysInZone` int(11) default NULL,
  `costSurveillance` double default NULL,
  PRIMARY KEY  (`iteration`,`jobID`,`productionTypeID`,`zoneID`),
  KEY `jobID` (`jobID`),
  KEY `productionTypeID` (`productionTypeID`),
  KEY `zoneID` (`zoneID`),
  CONSTRAINT `outiterationbyzoneandproductiontype_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outiterationbyzoneandproductiontype_ibfk_2` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outiterationbyzoneandproductiontype_ibfk_3` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`),
  CONSTRAINT `outiterationbyzoneandproductiontype_ibfk_4` FOREIGN KEY (`zoneID`) REFERENCES `inzone` (`zoneID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outIterationCosts`
--

DROP TABLE IF EXISTS `outIterationCosts`;
CREATE TABLE `outIterationCosts` (
  `jobID` int(11) NOT NULL default '0',
  `iteration` int(11) NOT NULL default '0',
  `productionTypeID` int(11) NOT NULL default '0',
  `destrAppraisal` double default NULL,
  `destrCleaning` double default NULL,
  `destrEuthanasia` double default NULL,
  `destrIndemnification` double default NULL,
  `destrDisposal` double default NULL,
  `vaccSetup` double default NULL,
  `vaccVaccination` double default NULL,
  PRIMARY KEY  (`iteration`,`jobID`,`productionTypeID`),
  KEY `jobID` (`jobID`),
  KEY `productionTypeID` (`productionTypeID`),
  CONSTRAINT `outiterationcosts_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outiterationcosts_ibfk_2` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outiterationcosts_ibfk_3` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outSelectDailyByProductionType`
--

DROP TABLE IF EXISTS `outSelectDailyByProductionType`;
CREATE TABLE `outSelectDailyByProductionType` (
  `jobID` int(11) default NULL,
  `productionTypeID` int(11) default NULL,
  `iteration` int(11) default NULL,
  `day` int(11) default NULL,
  `infcUIni` int(10) unsigned default NULL,
  `infcAIni` int(10) unsigned default NULL,
  `infcUAir` int(10) unsigned default NULL,
  `infcAAir` int(10) unsigned default NULL,
  `infcUDir` int(10) unsigned default NULL,
  `infcADir` int(10) unsigned default NULL,
  `infcUInd` int(10) unsigned default NULL,
  `infcAInd` int(10) unsigned default NULL,
  `detcUClin` int(10) unsigned default NULL,
  `detcAClin` int(10) unsigned default NULL,
  `vaccUIni` int(10) unsigned default NULL,
  `vaccAIni` int(10) unsigned default NULL,
  `vaccURing` int(10) unsigned default NULL,
  `vaccARing` int(10) unsigned default NULL,
  `unitsInZone` int(10) unsigned default NULL,
  `animalsInZone` int(10) unsigned default NULL,
  `descUIni` int(10) unsigned default NULL,
  `descAIni` int(10) unsigned default NULL,
  `descUDet` int(10) unsigned default NULL,
  `descADet` int(10) unsigned default NULL,
  `descUDir` int(10) unsigned default NULL,
  `descADir` int(10) unsigned default NULL,
  `descUInd` int(10) unsigned default NULL,
  `descAInd` int(10) unsigned default NULL,
  `descURing` int(10) unsigned default NULL,
  `descARing` int(10) unsigned default NULL,
  KEY `jobID` (`jobID`),
  KEY `iteration` (`iteration`),
  KEY `productionTypeID` (`productionTypeID`),
  CONSTRAINT `outselectdailybyproductiontype_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outselectdailybyproductiontype_ibfk_2` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`),
  CONSTRAINT `outselectdailybyproductiontype_ibfk_3` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`),
  CONSTRAINT `outselectdailybyproductiontype_ibfk_4` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `outSelectDailyByZoneAndProductionType`
--

DROP TABLE IF EXISTS `outSelectDailyByZoneAndProductionType`;
CREATE TABLE `outSelectDailyByZoneAndProductionType` (
  `jobID` int(11) default NULL,
  `zoneID` int(11) default NULL,
  `productionTypeID` int(11) default NULL,
  `iteration` int(11) default NULL,
  `day` int(11) default NULL,
  `unitsInZoneText` int(10) unsigned default NULL,
  `animalsInZoneText` int(10) unsigned default NULL,
  `unitsInZone` int(10) unsigned default NULL,
  `animalsInZone` int(10) unsigned default NULL,
  KEY `jobID` (`jobID`),
  KEY `zoneID` (`zoneID`),
  KEY `productionTypeID` (`productionTypeID`),
  KEY `iteration` (`iteration`),
  CONSTRAINT `outselectdailybyzoneandproductiontype_ibfk_1` FOREIGN KEY (`jobID`) REFERENCES `job` (`jobID`),
  CONSTRAINT `outselectdailybyzoneandproductiontype_ibfk_2` FOREIGN KEY (`zoneID`) REFERENCES `inzone` (`zoneID`),
  CONSTRAINT `outselectdailybyzoneandproductiontype_ibfk_3` FOREIGN KEY (`productionTypeID`) REFERENCES `inproductiontype` (`productionTypeID`),
  CONSTRAINT `outselectdailybyzoneandproductiontype_ibfk_4` FOREIGN KEY (`iteration`) REFERENCES `outiteration` (`iteration`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `scenario`
--

DROP TABLE IF EXISTS `scenario`;
CREATE TABLE `scenario` (
  `scenarioID` int(11) NOT NULL default '0',
  `descr` mediumtext,
  `nIterations` int(11) default NULL,
  `isComplete` tinyint(1) default NULL,
  `lastUpdated` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`scenarioID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Temporary table structure for view `vTmp`
--

DROP TABLE IF EXISTS `vTmp`;
/*!50001 DROP VIEW IF EXISTS `vTmp`*/;
/*!50001 CREATE TABLE `vTmp` (
  `jobID` int(11)
) */;

--
-- Final view structure for view `vTmp`
--

/*!50001 DROP TABLE IF EXISTS `vTmp`*/;
/*!50001 DROP VIEW IF EXISTS `vTmp`*/;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`areeves`@`%` SQL SECURITY DEFINER */
/*!50001 VIEW `vTmp` AS select distinct `outSelectDailyByProductionType`.`jobID` AS `jobID` from `outSelectDailyByProductionType` where (`outSelectDailyByProductionType`.`vaccURing` is not null) order by `outSelectDailyByProductionType`.`jobID` */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-02-20 23:31:28
