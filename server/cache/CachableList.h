
#ifndef KCACHABLE
#define KCACHABLE(name, key, symbol, type, query)
#endif
#ifndef CACHABLE
#define CACHABLE(name, symbol, type, query) \
	KCACHABLE(name, name, symbol, type, query)
#endif
#ifndef ICACHABLE
#define ICACHABLE(name, symbol, type, key, query) \
	KCACHABLE(name, name, symbol, type, query)
#endif


CACHABLE("config.BulkData.billtypes", BillTypes, TupleSet,
	"SELECT billTypeID,billTypeName,description FROM billTypes"
);
CACHABLE("config.BulkData.messages", EveMessages, TupleSet,
	"SELECT messageID,messageType,messageText,urlAudio,urlIcon FROM eveMessages"
);
CACHABLE("config.BulkData.allianceshortnames", AllianceShortnames, TupleSet,
	"SELECT allianceID,shortName FROM alliance_shortnames"
);
CACHABLE("config.BulkData.categories", EveCategories, TupleSet,
	"SELECT categoryID,categoryName,description,graphicID FROM eveCategories"
);
CACHABLE("config.BulkData.invtypereactions", invTypeReactions, TupleSet,
	"SELECT reactionTypeID,input,typeID,quantity FROM invTypeReactions"
);
/* this query is a strange one, we might have to develop a non-macro based
 * solution for building this one because it stretches all of the rules and is
 * requiring all kinds of special cases...
 *
 * I added the default value join in here to try to squeeze the size down a bit
 * (we are barely over the max packet size right now)... I have no idea how they
 * are determining what to include in the dgmTypeAttribs table (which has a
 * string value type, so it makes the client unhappy)... they must have some
 * sort of filter, but hell if I can figure it out...
 */
