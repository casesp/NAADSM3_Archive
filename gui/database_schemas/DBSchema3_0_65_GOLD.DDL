--    This SQL DDL script was generated by Microsoft Visual Studio (Release Date: LOCAL BUILD).

--    Driver Used : Microsoft Visual Studio - Microsoft Access Driver.
--    Document    : E:\SpreadModel\_Databases\DBSchema3_0_65_GOLD.vsd.
--    Time Created: October 20, 2005 12:32 PM.
--    Operation   : From Visio Update Database Wizard.
--    Connected data source : NADTemplate3_0_50
--    Connected server      : ACCESS
--    Connected database    : E:\SPREADMODEL\_Databases\template-3_0_50.mdb



-- Update existing table `inProductionTypePair`.
create table `inProductionTypePair_IM0` ( 
	`sourceProductionTypeID` INTEGER,
	`destProductionTypeID` INTEGER,
	`useDirectContact` BIT,
	`directContactSpreadID` INTEGER,
	`useIndirectContact` BIT,
	`indirectContactSpreadID` INTEGER,
	`useAirborneSpread` BIT,
	`airborneContactSpreadID` INTEGER); 

insert into `inProductionTypePair_IM0` (`sourceProductionTypeID`, `destProductionTypeID`, `useDirectContact`, `directContactSpreadID`, `useIndirectContact`, `indirectContactSpreadID`, `useAirborneSpread`, `airborneContactSpreadID`) 
    select `inProductionTypePair`.`sourceProductionTypeID`, 
           `inProductionTypePair`.`destProductionTypeID`, 
           `inProductionTypePair`.`useDirectContact`, 
           `inProductionTypePair`.`directContactSpreadID`, 
           `inProductionTypePair`.`useIndirectContact`, 
           `inProductionTypePair`.`indirectContactSpreadID`, 
           `inProductionTypePair`.`useAirborneSpread`, 
           `inProductionTypePair`.`airborneContactSpreadID`  
    from `inProductionTypePair`;


alter table `inProductionTypePair`
	drop constraint `inDiseaseSpread_inProductionTypePairs_FK1`;


alter table `inProductionTypePair`
	drop constraint `inDiseaseSpread_inProductionTypePairs_FK2`;


alter table `inProductionTypePair`
	drop constraint `inDiseaseSpread_inProductionTypePairs_FK3`;


alter table `inProductionTypePair`
	drop constraint `inProductionType_inProductionTypePairs_FK1`;


alter table `inProductionTypePair`
	drop constraint `inProductionType_inProductionTypePairs_FK2`;


drop table `inProductionTypePair`; 


-- `inProductionTypePair` : Table of inProductionTypePair  
create table `inProductionTypePair` ( 
	`sourceProductionTypeID` INTEGER,
	`destProductionTypeID` INTEGER,
	`useDirectContact` BIT,
	`directContactSpreadID` INTEGER,
	`useIndirectContact` BIT,
	`indirectContactSpreadID` INTEGER,
	`useAirborneSpread` BIT,
	`airborneContactSpreadID` INTEGER); 

-- Note: Access does not support the mandatory (not null) constraint in SQL DDL.
-- The mandatory constraints for table '`inProductionTypePair`' will be created in the database using DAO
-- only if there is a connection to the Access MDB file during DDL generation.
-- Not nullable columns for table 'inProductionTypePair' :
-- 	`sourceProductionTypeID`
-- 	`destProductionTypeID`
-- 	`directContactSpreadID`
-- 	`indirectContactSpreadID`
-- 	`airborneContactSpreadID`


update `inProductionTypePair_IM0` set `sourceProductionTypeID` = 1 where `sourceProductionTypeID` is null;

update `inProductionTypePair_IM0` set `destProductionTypeID` = 1 where `destProductionTypeID` is null;

update `inProductionTypePair_IM0` set `directContactSpreadID` = 1 where `directContactSpreadID` is null;

update `inProductionTypePair_IM0` set `indirectContactSpreadID` = 1 where `indirectContactSpreadID` is null;

update `inProductionTypePair_IM0` set `airborneContactSpreadID` = 1 where `airborneContactSpreadID` is null;

insert into `inProductionTypePair` (`sourceProductionTypeID`, `destProductionTypeID`, `useDirectContact`, `directContactSpreadID`, `useIndirectContact`, `indirectContactSpreadID`, `useAirborneSpread`, `airborneContactSpreadID`) 
    select `inProductionTypePair_IM0`.`sourceProductionTypeID`, 
           `inProductionTypePair_IM0`.`destProductionTypeID`, 
           `inProductionTypePair_IM0`.`useDirectContact`, 
           `inProductionTypePair_IM0`.`directContactSpreadID`, 
           `inProductionTypePair_IM0`.`useIndirectContact`, 
           `inProductionTypePair_IM0`.`indirectContactSpreadID`, 
           `inProductionTypePair_IM0`.`useAirborneSpread`, 
           `inProductionTypePair_IM0`.`airborneContactSpreadID`  
    from `inProductionTypePair_IM0`;


drop table `inProductionTypePair_IM0`; 


alter table `inProductionTypePair`
	add constraint `inProductionTypePairs_PK` primary key (`sourceProductionTypeID`, `destProductionTypeID`) ; 


-- Update existing table `inProductionType`.
create table `inProductionType_IM0` ( 
	`productionTypeID` COUNTER,
	`descr` VARCHAR(255),
	`transitionDisease` BIT,
	`latentPeriodPdfID` INTEGER,
	`subclinicalPeriodPdfID` INTEGER,
	`clinicalPeriodPdfID` INTEGER,
	`immunePeriodPdfID` INTEGER,
	`detection` BIT,
	`detProbReportVsTimeClinicalRelID` INTEGER,
	`detProbReportVsFirstDetectionRelID` INTEGER,
	`destrBasic` BIT,
	`destrRing` BIT,
	`destrRingRadius` DOUBLE,
	`destrDirect` BIT,
	`destrDirectSuccess` DOUBLE,
	`destrDirectTracePeriod` INTEGER,
	`destrIndirect` BIT,
	`destrIndirectSuccess` DOUBLE,
	`destrIndirectTracePeriod` INTEGER,
	`destrPriority` INTEGER,
	`useVaccination` BIT,
	`vaccMinTimeBetweenVaccinations` INTEGER,
	`vaccDaysToImmunity` INTEGER,
	`vaccImmunePeriodPdfID` INTEGER,
	`vaccRing` BIT,
	`vaccRingRadius` DOUBLE,
	`vaccPriority` INTEGER); 