CACHABLE("config.BulkData.dgmtypeattribs", dgmTypeAttribs, TupleSet,
	"SELECT "
	"	dgmTypeAttributes.typeID,"
	"	dgmTypeAttributes.attributeID,"
	"	IF(valueInt IS NULL, valueFloat, valueInt) AS value "
	" FROM dgmTypeAttributes"
	"	NATURAL JOIN dgmAttributeTypes"
	" WHERE"
	"	defaultValue!=valueInt "
);
CACHABLE("config.BulkData.dgmtypeeffects", dgmTypeEffects, TupleSet,
	"SELECT typeID,effectID,isDefault FROM dgmTypeEffects"
);
CACHABLE("config.BulkData.dgmeffects", dgmEffects, TupleSet,
	"SELECT effectID,effectName,effectCategory,preExpression,postExpression,description,guid,graphicID,isOffensive,isAssistance,durationAttributeID,trackingSpeedAttributeID,dischargeAttributeID,rangeAttributeID,falloffAttributeID,published,displayName,isWarpSafe,rangeChance,electronicChance,propulsionChance,distribution,sfxName,npcUsageChanceAttributeID,npcActivationChanceAttributeID FROM dgmEffects"
);
CACHABLE("config.BulkData.dgmattribs", dgmAttribs, TupleSet,
	"SELECT attributeID,attributeName,attributeCategory,description,maxAttributeID,attributeIdx,graphicID,chargeRechargeTimeID,defaultValue,published,unitID,displayName,stackable,highIsGood FROM dgmAttributes"
);
CACHABLE("config.BulkData.metagroups", invMetaGroups, TupleSet,
	"SELECT metaGroupID,metaGroupName,description,graphicID FROM invMetaGroups"
);
CACHABLE("config.BulkData.ramactivities", ramActivities, TupleSet,
	"SELECT activityID,activityName,iconNo,description,published FROM ramActivities"
);
CACHABLE("config.BulkData.ramaltypesdetailpergroup", ramALTypeGroup, TupleSet,
	"SELECT assemblyLineTypeID,activityID,groupID,timeMultiplier,materialMultiplier FROM ramALTypeGroup"
);
CACHABLE("config.BulkData.ramaltypesdetailpercategory", ramALTypeCategory, TupleSet,
	"SELECT assemblyLineTypeID,activityID,categoryID,timeMultiplier,materialMultiplier FROM ramALTypeCategory"
);
CACHABLE("config.BulkData.ramaltypes", ramALTypes, TupleSet,
	"SELECT assemblyLineTypeID,assemblyLineTypeName,typeName,description,activityID,baseTimeMultiplier,baseMaterialMultiplier,volume FROM ramALTypes"
);
CACHABLE("config.BulkData.ramcompletedstatuses", ramCompletedStatuses, TupleSet,
	"SELECT completedStatusID,completedStatusName,completedStatusText FROM ramCompletedStatuses"
);
//no:
CACHABLE("config.BulkData.ramtypematerials", ramTypeMaterials, RowsetTuple,
	"SELECT requiredTypeID AS typeID,typeID AS materialTypeID,quantity FROM TL2MaterialsForTypeWithActivity"
	//as a blue.DBRow set
	//SELECT requiredTypeID AS typeID,typeID AS materialTypeID,quantity FROM TL2MaterialsForTypeWithActivity
	/*
uint32 quantity;
uint16 materialTypeID;
uint16 typeID;
    */
);
//no:
CACHABLE("config.BulkData.ramtyperequirements", ramTypeRequirements, RowsetTuple,
	"SELECT typeID,activity,requiredTypeID,quantity,damagePerJob,recycle FROM TL2MaterialsForTypeWithActivity"
	//as a blue.DBRow set
	//SELECT typeID,activity,requiredTypeID,quantity,damagePerJob,recycle FROM TL2MaterialsForTypeWithActivity
	/*
double damagePerJob;
uint32 activity;
uint32 quantity;
uint16 requiredTypeID;
uint16 typeID;
uint8 recycle;
    */
);
CACHABLE("config.BulkData.tickernames", tickerNames, TupleSet,
	//npc corps only I think.
	"SELECT corporationID,tickerName,shape1,shape2,shape3,color1,color2,color3 FROM corporation WHERE hasPlayerPersonnelManager=0"
);
CACHABLE("config.BulkData.groups", invGroups, TupleSet,
	"SELECT groupID,categoryID,groupName,description,graphicID,useBasePrice,allowManufacture,allowRecycler,anchored,anchorable,fittableNonSingleton FROM invGroups"
);
CACHABLE("config.BulkData.shiptypes", invShipTypes, TupleSet,
	"SELECT shipTypeID,powerCoreTypeID,capacitorTypeID,shieldTypeID,propulsionTypeID,sensorTypeID,armorTypeID,computerTypeID,weaponTypeID,miningTypeID,skillTypeID FROM invShipTypes"
);
CACHABLE("config.BulkData.locations", cacheLocations, TupleSet,
	//this is such as worthless cache item...
	"SELECT locationID,locationName,x,y,z FROM cacheLocations"
);
CACHABLE("config.BulkData.constants", eveConstants, TupleSet,
	"SELECT constantID,constantValue FROM eveConstants"
);
CACHABLE("config.BulkData.bptypes", invBlueprintTypes, TupleSet,
	//old method used dedicated table invBlueprints
	//"SELECT blueprintTypeName,description,graphicID,basePrice,blueprintTypeID,parentBlueprintTypeID,productTypeID,productionTime,techLevel,researchProductivityTime,researchMaterialTime,researchCopyTime,researchTechTime,productivityModifier,materialModifier,wasteFactor,chanceOfReverseEngineering,maxProductionLimit FROM invBlueprints"
	//all data can be obtained from existing tables:
	"SELECT invTypes.typeName AS blueprintTypeName,invTypes.description,invTypes.graphicID,invTypes.basePrice,blueprintTypeID,parentBlueprintTypeID,productTypeID,productionTime,techLevel,researchProductivityTime,researchMaterialTime,researchCopyTime,researchTechTime,productivityModifier,materialModifier,wasteFactor,chanceOfReverseEngineering,maxProductionLimit FROM invBlueprintTypes,invTypes WHERE invBlueprintTypes.blueprintTypeID=invTypes.typeID"
);
CACHABLE("config.BulkData.graphics", eveGraphics, TupleSet,
	"SELECT graphicID,url3D,urlWeb,icon,urlSound,explosionID FROM eveGraphics"
);
CACHABLE("config.BulkData.types", invTypes, TupleSet,
	"SELECT typeID,groupID,typeName,description,graphicID,radius,mass,volume,capacity,portionSize,raceID,basePrice,published,marketGroupID,chanceOfDuplicating FROM invTypes"
);
CACHABLE("config.BulkData.invmetatypes", invMetaTypes, TupleSet,
	"SELECT typeID,parentTypeID,metaGroupID FROM invMetaTypes"
);
CACHABLE("config.Bloodlines", chrBloodlines, Rowset,
	"SELECT bloodlineID,bloodlineName,raceID,description,maleDescription,femaleDescription,shipTypeID,corporationID,perception,willpower,charisma,memory,intelligence,bonusTypeID,skillTypeID1,skillTypeID2,graphicID FROM chrBloodlines"
);
CACHABLE("config.Units", eveUnits, Rowset,
	"SELECT unitID,unitName,displayName FROM eveUnits"
);
CACHABLE("config.Statistics", eveStatistics, Rowset,
	"SELECT statisticID,statisticName,statisticType,description,orderID,graphicID FROM eveStatistics"
);
CACHABLE("config.EncyclopediaTypes", eveEncyclTypes, Rowset,
	"SELECT encyclopediaTypeID,encyclopediaTypeName FROM eveEncyclTypes"
);
CACHABLE("config.ChannelTypes", channelTypes, Rowset,
	"SELECT channelTypeID,channelTypeName,defaultName FROM channelTypes"
);
CACHABLE("config.BulkData.owners", cacheOwners, TupleSet,
	"SELECT ownerID,ownerName,typeID FROM cacheOwners"
);
CACHABLE("config.StaticOwners", eveStaticOwners, Rowset,
	//im pretty sure this data should not be in its own table, but I cant figure out the filter on eveNames to get it.
	"SELECT ownerID,ownerName,typeID FROM eveStaticOwners"
);
CACHABLE("config.Races", chrRaces, Rowset,
	"SELECT raceID,raceName,description,skillTypeID1,typeID,typeQuantity,graphicID FROM chrRaces"
);
CACHABLE("config.Attributes", chrAttributes, Rowset,
	"SELECT attributeID,attributeName,description,graphicID FROM chrAttributes"
);
CACHABLE("config.Roles", eveRoles, Rowset,
	"SELECT roleID,roleName,description FROM eveRoles"
);
CACHABLE("config.Flags", eveFlags, Rowset,
	"SELECT flagID,flagName,flagText,flagType,orderID FROM eveFlags"
);
CACHABLE("config.StaticLocations", eveStaticLocations, Rowset,
	//im pretty sure this data should not be in its own table, but I cant figure out the filter on mapDenormalize to get it.
	"SELECT locationID,locationName,x,y,z FROM eveStaticLocations"
);
CACHABLE("config.InvContrabandTypes", invContrabandTypes, Rowset,
	"SELECT factionID,typeID,standingLoss,confiscateMinSec,fineByValue,attackMinSec FROM invContrabandTypes"
);