insert into `inProductionType_IM0` ( `descr`, `transitionDisease`, `latentPeriodPdfID`, `subclinicalPeriodPdfID`, `clinicalPeriodPdfID`, `immunePeriodPdfID`, `detection`, `detProbReportVsTimeClinicalRelID`, `detProbReportVsFirstDetectionRelID`, `destrBasic`, `destrRing`, `destrRingRadius`, `destrDirect`, `destrDirectSuccess`, `destrDirectTracePeriod`, `destrIndirect`, `destrIndirectSuccess`, `destrIndirectTracePeriod`, `destrPriority`, `useVaccination`, `vaccMinTimeBetweenVaccinations`, `vaccDaysToImmunity`, `vaccImmunePeriodPdfID`, `vaccRing`, `vaccRingRadius`, `vaccPriority`) 
    select `inProductionType`.`descr`, 
           `inProductionType`.`transitionDisease`, 
           `inProductionType`.`latentPeriodPdfID`, 
           `inProductionType`.`subclinicalPeriodPdfID`, 
           `inProductionType`.`clinicalPeriodPdfID`, 
           `inProductionType`.`immunePeriodPdfID`, 
           `inProductionType`.`detection`, 
           `inProductionType`.`detProbReportVsTimeClinicalRelID`, 
           `inProductionType`.`detProbReportVsFirstDetectionRelID`, 
           `inProductionType`.`destrBasic`, 
           `inProductionType`.`destrRing`, 
           `inProductionType`.`destrRingRadius`, 
           `inProductionType`.`destrDirect`, 
           `inProductionType`.`destrDirectSuccess`, 
           `inProductionType`.`destrDirectTracePeriod`, 
           `inProductionType`.`destrIndirect`, 
           `inProductionType`.`destrIndirectSuccess`, 
           `inProductionType`.`destrIndirectTracePeriod`, 
           `inProductionType`.`destrPriority`, 
           `inProductionType`.`useVaccination`, 
           `inProductionType`.`vaccMinTimeBetweenVaccinations`, 
           `inProductionType`.`vaccDaysToImmunity`, 
           `inProductionType`.`vaccImmunePeriodPdfID`, 
           `inProductionType`.`vaccRing`, 
           `inProductionType`.`vaccRingRadius`, 
           `inProductionType`.`vaccPriority`  
    from `inProductionType`;


alter table `dynHerd`
	drop constraint `inProductionType_dynHerd_FK1`;


alter table `outDailyByProductionType`
	drop constraint `inProductionType_outDailyByProductionType_FK1`;


alter table `outIterationByProductionType`
	drop constraint `inProductionType_outIterationByProductionType_FK1`;


alter table `outEpidemicCurves`
	drop constraint `inProductionType_outEpidemicCurves_FK1`;


alter table `inProductionType`
	drop constraint `FK1`;


alter table `inProductionType`
	drop constraint `FK2`;


alter table `inProductionType`
	drop constraint `FK4`;


alter table `inProductionType`
	drop constraint `inChart_inProductionType_FK3`;


alter table `inProductionType`
	drop constraint `inChart_inProductionType_FK5`;


alter table `inProductionType`
	drop constraint `inChart_inProductionType_FK6`;


alter table `inProductionType`
	drop constraint `inChart_inProductionType_FK7`;


drop table `inProductionType`; 


-- `inProductionType` : Table of inProductionType  
create table `inProductionType` ( 
	`productionTypeID` COUNTER,
	`descr` VARCHAR(255),
	`transitionDisease` BIT,
	`latentPeriodPdfID` INTEGER,
	`subclinicalPeriodPdfID` INTEGER,
	`clinicalPeriodPdfID` INTEGER,
	`immunePeriodPdfID` INTEGER,
	`detection` BIT,
	`detProbReportVsTimeClinicalRelID` INTEGER,
	`detProbReportVsFirstDetectionRelID` INTEGER,
	`destrBasic` BIT,
	`destrRing` BIT,
	`destrRingRadius` DOUBLE,
	`destrDirect` BIT,
	`destrDirectSuccess` DOUBLE,
	`destrDirectTracePeriod` INTEGER,
	`destrIndirect` BIT,
	`destrIndirectSuccess` DOUBLE,
	`destrIndirectTracePeriod` INTEGER,
	`destrPriority` INTEGER,
	`useVaccination` BIT,
	`vaccMinTimeBetweenVaccinations` INTEGER,
	`vaccDaysToImmunity` INTEGER,
	`vaccImmunePeriodPdfID` INTEGER,
	`vaccRing` BIT,
	`vaccRingRadius` DOUBLE,
	`vaccPriority` INTEGER); 

-- Note: Access does not support the mandatory (not null) constraint in SQL DDL.
-- The mandatory constraints for table '`inProductionType`' will be created in the database using DAO
-- only if there is a connection to the Access MDB file during DDL generation.
-- Not nullable columns for table 'inProductionType' :
-- 	`productionTypeID`
-- 	`latentPeriodPdfID`
-- 	`subclinicalPeriodPdfID`
-- 	`clinicalPeriodPdfID`
-- 	`immunePeriodPdfID`
-- 	`detProbReportVsTimeClinicalRelID`
-- 	`detProbReportVsFirstDetectionRelID`
-- 	`vaccImmunePeriodPdfID`


-- Note: Access does not support default values in SQL DDL.  Default values will be created in
-- the database using DAO only if there is a connection to the Access MDB file during DDL generation.
-- Columns with default values for table 'inProductionType' :
-- 	`destrDirectTracePeriod`
-- 	`destrIndirectSuccess`


update `inProductionType_IM0` set `latentPeriodPdfID` = 1 where `latentPeriodPdfID` is null;

update `inProductionType_IM0` set `subclinicalPeriodPdfID` = 1 where `subclinicalPeriodPdfID` is null;

update `inProductionType_IM0` set `clinicalPeriodPdfID` = 1 where `clinicalPeriodPdfID` is null;

update `inProductionType_IM0` set `immunePeriodPdfID` = 1 where `immunePeriodPdfID` is null;

update `inProductionType_IM0` set `detProbReportVsTimeClinicalRelID` = 1 where `detProbReportVsTimeClinicalRelID` is null;

update `inProductionType_IM0` set `detProbReportVsFirstDetectionRelID` = 1 where `detProbReportVsFirstDetectionRelID` is null;

update `inProductionType_IM0` set `vaccImmunePeriodPdfID` = 1 where `vaccImmunePeriodPdfID` is null;

insert into `inProductionType` (`descr`, `transitionDisease`, `latentPeriodPdfID`, `subclinicalPeriodPdfID`, `clinicalPeriodPdfID`, `immunePeriodPdfID`, `detection`, `detProbReportVsTimeClinicalRelID`, `detProbReportVsFirstDetectionRelID`, `destrBasic`, `destrRing`, `destrRingRadius`, `destrDirect`, `destrDirectSuccess`, `destrDirectTracePeriod`, `destrIndirect`, `destrIndirectSuccess`, `destrIndirectTracePeriod`, `destrPriority`, `useVaccination`, `vaccMinTimeBetweenVaccinations`, `vaccDaysToImmunity`, `vaccImmunePeriodPdfID`, `vaccRing`, `vaccRingRadius`, `vaccPriority`) 
    select `inProductionType_IM0`.`descr`, 
           `inProductionType_IM0`.`transitionDisease`, 
           `inProductionType_IM0`.`latentPeriodPdfID`, 
           `inProductionType_IM0`.`subclinicalPeriodPdfID`, 
           `inProductionType_IM0`.`clinicalPeriodPdfID`, 
           `inProductionType_IM0`.`immunePeriodPdfID`, 
           `inProductionType_IM0`.`detection`, 
           `inProductionType_IM0`.`detProbReportVsTimeClinicalRelID`, 
           `inProductionType_IM0`.`detProbReportVsFirstDetectionRelID`, 
           `inProductionType_IM0`.`destrBasic`, 
           `inProductionType_IM0`.`destrRing`, 
           `inProductionType_IM0`.`destrRingRadius`, 
           `inProductionType_IM0`.`destrDirect`, 
           `inProductionType_IM0`.`destrDirectSuccess`, 
           `inProductionType_IM0`.`destrDirectTracePeriod`, 
           `inProductionType_IM0`.`destrIndirect`, 
           `inProductionType_IM0`.`destrIndirectSuccess`, 
           `inProductionType_IM0`.`destrIndirectTracePeriod`, 
           `inProductionType_IM0`.`destrPriority`, 
           `inProductionType_IM0`.`useVaccination`, 
           `inProductionType_IM0`.`vaccMinTimeBetweenVaccinations`, 
           `inProductionType_IM0`.`vaccDaysToImmunity`, 
           `inProductionType_IM0`.`vaccImmunePeriodPdfID`, 
           `inProductionType_IM0`.`vaccRing`, 
           `inProductionType_IM0`.`vaccRingRadius`, 
           `inProductionType_IM0`.`vaccPriority`  
    from `inProductionType_IM0`;


drop table `inProductionType_IM0`; 


alter table `inProductionType`
	add constraint `inProductionType_PK` primary key (`productionTypeID`) ; 


-- Update existing table `inGeneral`.
create table `inGeneral_IM0` ( 
	`inGeneralID` VARCHAR(10),
	`scenarioDescr` LONGTEXT,
	`iterations` INTEGER,
	`days` INTEGER,
	`simStopReason` VARCHAR(25),
	`includeContactSpread` BIT,
	`includeAirborneSpread` BIT,
	`includeEconomics` BIT,
	`useFixedRandomSeed` BIT,
	`randomSeed` INTEGER,
	`useFixedPoissonValue` BIT,
	`poissonValue` DOUBLE,
	`saveAllDailyOutputs` BIT,
	`saveDailyOutputsForIterations` INTEGER,
	`writeDailyStatesFile` BIT,
	`dailyStatesFileName` VARCHAR(255),
	`saveDailyEvents` BIT,
	`saveDailyExposures` BIT); 

insert into `inGeneral_IM0` (`inGeneralID`, `scenarioDescr`, `iterations`, `days`, `simStopReason`, `includeContactSpread`, `includeAirborneSpread`, `includeEconomics`, `useFixedRandomSeed`, `randomSeed`, `useFixedPoissonValue`, `poissonValue`, `saveAllDailyOutputs`, `saveDailyOutputsForIterations`, `writeDailyStatesFile`, `dailyStatesFileName`, `saveDailyEvents`, `saveDailyExposures`) 
    select `inGeneral`.`inGeneralID`, 
           `inGeneral`.`scenarioDescr`, 
           `inGeneral`.`iterations`, 
           `inGeneral`.`days`, 
           `inGeneral`.`simStopReason`, 
           `inGeneral`.`includeContactSpread`, 
           `inGeneral`.`includeAirborneSpread`, 
           `inGeneral`.`includeEconomics`, 
           `inGeneral`.`useFixedRandomSeed`, 
           `inGeneral`.`randomSeed`, 
           `inGeneral`.`useFixedPoissonValue`, 
           `inGeneral`.`poissonValue`, 
           `inGeneral`.`saveAllDailyOutputs`, 
           `inGeneral`.`saveDailyOutputsForIterations`, 
           `inGeneral`.`writeDailyStatesFile`, 
           `inGeneral`.`dailyStatesFileName`, 
           `inGeneral`.`saveDailyEvents`, 
           `inGeneral`.`saveDailyExposures`  
    from `inGeneral`;


drop table `inGeneral`; 