//GetCharCreationInfo
KCACHABLE("charCreationInfo.bloodlines", "bloodlines", c_chrBloodlines, Rowset,
	"SELECT bloodlineID,bloodlineName,raceID,description,maleDescription,femaleDescription,shipTypeID,corporationID,perception,willpower,charisma,memory,intelligence,bonusTypeID,skillTypeID1,skillTypeID2,graphicID FROM chrBloodlines"
);
KCACHABLE("charCreationInfo.departments", "departments", c_chrDepartments, Rowset,
	"SELECT schoolID,departmentID,departmentName,description,skillTypeID1,skillTypeID2,skillTypeID3,graphicID FROM chrDepartments"
);
KCACHABLE("charCreationInfo.races", "races", c_chrRaces, Rowset,
	"SELECT raceID,raceName,description,skillTypeID1,typeID,typeQuantity,graphicID FROM chrRaces"
);
KCACHABLE("charCreationInfo.ancestries", "ancestries", c_chrAncestries, Rowset,
	"SELECT ancestryID,ancestryName,bloodlineID,description,postPerception,postWillpower,postCharisma,postMemory,postIntelligence,perception,willpower,charisma,memory,intelligence,skillTypeID1,skillTypeID2,typeID,typeQuantity,typeID2,typeQuantity2,graphicID FROM chrAncestries"
);
KCACHABLE("charCreationInfo.specialities", "specialities", c_chrSpecialities, Rowset,
	"SELECT fieldID,specialityID,specialityName,description,skillTypeID1,skillTypeID2,typeID,typeQuantity,typeID2,typeQuantity2,graphicID FROM chrSpecialities"
);
KCACHABLE("charCreationInfo.schools", "schools", c_chrSchools, Rowset,
	"SELECT raceID,schoolID,schoolName,description,graphicID,corporationID,agentID,newAgentID FROM chrSchools"
);
KCACHABLE("charCreationInfo.attributes", "attributes", c_chrAttributes, Rowset,
	"SELECT attributeID,attributeName,description,graphicID FROM chrAttributes"
);
KCACHABLE("charCreationInfo.fields", "fields", c_chrFields, Rowset,
	"SELECT departmentID,fieldID,fieldName,description,skillTypeID1,skillTypeID2,typeID,typeQuantity,typeID2,typeQuantity2,graphicID FROM chrFields"
);
KCACHABLE("charCreationInfo.bl_accessories", "accessories", bl_accessories, Rowset,
	"SELECT bloodlineID,gender,accessoryID,npc FROM chrBLAccessories"
);
KCACHABLE("charCreationInfo.bl_lights", "lights", bl_lights, Rowset,
	"SELECT lightID,lightName FROM chrBLLights"
);
KCACHABLE("charCreationInfo.bl_skins", "skins", bl_skins, Rowset,
	"SELECT bloodlineID,gender,skinID,npc FROM chrBLSkins"
);
KCACHABLE("charCreationInfo.bl_beards", "beards", bl_beards, Rowset,
	"SELECT bloodlineID,gender,beardID,npc FROM chrBLBeards"
);
KCACHABLE("charCreationInfo.bl_eyes", "eyes", bl_eyes, Rowset,
	"SELECT bloodlineID,gender,eyesID,npc FROM chrBLEyes"
);
KCACHABLE("charCreationInfo.bl_lipsticks", "lipsticks", bl_lipsticks, Rowset,
	"SELECT bloodlineID,gender,lipstickID,npc FROM chrBLLipsticks"
);
KCACHABLE("charCreationInfo.bl_makeups", "makeups", bl_makeups, Rowset,
	"SELECT bloodlineID,gender,makeupID,npc FROM chrBLMakeups"
);
KCACHABLE("charCreationInfo.bl_hairs", "hairs", bl_hairs, Rowset,
	"SELECT bloodlineID,gender,hairID,npc FROM chrBLHairs"
);
KCACHABLE("charCreationInfo.bl_backgrounds", "backgrounds", bl_backgrounds, Rowset,
	"SELECT backgroundID,backgroundName FROM chrBLBackgrounds"
);
KCACHABLE("charCreationInfo.bl_decos", "decos", bl_decos, Rowset,
	"SELECT bloodlineID,gender,decoID,npc FROM chrBLDecos"
);
KCACHABLE("charCreationInfo.bl_eyebrows", "eyebrows", bl_eyebrows, Rowset,
	"SELECT bloodlineID,gender,eyebrowsID,npc FROM chrBLEyebrows"
);
KCACHABLE("charCreationInfo.bl_costumes", "costumes", bl_costumes, Rowset,
	"SELECT bloodlineID,gender,costumeID,npc FROM chrBLCostumes"
);