-- `inGeneral` : Table of inGeneral  
create table `inGeneral` ( 
	`inGeneralID` VARCHAR(10),
	`scenarioDescr` LONGTEXT,
	`iterations` INTEGER,
	`days` INTEGER,
	`simStopReason` VARCHAR(25),
	`includeContactSpread` BIT,
	`includeAirborneSpread` BIT,
	`includeEconomics` BIT,
	`useFixedRandomSeed` BIT,
	`randomSeed` INTEGER,
	`useFixedPoissonValue` BIT,
	`poissonValue` DOUBLE,
	`saveAllDailyOutputs` BIT,
	`saveDailyOutputsForIterations` INTEGER,
	`writeDailyStatesFile` BIT,
	`dailyStatesFileName` VARCHAR(255),
	`saveDailyEvents` BIT,
	`saveDailyExposures` BIT); 

-- Note: Access does not support the mandatory (not null) constraint in SQL DDL.
-- The mandatory constraints for table '`inGeneral`' will be created in the database using DAO
-- only if there is a connection to the Access MDB file during DDL generation.
-- Not nullable columns for table 'inGeneral' :
-- 	`inGeneralID`


update `inGeneral_IM0` set `inGeneralID` = '1' where `inGeneralID` is null;

insert into `inGeneral` (`inGeneralID`, `scenarioDescr`, `iterations`, `days`, `simStopReason`, `includeContactSpread`, `includeAirborneSpread`, `includeEconomics`, `useFixedRandomSeed`, `randomSeed`, `useFixedPoissonValue`, `poissonValue`, `saveAllDailyOutputs`, `saveDailyOutputsForIterations`, `writeDailyStatesFile`, `dailyStatesFileName`, `saveDailyEvents`, `saveDailyExposures`) 
    select `inGeneral_IM0`.`inGeneralID`, 
           `inGeneral_IM0`.`scenarioDescr`, 
           `inGeneral_IM0`.`iterations`, 
           `inGeneral_IM0`.`days`, 
           `inGeneral_IM0`.`simStopReason`, 
           `inGeneral_IM0`.`includeContactSpread`, 
           `inGeneral_IM0`.`includeAirborneSpread`, 
           `inGeneral_IM0`.`includeEconomics`, 
           `inGeneral_IM0`.`useFixedRandomSeed`, 
           `inGeneral_IM0`.`randomSeed`, 
           `inGeneral_IM0`.`useFixedPoissonValue`, 
           `inGeneral_IM0`.`poissonValue`, 
           `inGeneral_IM0`.`saveAllDailyOutputs`, 
           `inGeneral_IM0`.`saveDailyOutputsForIterations`, 
           `inGeneral_IM0`.`writeDailyStatesFile`, 
           `inGeneral_IM0`.`dailyStatesFileName`, 
           `inGeneral_IM0`.`saveDailyEvents`, 
           `inGeneral_IM0`.`saveDailyExposures`  
    from `inGeneral_IM0`;


drop table `inGeneral_IM0`; 


alter table `inGeneral`
	add constraint `inGeneral_PK` primary key (`inGeneralID`) ; 


create index `inGeneralID` on `inGeneral` (
	`inGeneralID` ASC); 


-- Update existing table `inDiseaseSpread`.
create table `inDiseaseSpread_IM0` ( 
	`spreadID` COUNTER,
	`spreadMethodCode` CHAR(1),
	`latentCanInfect` BIT,
	`subclinicalCanInfect` BIT,
	`contactRate` DOUBLE,
	`infectionProbability` DOUBLE,
	`distancePdfID` INTEGER,
	`movementControlRelID` INTEGER,
	`transportDelayPdfID` INTEGER,
	`probAirborneSpread1km` DOUBLE,
	`maxDistAirborneSpread` DOUBLE,
	`windDirectionStart` INTEGER,
	`windDirectionEnd` INTEGER); 

insert into `inDiseaseSpread_IM0` ( `spreadMethodCode`, `latentCanInfect`, `subclinicalCanInfect`, `contactRate`, `infectionProbability`, `distancePdfID`, `movementControlRelID`, `transportDelayPdfID`, `probAirborneSpread1km`, `maxDistAirborneSpread`, `windDirectionStart`, `windDirectionEnd`) 
    select `inDiseaseSpread`.`spreadMethodCode`, 
           `inDiseaseSpread`.`latentCanInfect`, 
           `inDiseaseSpread`.`subclinicalCanInfect`, 
           `inDiseaseSpread`.`contactRate`, 
           `inDiseaseSpread`.`infectionProbability`, 
           `inDiseaseSpread`.`distancePdfID`, 
           `inDiseaseSpread`.`movementControlRelID`, 
           `inDiseaseSpread`.`transportDelayPdfID`, 
           `inDiseaseSpread`.`probAirborneSpread1km`, 
           `inDiseaseSpread`.`maxDistAirborneSpread`, 
           `inDiseaseSpread`.`windDirectionStart`, 
           `inDiseaseSpread`.`windDirectionEnd`  
    from `inDiseaseSpread`;


alter table `inDiseaseSpread`
	drop constraint `inChart_inDiseaseSpread_FK3`;


alter table `inDiseaseSpread`
	drop constraint `inChart_inProductionTypePair_FK1`;


alter table `inDiseaseSpread`
	drop constraint `inChart_inProductionTypePair_FK3`;


drop table `inDiseaseSpread`; 


-- `inDiseaseSpread` : Table of inDiseaseSpread  
create table `inDiseaseSpread` ( 
	`spreadID` COUNTER,
	`spreadMethodCode` CHAR(1),
	`latentCanInfect` BIT,
	`subclinicalCanInfect` BIT,
	`contactRate` DOUBLE,
	`infectionProbability` DOUBLE,
	`distancePdfID` INTEGER,
	`movementControlRelID` INTEGER,
	`transportDelayPdfID` INTEGER,
	`probAirborneSpread1km` DOUBLE,
	`maxDistAirborneSpread` DOUBLE,
	`windDirectionStart` INTEGER,
	`windDirectionEnd` INTEGER); 

-- Note: Access does not support the mandatory (not null) constraint in SQL DDL.
-- The mandatory constraints for table '`inDiseaseSpread`' will be created in the database using DAO
-- only if there is a connection to the Access MDB file during DDL generation.
-- Not nullable columns for table 'inDiseaseSpread' :
-- 	`spreadID`
-- 	`distancePdfID`
-- 	`movementControlRelID`
-- 	`transportDelayPdfID`


update `inDiseaseSpread_IM0` set `distancePdfID` = 1 where `distancePdfID` is null;

update `inDiseaseSpread_IM0` set `movementControlRelID` = 1 where `movementControlRelID` is null;

update `inDiseaseSpread_IM0` set `transportDelayPdfID` = 1 where `transportDelayPdfID` is null;

insert into `inDiseaseSpread` (`spreadMethodCode`, `latentCanInfect`, `subclinicalCanInfect`, `contactRate`, `infectionProbability`, `distancePdfID`, `movementControlRelID`, `transportDelayPdfID`, `probAirborneSpread1km`, `maxDistAirborneSpread`, `windDirectionStart`, `windDirectionEnd`) 
    select `inDiseaseSpread_IM0`.`spreadMethodCode`, 
           `inDiseaseSpread_IM0`.`latentCanInfect`, 
           `inDiseaseSpread_IM0`.`subclinicalCanInfect`, 
           `inDiseaseSpread_IM0`.`contactRate`, 
           `inDiseaseSpread_IM0`.`infectionProbability`, 
           `inDiseaseSpread_IM0`.`distancePdfID`, 
           `inDiseaseSpread_IM0`.`movementControlRelID`, 
           `inDiseaseSpread_IM0`.`transportDelayPdfID`, 
           `inDiseaseSpread_IM0`.`probAirborneSpread1km`, 
           `inDiseaseSpread_IM0`.`maxDistAirborneSpread`, 
           `inDiseaseSpread_IM0`.`windDirectionStart`, 
           `inDiseaseSpread_IM0`.`windDirectionEnd`  
    from `inDiseaseSpread_IM0`;


drop table `inDiseaseSpread_IM0`; 


alter table `inDiseaseSpread`
	add constraint `inProductionTypePair_PK` primary key (`spreadID`) ; 


-- Update existing table `inControlsGlobal`.
create table `inControlsGlobal_IM0` ( 
	`controlsGlobalID` CHAR(10),
	`includeDetection` BIT,
	`includeDestruction` BIT,
	`destrProgramDelay` INTEGER,
	`destrCapacityRelID` INTEGER,
	`destrPriorityOrder` VARCHAR(255),
	`destrReasonOrder` VARCHAR(255),
	`includeVaccination` BIT,
	`vaccDetectedUnitsBeforeStart` INTEGER,
	`vaccCapacityRelID` INTEGER,
	`vaccPriorityOrder` VARCHAR(255)); 

insert into `inControlsGlobal_IM0` (`controlsGlobalID`, `includeDetection`, `includeDestruction`, `destrProgramDelay`, `destrCapacityRelID`, `destrPriorityOrder`, `destrReasonOrder`, `includeVaccination`, `vaccDetectedUnitsBeforeStart`, `vaccCapacityRelID`, `vaccPriorityOrder`) 
    select `inControlsGlobal`.`controlsGlobalID`, 
           `inControlsGlobal`.`includeDetection`, 
           `inControlsGlobal`.`includeDestruction`, 
           `inControlsGlobal`.`destrProgramDelay`, 
           `inControlsGlobal`.`destrCapacityRelID`, 
           `inControlsGlobal`.`destrPriorityOrder`, 
           `inControlsGlobal`.`destrReasonOrder`, 
           `inControlsGlobal`.`includeVaccination`, 
           `inControlsGlobal`.`vaccDetectedUnitsBeforeStart`, 
           `inControlsGlobal`.`vaccCapacityRelID`, 
           `inControlsGlobal`.`vaccPriorityOrder`  
    from `inControlsGlobal`;


alter table `inControlsGlobal`
	drop constraint `inChart_inControlsGlobal_FK1`;


alter table `inControlsGlobal`
	drop constraint `inChart_inControlsGlobal_FK2`;


drop table `inControlsGlobal`; 


-- `inControlsGlobal` : Table of inControlsGlobal  
create table `inControlsGlobal` ( 
	`controlsGlobalID` CHAR(10),
	`includeDetection` BIT,
	`includeDestruction` BIT,
	`destrProgramDelay` INTEGER,
	`destrCapacityRelID` INTEGER,
	`destrPriorityOrder` VARCHAR(255),
	`destrReasonOrder` VARCHAR(255),
	`includeVaccination` BIT,
	`vaccDetectedUnitsBeforeStart` INTEGER,
	`vaccCapacityRelID` INTEGER,
	`vaccPriorityOrder` VARCHAR(255)); 

-- Note: Access does not support the mandatory (not null) constraint in SQL DDL.
-- The mandatory constraints for table '`inControlsGlobal`' will be created in the database using DAO
-- only if there is a connection to the Access MDB file during DDL generation.
-- Not nullable columns for table 'inControlsGlobal' :
-- 	`controlsGlobalID`
-- 	`destrCapacityRelID`
-- 	`vaccCapacityRelID`


update `inControlsGlobal_IM0` set `controlsGlobalID` = '1' where `controlsGlobalID` is null;

update `inControlsGlobal_IM0` set `destrCapacityRelID` = 1 where `destrCapacityRelID` is null;

update `inControlsGlobal_IM0` set `vaccCapacityRelID` = 1 where `vaccCapacityRelID` is null;