//GetAppearanceInfo
KCACHABLE("charCreationInfo.eyebrows", "eyebrows", a_eyebrows, Rowset,
	"SELECT eyebrowsID,eyebrowsName FROM chrEyebrows"
);
KCACHABLE("charCreationInfo.eyes", "eyes", a_eyes, Rowset,
	"SELECT eyesID,eyesName FROM chrEyes"
);
KCACHABLE("charCreationInfo.decos", "decos", a_decos, Rowset,
	"SELECT decoID,decoName FROM chrDecos"
);
KCACHABLE("charCreationInfo.hairs", "hairs", a_hairs, Rowset,
	"SELECT hairID,hairName FROM chrHairs"
);
KCACHABLE("charCreationInfo.backgrounds", "backgrounds", a_backgrounds, Rowset,
	"SELECT backgroundID,backgroundName FROM chrBackgrounds"
);
KCACHABLE("charCreationInfo.accessories", "accessories", a_accessories, Rowset,
	"SELECT accessoryID,accessoryName FROM chrAccessories"
);
KCACHABLE("charCreationInfo.lights", "lights", a_lights, Rowset,
	"SELECT lightID,lightName FROM chrLights"
);
KCACHABLE("charCreationInfo.costumes", "costumes", a_costumes, Rowset,
	"SELECT costumeID,costumeName FROM chrCostumes"
);
KCACHABLE("charCreationInfo.makeups", "makeups", a_makeups, Rowset,
	"SELECT makeupID,makeupName FROM chrMakeups"
);
KCACHABLE("charCreationInfo.beards", "beards", a_beards, Rowset,
	"SELECT beardID,beardName FROM chrBeards"
);
KCACHABLE("charCreationInfo.skins", "skins", a_skins, Rowset,
	"SELECT skinID,skinName FROM chrSkins"
);
KCACHABLE("charCreationInfo.lipsticks", "lipsticks", a_lipsticks, Rowset,
	"SELECT lipstickID,lipstickName FROM chrLipsticks"
);