insert into `inControlsGlobal` (`controlsGlobalID`, `includeDetection`, `includeDestruction`, `destrProgramDelay`, `destrCapacityRelID`, `destrPriorityOrder`, `destrReasonOrder`, `includeVaccination`, `vaccDetectedUnitsBeforeStart`, `vaccCapacityRelID`, `vaccPriorityOrder`) 
    select `inControlsGlobal_IM0`.`controlsGlobalID`, 
           `inControlsGlobal_IM0`.`includeDetection`, 
           `inControlsGlobal_IM0`.`includeDestruction`, 
           `inControlsGlobal_IM0`.`destrProgramDelay`, 
           `inControlsGlobal_IM0`.`destrCapacityRelID`, 
           `inControlsGlobal_IM0`.`destrPriorityOrder`, 
           `inControlsGlobal_IM0`.`destrReasonOrder`, 
           `inControlsGlobal_IM0`.`includeVaccination`, 
           `inControlsGlobal_IM0`.`vaccDetectedUnitsBeforeStart`, 
           `inControlsGlobal_IM0`.`vaccCapacityRelID`, 
           `inControlsGlobal_IM0`.`vaccPriorityOrder`  
    from `inControlsGlobal_IM0`;


drop table `inControlsGlobal_IM0`; 


alter table `inControlsGlobal`
	add constraint `inControlsGlobal_PK` primary key (`controlsGlobalID`) ; 


-- Update existing table `inChartDetail`.
create table `inChartDetail_IM0` ( 
	`pointOrder` INTEGER,
	`x` DOUBLE,
	`y` DOUBLE,
	`chartID` INTEGER); 

insert into `inChartDetail_IM0` (`pointOrder`, `x`, `y`, `chartID`) 
    select `inChartDetail`.`pointOrder`, 
           `inChartDetail`.`x`, 
           `inChartDetail`.`y`, 
           `inChartDetail`.`chartID`  
    from `inChartDetail`;


alter table `inChartDetail`
	drop constraint `inChart_inChartDetail_FK1`;


drop table `inChartDetail`; 


-- `inChartDetail` : Table of inChartDetail  
create table `inChartDetail` ( 
	`pointOrder` INTEGER,
	`x` DOUBLE,
	`y` DOUBLE,
	`chartID` INTEGER); 

-- Note: Access does not support the mandatory (not null) constraint in SQL DDL.
-- The mandatory constraints for table '`inChartDetail`' will be created in the database using DAO
-- only if there is a connection to the Access MDB file during DDL generation.
-- Not nullable columns for table 'inChartDetail' :
-- 	`pointOrder`
-- 	`chartID`


update `inChartDetail_IM0` set `pointOrder` = 1 where `pointOrder` is null;

update `inChartDetail_IM0` set `chartID` = 1 where `chartID` is null;

insert into `inChartDetail` (`pointOrder`, `x`, `y`, `chartID`) 
    select `inChartDetail_IM0`.`pointOrder`, 
           `inChartDetail_IM0`.`x`, 
           `inChartDetail_IM0`.`y`, 
           `inChartDetail_IM0`.`chartID`  
    from `inChartDetail_IM0`;


drop table `inChartDetail_IM0`; 


alter table `inChartDetail`
	add constraint `inChartDetail_PK` primary key (`pointOrder`, `chartID`) ; 


-- Update existing table `inChart`.
create table `inChart_IM0` ( 
	`chartID` COUNTER,
	`fieldName` VARCHAR(50),
	`chartName` VARCHAR(150),
	`isPdf` BIT,
	`chartType` VARCHAR(20),
	`mean` DOUBLE,
	`stddev` DOUBLE,
	`min` DOUBLE,
	`mode` DOUBLE,
	`max` DOUBLE,
	`alpha` DOUBLE,
	`beta` DOUBLE,
	`location` DOUBLE,
	`scale` DOUBLE,
	`shape` DOUBLE,
	`zeta` DOUBLE,
	`sigma` DOUBLE,
	`xAxisUnits` VARCHAR(50),
	`yAxisUnits` VARCHAR(50)); 

insert into `inChart_IM0` ( `fieldName`, `chartName`, `isPdf`, `chartType`, `mean`, `stddev`, `min`, `mode`, `max`, `alpha`, `beta`, `location`, `scale`, `shape`, `zeta`, `sigma`, `xAxisUnits`, `yAxisUnits`) 
    select `inChart`.`fieldName`, 
           `inChart`.`chartName`, 
           `inChart`.`isPdf`, 
           `inChart`.`chartType`, 
           `inChart`.`mean`, 
           `inChart`.`stddev`, 
           `inChart`.`min`, 
           `inChart`.`mode`, 
           `inChart`.`max`, 
           `inChart`.`alpha`, 
           `inChart`.`beta`, 
           `inChart`.`location`, 
           `inChart`.`scale`, 
           `inChart`.`shape`, 
           `inChart`.`zeta`, 
           `inChart`.`sigma`, 
           `inChart`.`xAxisUnits`, 
           `inChart`.`yAxisUnits`  
    from `inChart`;


drop table `inChart`; 


-- `inChart` : Table of inChart  
create table `inChart` ( 
	`chartID` COUNTER,
	`fieldName` VARCHAR(50),
	`chartName` VARCHAR(150),
	`isPdf` BIT,
	`chartType` VARCHAR(20),
	`mean` DOUBLE,
	`stddev` DOUBLE,
	`min` DOUBLE,
	`mode` DOUBLE,
	`max` DOUBLE,
	`alpha` DOUBLE,
	`beta` DOUBLE,
	`location` DOUBLE,
	`scale` DOUBLE,
	`shape` DOUBLE,
	`zeta` DOUBLE,
	`sigma` DOUBLE,
	`xAxisUnits` VARCHAR(50),
	`yAxisUnits` VARCHAR(50)); 

-- Note: Access does not support the mandatory (not null) constraint in SQL DDL.
-- The mandatory constraints for table '`inChart`' will be created in the database using DAO
-- only if there is a connection to the Access MDB file during DDL generation.
-- Not nullable columns for table 'inChart' :
-- 	`chartID`


insert into `inChart` (`fieldName`, `chartName`, `isPdf`, `chartType`, `mean`, `stddev`, `min`, `mode`, `max`, `alpha`, `beta`, `location`, `scale`, `shape`, `zeta`, `sigma`, `xAxisUnits`, `yAxisUnits`) 
    select `inChart_IM0`.`fieldName`, 
           `inChart_IM0`.`chartName`, 
           `inChart_IM0`.`isPdf`, 
           `inChart_IM0`.`chartType`, 
           `inChart_IM0`.`mean`, 
           `inChart_IM0`.`stddev`, 
           `inChart_IM0`.`min`, 
           `inChart_IM0`.`mode`, 
           `inChart_IM0`.`max`, 
           `inChart_IM0`.`alpha`, 
           `inChart_IM0`.`beta`, 
           `inChart_IM0`.`location`, 
           `inChart_IM0`.`scale`, 
           `inChart_IM0`.`shape`, 
           `inChart_IM0`.`zeta`, 
           `inChart_IM0`.`sigma`, 
           `inChart_IM0`.`xAxisUnits`, 
           `inChart_IM0`.`yAxisUnits`  
    from `inChart_IM0`;


drop table `inChart_IM0`; 


alter table `inChart`
	add constraint `inChart_PK` primary key (`chartID`) ; 


create unique index `chartNameIndex` on `inChart` (
	`chartName`); 


-- Update existing table `dynHerd`.
create table `dynHerd_IM1` ( 
	`herdID` COUNTER,
	`productionTypeID` INTEGER,
	`latitude` DOUBLE,
	`longitude` DOUBLE,
	`initialStateCode` CHAR(1),
	`daysLeftInInitialState` INTEGER,
	`initialSize` INTEGER,
	`finalStateCode` CHAR(1),
	`finalApparentStateCode` CHAR(1),
	`cumInfected` INTEGER,
	`cumDestroyed` INTEGER,
	`cumVaccinated` INTEGER); 

insert into `dynHerd_IM1` ( `productionTypeID`, `latitude`, `longitude`, `initialStateCode`, `daysLeftInInitialState`, `initialSize`, `finalStateCode`, `finalApparentStateCode`, `cumInfected`, `cumDestroyed`, `cumVaccinated`) 
    select `dynHerd`.`productionTypeID`, 
           `dynHerd`.`latitude`, 
           `dynHerd`.`longitude`, 
           `dynHerd`.`initialStateCode`, 
           `dynHerd`.`daysLeftInInitialState`, 
           `dynHerd`.`initialSize`, 
           `dynHerd`.`finalStateCode`, 
           `dynHerd`.`finalApparentStateCode`, 
           `dynHerd`.`cumInfected`, 
           `dynHerd`.`cumDestroyed`, 
           `dynHerd`.`cumVaccinated`  
    from `dynHerd`;


alter table `outDailyExposures`
	drop constraint `dynHerd_outDailyExposures_FK1`;


alter table `outDailyExposures`
	drop constraint `dynHerd_outDailyExposures_FK2`;


alter table `outDailyEvents`
	drop constraint `dynHerd_outDailyEvents_FK1`;


alter table `dynHerd`
	drop constraint `inProductionType_dynHerd_FK1`;


drop table `dynHerd`; 


-- `dynHerd` : Table of dynHerd  
create table `dynHerd` ( 
	`herdID` COUNTER,
	`productionTypeID` INTEGER,
	`latitude` DOUBLE,
	`longitude` DOUBLE,
	`initialStateCode` CHAR(1),
	`daysLeftInInitialState` INTEGER,
	`initialSize` INTEGER,
	`finalStateCode` CHAR(1),
	`finalApparentStateCode` CHAR(1),
	`cumInfected` INTEGER,
	`cumDestroyed` INTEGER,
	`cumVaccinated` INTEGER); 

-- Note: Access does not support the mandatory (not null) constraint in SQL DDL.
-- The mandatory constraints for table '`dynHerd`' will be created in the database using DAO
-- only if there is a connection to the Access MDB file during DDL generation.
-- Not nullable columns for table 'dynHerd' :
-- 	`herdID`
-- 	`productionTypeID`


insert into `dynHerd` (`productionTypeID`, `latitude`, `longitude`, `initialStateCode`, `daysLeftInInitialState`, `initialSize`, `finalStateCode`, `finalApparentStateCode`, `cumInfected`, `cumDestroyed`, `cumVaccinated`) 
    select `dynHerd_IM1`.`productionTypeID`, 
           `dynHerd_IM1`.`latitude`, 
           `dynHerd_IM1`.`longitude`, 
           `dynHerd_IM1`.`initialStateCode`, 
           `dynHerd_IM1`.`daysLeftInInitialState`, 
           `dynHerd_IM1`.`initialSize`, 
           `dynHerd_IM1`.`finalStateCode`, 
           `dynHerd_IM1`.`finalApparentStateCode`, 
           `dynHerd_IM1`.`cumInfected`, 
           `dynHerd_IM1`.`cumDestroyed`, 
           `dynHerd_IM1`.`cumVaccinated`  
    from `dynHerd_IM1`;


drop table `dynHerd_IM1`; 


alter table `dynHerd`
	add constraint `dynHerd_PK` primary key (`herdID`) ; 


-- Update existing table `DBSchemaVersion`.
create table `DBSchemaVersion_IM0` ( 
	`VersionNumber` VARCHAR(255),
	`VersionApplication` CHAR(10),
	`VersionDate` DATETIME,
	`VersionInfoURL` VARCHAR(255)); 

insert into `DBSchemaVersion_IM0` (`VersionNumber`, `VersionApplication`, `VersionDate`, `VersionInfoURL`) 
    select `DBSchemaVersion`.`VersionNumber`, 
           `DBSchemaVersion`.`VersionApplication`, 
           `DBSchemaVersion`.`VersionDate`, 
           `DBSchemaVersion`.`VersionInfoURL`  
    from `DBSchemaVersion`;


drop table `DBSchemaVersion`; 


-- `DBSchemaVersion` : Table of DBSchemaVersion  
create table `DBSchemaVersion` ( 
	`VersionNumber` VARCHAR(255),
	`VersionApplication` CHAR(10),
	`VersionDate` DATETIME,
	`VersionInfoURL` VARCHAR(255)); 