//other
ICACHABLE("dogmaIM.attributesByName", dgmAttributesByName, IndexRowset, "attributeName",
	"SELECT attributeID,attributeName,attributeCategory,description,maxAttributeID,attributeIdx,graphicID,chargeRechargeTimeID,defaultValue,published,displayName,unitID,stackable,highIsGood FROM dgmAttributeTypes"
);


/* Kludged cached method calls..
 *
 * Right now we can only cache method calls which do not take any arguments.
 *
 */
#if 0
CACHABLE("Method Call.server."
         "map.GetSolarSystemPseudoSecurities", 
          map_GetSolarSystemPseudoSecurities, Rowset,
		"SELECT solarSystemID, security FROM mapSolarSystems"
);
CACHABLE("Method Call.server."
         "agentMgr.GetAgents", 
          agentMgr_GetAgents, Rowset,
		"SELECT agentID,agentTypeID,divisionID,level,stationID,bloodlineID,quality,corporationID,gender"
		"	FROM agtAgents"
);
CACHABLE("Method Call.server."
         "account.GetRefTypes", 
          account_GetRefTypes, Rowset,
		"SELECT refTypeID,refTypeText,description FROM market_refTypes"
);
//Method Call.server.map.GetStationExtraInfo is too complex for here...
CACHABLE("Method Call.server."
         "account.GetKeyMap", 
          account_GetKeyMap, Rowset,
		"SELECT accountKey,accountType,accountName,description FROM market_keyMap"
);
CACHABLE("Method Call.server."
         "billMgr.GetBillTypes", 
          billMgr_GetBillTypes, Rowset,
		"SELECT billTypeID,billTypeName,description FROM billTypes"
);
CACHABLE("Method Call.server."
         "standing2.GetNPCNPCStandings", 
          standing2_GetNPCNPCStandings, Rowset,
		"SELECT fromID,toID,standing FROM npcStandings"
);
//Method Call.server.beyonce.GetFormations is too complex for here...
#endif


#undef CACHABLE
#undef KCACHABLE
#undef ICACHABLE