-- Note: Access does not support the mandatory (not null) constraint in SQL DDL.
-- The mandatory constraints for table '`DBSchemaVersion`' will be created in the database using DAO
-- only if there is a connection to the Access MDB file during DDL generation.
-- Not nullable columns for table 'DBSchemaVersion' :
-- 	`VersionNumber`
-- 	`VersionApplication`


update `DBSchemaVersion_IM0` set `VersionNumber` = '1' where `VersionNumber` is null;

update `DBSchemaVersion_IM0` set `VersionApplication` = '1' where `VersionApplication` is null;

insert into `DBSchemaVersion` (`VersionNumber`, `VersionApplication`, `VersionDate`, `VersionInfoURL`) 
    select `DBSchemaVersion_IM0`.`VersionNumber`, 
           `DBSchemaVersion_IM0`.`VersionApplication`, 
           `DBSchemaVersion_IM0`.`VersionDate`, 
           `DBSchemaVersion_IM0`.`VersionInfoURL`  
    from `DBSchemaVersion_IM0`;


drop table `DBSchemaVersion_IM0`; 


alter table `DBSchemaVersion`
	add constraint `DBSchemaVersion_PK` primary key (`VersionNumber`, `VersionApplication`) ; 


-- Restore foreign key constraints for existing tables
alter table `inProductionTypePair`
	add constraint `inDiseaseSpread_inProductionTypePairs_FK1` foreign key (
		`directContactSpreadID`)
	 references `inDiseaseSpread` (
		`spreadID`); 

alter table `inProductionTypePair`
	add constraint `inDiseaseSpread_inProductionTypePairs_FK2` foreign key (
		`indirectContactSpreadID`)
	 references `inDiseaseSpread` (
		`spreadID`); 

alter table `inProductionTypePair`
	add constraint `inDiseaseSpread_inProductionTypePairs_FK3` foreign key (
		`airborneContactSpreadID`)
	 references `inDiseaseSpread` (
		`spreadID`); 

alter table `inProductionTypePair`
	add constraint `inProductionType_inProductionTypePairs_FK1` foreign key (
		`sourceProductionTypeID`)
	 references `inProductionType` (
		`productionTypeID`); 

alter table `inProductionTypePair`
	add constraint `inProductionType_inProductionTypePairs_FK2` foreign key (
		`destProductionTypeID`)
	 references `inProductionType` (
		`productionTypeID`); 

alter table `inProductionType`
	add constraint `FK1` foreign key (
		`latentPeriodPdfID`)
	 references `inChart` (
		`chartID`); 

alter table `inProductionType`
	add constraint `FK2` foreign key (
		`subclinicalPeriodPdfID`)
	 references `inChart` (
		`chartID`); 

alter table `inProductionType`
	add constraint `FK4` foreign key (
		`immunePeriodPdfID`)
	 references `inChart` (
		`chartID`); 

alter table `inProductionType`
	add constraint `inChart_inProductionType_FK3` foreign key (
		`clinicalPeriodPdfID`)
	 references `inChart` (
		`chartID`); 

alter table `inProductionType`
	add constraint `inChart_inProductionType_FK5` foreign key (
		`detProbReportVsTimeClinicalRelID`)
	 references `inChart` (
		`chartID`); 

alter table `inProductionType`
	add constraint `inChart_inProductionType_FK6` foreign key (
		`detProbReportVsFirstDetectionRelID`)
	 references `inChart` (
		`chartID`); 

alter table `inProductionType`
	add constraint `inChart_inProductionType_FK7` foreign key (
		`vaccImmunePeriodPdfID`)
	 references `inChart` (
		`chartID`); 

alter table `dynHerd`
	add constraint `inProductionType_dynHerd_FK1` foreign key (
		`productionTypeID`)
	 references `inProductionType` (
		`productionTypeID`); 

alter table `outDailyByProductionType`
	add constraint `inProductionType_outDailyByProductionType_FK1` foreign key (
		`productionTypeID`)
	 references `inProductionType` (
		`productionTypeID`); 

alter table `outIterationByProductionType`
	add constraint `inProductionType_outIterationByProductionType_FK1` foreign key (
		`productionTypeID`)
	 references `inProductionType` (
		`productionTypeID`); 

alter table `outEpidemicCurves`
	add constraint `inProductionType_outEpidemicCurves_FK1` foreign key (
		`productionTypeID`)
	 references `inProductionType` (
		`productionTypeID`); 

alter table `inDiseaseSpread`
	add constraint `inChart_inDiseaseSpread_FK3` foreign key (
		`transportDelayPdfID`)
	 references `inChart` (
		`chartID`); 

alter table `inDiseaseSpread`
	add constraint `inChart_inProductionTypePair_FK1` foreign key (
		`distancePdfID`)
	 references `inChart` (
		`chartID`); 

alter table `inDiseaseSpread`
	add constraint `inChart_inProductionTypePair_FK3` foreign key (
		`movementControlRelID`)
	 references `inChart` (
		`chartID`); 

alter table `inControlsGlobal`
	add constraint `inChart_inControlsGlobal_FK1` foreign key (
		`destrCapacityRelID`)
	 references `inChart` (
		`chartID`); 

alter table `inControlsGlobal`
	add constraint `inChart_inControlsGlobal_FK2` foreign key (
		`vaccCapacityRelID`)
	 references `inChart` (
		`chartID`); 

alter table `inChartDetail`
	add constraint `inChart_inChartDetail_FK1` foreign key (
		`chartID`)
	 references `inChart` (
		`chartID`); 

alter table `outDailyExposures`
	add constraint `dynHerd_outDailyExposures_FK1` foreign key (
		`exposedHerdID`)
	 references `dynHerd` (
		`herdID`); 

alter table `outDailyExposures`
	add constraint `dynHerd_outDailyExposures_FK2` foreign key (
		`exposingHerdID`)
	 references `dynHerd` (
		`herdID`); 

alter table `outDailyEvents`
	add constraint `dynHerd_outDailyEvents_FK1` foreign key (
		`herdID`)
	 references `dynHerd` (
		`herdID`); 


-- This is the end of the Microsoft Visual Studio generated SQL DDL script.
