// (c) AbyssX Group
#include "../WorldEnvironment.h"

// Constructor: Player.
Player::Player(QuadWord objectGuid, const char *name, DoubleWord accountId, Client *client) :
	Unit(objectGuid, name), mAccountId(accountId), mClient(client)
{
	spellcount = 0;
	mObjectModel = 0x0032;
	memset(mInventory,0x00,sizeof(mInventory));
	memset(mBackPack,0x00,sizeof(mBackPack));
	mLastMovePacket = NULL;
	mLastStrafePacket = NULL;
	mLastSwimPacket = NULL;
	mLastTurnPacket = NULL;
	mLastWalkPacket = NULL;
	memset(&mStats,0x00,sizeof(mStats));
	memset(&mResistances,0x00,sizeof(mResistances));
}

// Destructor: ~Player.
Player::~Player()
{
	if (mLastMovePacket)
		delete mLastMovePacket;
	if (mLastStrafePacket)
		delete mLastStrafePacket;
	if (mLastSwimPacket)
		delete mLastSwimPacket;
	if (mLastTurnPacket)
		delete mLastTurnPacket;
	if (mLastWalkPacket)
		delete mLastWalkPacket;

}

void RecoverStats(void *Param)
{
	Player *ply;
	ply = (Player *)Param;
	ply->Replenish();
}

void ResSickness(void *Param)
{
	Player *ply;
	ply = (Player *)Param;
	ply->RessurectionSickness();
}

Word Player::GetObjectModel(void)
{
	Word model = mObjectModel;		// Set to 0x0032 in the constructor atm!

	if (mRace < 7)
		model = 0x002F + (mRace * 2) + mGender;
	else if (mRace == 7)
		model = 0x061B + mGender;
	else if (mRace == 8)
		model = 0x05C6 + mGender;

	return model;
}

// Sets the last move packet
void Player::SetLastMovePacket(Packet *pack)
{
	if (!pack)
	{
		if (mLastMovePacket)
			delete mLastMovePacket;
		mLastMovePacket = NULL;
		return;
	}

	if (!mLastMovePacket)
		mLastMovePacket = new Packet;
	mLastMovePacket->QuickCopy(pack);
}

// Sets the last strafe packet
void Player::SetLastStrafePacket(Packet *pack)
{
	if (!pack)
	{
		if (mLastStrafePacket)
			delete mLastStrafePacket;
		mLastStrafePacket = NULL;
		return;
	}

	if (!mLastStrafePacket)
		mLastStrafePacket = new Packet;
	mLastStrafePacket->QuickCopy(pack);
}

// Sets the last swim packet
void Player::SetLastSwimPacket(Packet *pack)
{
	if (!pack)
	{
		if (mLastSwimPacket)
			delete mLastSwimPacket;
		mLastSwimPacket = NULL;
		return;
	}

	if (!mLastSwimPacket)
		mLastSwimPacket = new Packet;
	mLastSwimPacket->QuickCopy(pack);
}

// Sets the last turn packet
void Player::SetLastTurnPacket(Packet *pack)
{
	if (!pack)
	{
		if (mLastTurnPacket)
			delete mLastTurnPacket;
		mLastTurnPacket = NULL;
		return;
	}

	if (!mLastTurnPacket)
		mLastTurnPacket = new Packet;
	mLastTurnPacket->QuickCopy(pack);
}

// Sets the last walk packet
void Player::SetLastWalkPacket(Packet *pack)
{
	if (!pack)
	{
		if (mLastWalkPacket)
			delete mLastWalkPacket;
		mLastWalkPacket = NULL;
		return;
	}

	if (!mLastWalkPacket)
		mLastWalkPacket = new Packet;
	mLastWalkPacket->QuickCopy(pack);
}

// Method: AddCreatedPlayer.
void Player::AddCreatedPlayer(Player *player)
{
	mPlayersCreated[player->GetObjectGuid()] = (Player *)player;
}

// Method: RemoveCreatedPlayer.
void Player::RemoveCreatedPlayer(Player *player)
{
	mPlayersCreated.erase(player->GetObjectGuid());
}
// Method: AddCreatedMob.
void Player::AddCreatedMob(Mob *mob)
{
	mMobsCreated[mob->GetObjectGuid()] = (Mob *)mob;
	//printf("Number of Objects in Object List: %d.\n",mMobsCreated.size());
}

// Method: RemoveCreatedMob.
void Player::RemoveCreatedMob(Mob *mob)
{
	mMobsCreated.erase(mob->GetObjectGuid());
}
/*
int Player::Save(void)
{
	Database *db;
	int result;
	char *buf;
	
	db = Database::GetSingletonPtr();
	if (!db || !*db)
		return 0;
	
	buf = new char[2048];  // should be large enough
	if (!buf)
		return 0;
	snprintf(buf, 2048, "SELECT characterId FROM %sCharacter WHERE characterId='"I64FMTD"';",
		CONFIG(Database.TablePrefix).c_str(), mObjectGuid);
	result = db->Query(buf);
	
	if (!result)
		return 0;
	if (result == -1)  // character does not yet exist in db
	{
		// generated by a short Python script
		snprintf(buf, 2048, "INSERT INTO %sCharacter VALUES ('"I64FMTD"', '%d', '%f', "
			"'%f', '%f', '%f', '%f', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', "
			"'%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', "
			"'%d', '%d', '%d', '%d');", CONFIG(Database.TablePrefix).c_str(), mObjectGuid, mAccountId, 
			mX, mY, mZ, mFacing, mScale, mName.c_str(), mHealth, mMaximumHealth, mMana, 
			mMaximumMana, mLevel, mStatusField0, mStatusField1, mStatusField2, mStatusField3, 
			mStatusField4, mBaseStatusField0, mBaseStatusField1, mBaseStatusField2, 
			mBaseStatusField3, mBaseStatusField4, mRace, mClass, mGender, mSkin, mFace, 
			mHairStyle, mFacialHair, mExperiencePoints, mRestState, mGM);
		if (db->Query(buf))
			return 1;
		return 0;
	}
	else  // character does exist in db
	{
		// generated by a short Python script
		snprintf(buf, 2048, "UPDATE %sCharacter SET x = '%f', y = '%f', z = '%f', "
			"facing = '%f', scale = '%f', name = '%s', health = '%d', maximumHealth = '%d', "
			"mana = '%d', maximumMana = '%d', level = '%d', statusField0 = '%d', "
			"statusField1 = '%d', statusField2 = '%d', statusField3 = '%d', "
			"statusField4 = '%d', baseStatusField0 = '%d', baseStatusField1 = '%d', "
			"baseStatusField2 = '%d', baseStatusField3 = '%d', baseStatusField4 = '%d', "
			"race = '%d', class = '%d', gender = '%d', skin = '%d', face = '%d', "
			"hairStyle = '%d', facialHair = '%d', experiencePoints = '%d', restState = '%d', "
			"gm = '%d' WHERE characterId='"I64FMTD"';", CONFIG(Database.TablePrefix).c_str(), 
			mX, mY, mZ, mFacing, mScale, mName.c_str(), mHealth, mMaximumHealth, mMana, mMaximumMana, 
			mLevel, mStatusField0, mStatusField1, mStatusField2, mStatusField3, mStatusField4, 
			mBaseStatusField0, mBaseStatusField1, mBaseStatusField2, mBaseStatusField3, 
			mBaseStatusField4, mRace, mClass, mGender, mSkin, mFace, mHairStyle, mFacialHair, 
			mExperiencePoints, mRestState, mGM, mObjectGuid);
		if (db->Query(buf))
			return 1;
		return 0;
	}
	delete buf;
}

int Player::Load(QuadWord guid)
{
	Database *db;
	Field *fields;
	char *buf;
	
	db = Database::GetSingletonPtr();
	if (!db || !*db)
		return 0;
	
	buf = new char[2048];  // should be large enough
	if (!buf)
		return 0;

	snprintf(buf, 2048, "SELECT * FROM %sCharacter WHERE characterId='"I64FMTD"';", 
		CONFIG(Database.TablePrefix).c_str(), guid);
	if (!(db->Query(buf) > 0))
		return 0;
	delete buf;
	
	fields = db->Fetch();
	
	// generated by a short Python script
	mAccountId = atoi(fields[1].Value());
	mX = (float)atof(fields[2].Value());
	mY = (float)atof(fields[3].Value());
	mZ = (float)atof(fields[4].Value());
	mFacing = (float)atof(fields[5].Value());
	mScale = (float)atof(fields[6].Value());
	mName = fields[7].Value();
	mHealth = atoi(fields[8].Value());
	mMaximumHealth = atoi(fields[9].Value());
	mMana = atoi(fields[10].Value());
	mMaximumMana = atoi(fields[11].Value());
	mLevel = atoi(fields[12].Value());
	mStatusField0 = atoi(fields[13].Value());
	mStatusField1 = atoi(fields[14].Value());
	mStatusField2 = atoi(fields[15].Value());
	mStatusField3 = atoi(fields[16].Value());
	mStatusField4 = atoi(fields[17].Value());
	mBaseStatusField0 = atoi(fields[18].Value());
	mBaseStatusField1 = atoi(fields[19].Value());
	mBaseStatusField2 = atoi(fields[20].Value());
	mBaseStatusField3 = atoi(fields[21].Value());
	mBaseStatusField4 = atoi(fields[22].Value());
	mRace = atoi(fields[23].Value());
	mClass = atoi(fields[24].Value());
	mGender = atoi(fields[25].Value());
	mSkin = atoi(fields[26].Value());
	mFace = atoi(fields[27].Value());
	mHairStyle = atoi(fields[28].Value());
	mFacialHair = atoi(fields[29].Value());
	mExperiencePoints = atoi(fields[30].Value());
	mRestState = atoi(fields[31].Value());
	mGM = atoi(fields[32].Value()) != 0;

	return 1;
}
*/

DoubleWord Player::GetStartZone()
{
	switch (mRace)
	{
		case RACE_HUMAN:
			return Z_HUMAN;
		case RACE_ORC:
			return Z_ORC;
		case RACE_DWARF:
			return Z_DWARF;
		case RACE_NIGHT_ELF:
			return Z_NIGHT_ELF;
		case RACE_UNDEAD:
			return Z_UNDEAD;
		case RACE_TAUREN:
			return Z_TAUREN;
		case RACE_GNOME:
			return Z_GNOME;
		case RACE_TROLL:
			return Z_TROLL;
	}

	return 0;
}

DoubleWord Player::GetStartMap()
{
	switch (mRace)
	{
		case RACE_HUMAN:
			return MAP_AZEROTH;
			break;

		case RACE_ORC:
			return MAP_KALIMDOR;
			break;

		case RACE_DWARF:
			return MAP_AZEROTH;
			break;

		case RACE_NIGHT_ELF:
			return MAP_KALIMDOR;
			break;

		case RACE_UNDEAD:
			return MAP_AZEROTH;
			break;

		case RACE_TAUREN:
			return MAP_KALIMDOR;
			break;

		case RACE_GNOME:
			return MAP_AZEROTH;
			break;

		case RACE_TROLL:
			return MAP_KALIMDOR;
			break;
	}

	return 0;
}

void Player::SetStartCoordinates(void)
{
	switch (mRace)
	{
		case RACE_HUMAN:
			SetXCoordinate(-8949.293f);
			SetYCoordinate(-132.7996f);
			SetZCoordinate(83.5923f);
			SetFacing(3.055527f); 
			break;
		case RACE_ORC:
			SetXCoordinate(-623.1572f);
			SetYCoordinate(-4269.067f);
			SetZCoordinate(37.98628f);
			SetFacing(4.845455f);
			break;
		case RACE_DWARF:
			SetXCoordinate(-6242.356f);
			SetYCoordinate(331.8532f);
			SetZCoordinate(382.7297f);
			SetFacing(2.786925f);
			break;
		case RACE_NIGHT_ELF:
			SetXCoordinate(10310.46f);
			SetYCoordinate(833.0206f);
			SetZCoordinate(1326.422f);
			SetFacing(5.481625f);
			break;
		case RACE_UNDEAD:
			SetXCoordinate(1671.826f);
			SetYCoordinate(1678.36f);
			SetZCoordinate(120.7188f);
			SetFacing(3.169415f);
			break;
		case RACE_TAUREN:
			SetXCoordinate(-2917.58f);
			SetYCoordinate(-257.98f);
			SetZCoordinate(52.9968f);
			SetFacing(4.023132f);
			break;
		case RACE_GNOME:
			SetXCoordinate(-6240.004f);
			SetYCoordinate(330.6176f);
			SetZCoordinate(382.7289f);
			SetFacing(2.760215f);
			break;
		case RACE_TROLL:
			SetXCoordinate(-620.5801f);
			SetYCoordinate(-4252.964f);
			SetZCoordinate(38.28676f);
			SetFacing(3.424336f);
			break;
	}
}

Float *Player::GetStartCoordinates(void)
{
	switch (mRace)
	{
		case RACE_HUMAN:
			mCoords[0] = (-8949.293f);
			mCoords[1] = (-132.7996f);
			return mCoords;
			break;
		case RACE_ORC:
			mCoords[0] = (-623.1572f);
			mCoords[1] = (-4269.067f);
			return mCoords;
			break;
		case RACE_DWARF:
			mCoords[0] = (-6242.356f);
			mCoords[1] = (331.8352f);
			return mCoords;
			break;
		case RACE_NIGHT_ELF:
			mCoords[0] = (10310.46f);
			mCoords[1] = (833.0206f);
			return mCoords;
			break;
		case RACE_UNDEAD:
			mCoords[0] = (1671.826f);
			mCoords[1] = (1678.36f);
			return mCoords;
			break;
		case RACE_TAUREN:
			mCoords[0] = (-2917.58f);
			mCoords[1] = (-257.98f);
			return mCoords;
			break;
		case RACE_GNOME:
			mCoords[0] = (-6240.004f);
			mCoords[1] = (330.6176f);
			return mCoords;
			break;
		case RACE_TROLL:
			mCoords[0] = (-620.5801f);
			mCoords[1] = (-4252.964f);
			return mCoords;
			break;
	}
	return NULL;
}

void Player::LevelUP()
{
	if (GetLevel() >= 100)
		return;

	SetLevel(GetLevel() + 1);

	SetExperiencePoints(0);

	DoubleWord exp = GetExperienceNextLevel();
	exp = exp + (exp - (exp * 85 / 100) + 650);
	SetExperienceNextLevel(exp);

	DoubleWord hp = GetMaximumHealth();
	hp = hp + (hp - (hp * 90 / 100 ));
	SetMaximumHealth(hp);
	SetHealth(hp);

	SetHealth(GetMaximumHealth());
}

void Player::SetInventory(Word X, QuadWord GUID)
{
	mInventory[X] = GUID;
}

QuadWord Player::GetInventory(Word X)
{
	return mInventory[X];
}

void Player::SetBackPack(Word X, QuadWord GUID)
{
	mBackPack[X] = GUID;
}

QuadWord Player::GetBackPack(Word X)
{
	return mBackPack[X];
}

void Player::SetRaceFaction()
{
	switch (mRace)
	{
		case RACE_HUMAN:
			SetFaction(ALLIANCE);
			break;

		case RACE_ORC:
			SetFaction(HORDE);
			break;

		case RACE_DWARF:
			SetFaction(ALLIANCE);
			break;

		case RACE_NIGHT_ELF:
			SetFaction(ALLIANCE);
			break;

		case RACE_UNDEAD:
			SetFaction(HORDE);
			break;

		case RACE_TAUREN:
			SetFaction(HORDE);
			break;

		case RACE_GNOME:
			SetFaction(ALLIANCE);
			break;

		case RACE_TROLL:
			SetFaction(HORDE);
			break;
	}
}

void Player::GetPlayerMovie()
{
	Packet retpack;

	if (GetMovie() == true)
	{
		SetMovie(false);
		retpack.Build(SMSG_TRIGGER_CINEMATIC);
		switch (mRace)
		{
		case RACE_HUMAN:
			retpack.AddDoubleWord(81);
			break;
		
		case RACE_ORC:
			retpack.AddDoubleWord(21);
			break;

		case RACE_DWARF:
			retpack.AddDoubleWord(41);
			break;

		case RACE_NIGHT_ELF:
			retpack.AddDoubleWord(61);
			break;

		case RACE_UNDEAD:
			retpack.AddDoubleWord(2);
			break;

		case RACE_TAUREN:
			retpack.AddDoubleWord(141);
			break;

		case RACE_GNOME:
			retpack.AddDoubleWord(101);
			break;

		case RACE_TROLL:
			retpack.AddDoubleWord(121);
			break;
		}
		WorldServer::GetSingleton().WriteData(GetClient(),&retpack);
	}
}

void Player::SetPlayerEmote(DoubleWord EmoteID)
{
	Packet retpack;

	ObjectUpdate objupd;

	switch (EmoteID)
	{
		case 0:

			SetStandState(EmoteID);

			Packets::UpdateObjectHeader(this, &retpack);
	
			objupd.Clear();
			objupd.Touch(ObjectUpdate::UPDOBJECT);
			objupd.Touch(ObjectUpdate::UPDUNIT);
			objupd.Touch(ObjectUpdate::UPDPLAYER);

			objupd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_BYTES_1,
			((DoubleWord)0xEE              << 24) |
			((DoubleWord)0x00              << 16) |
			((DoubleWord)0xEE              <<  8) |
			((DoubleWord)GetStandState()   <<  0));

			retpack.AddObjectUpdate(&objupd);
			WorldServer::GetSingleton().SendToPlayersInRange(&retpack, this);

			UnROOT();

			mPlayerEmote = 0;

			break;

		case 1:

			SetStandState(EmoteID);

			Packets::UpdateObjectHeader(this, &retpack);
	
			objupd.Clear();
			objupd.Touch(ObjectUpdate::UPDOBJECT);
			objupd.Touch(ObjectUpdate::UPDUNIT);
			objupd.Touch(ObjectUpdate::UPDPLAYER);

			objupd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_BYTES_1,
			((DoubleWord)0xEE              << 24) |
			((DoubleWord)0x00              << 16) |
			((DoubleWord)0xEE              <<  8) |
			((DoubleWord)GetStandState()   <<  0));
	
			retpack.AddObjectUpdate(&objupd);
			WorldServer::GetSingleton().SendToPlayersInRange(&retpack, this);

			ROOT();

			mPlayerEmote = 1;

			break;

		case 3:

			SetStandState(EmoteID);

			Packets::UpdateObjectHeader(this, &retpack);
	
			objupd.Clear();
			objupd.Touch(ObjectUpdate::UPDOBJECT);
			objupd.Touch(ObjectUpdate::UPDUNIT);
			objupd.Touch(ObjectUpdate::UPDPLAYER);

			objupd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_BYTES_1,
			((DoubleWord)0xEE              << 24) |
			((DoubleWord)0x00              << 16) |
			((DoubleWord)0xEE              <<  8) |
			((DoubleWord)GetStandState()   <<  0));
	
			retpack.AddObjectUpdate(&objupd);
			WorldServer::GetSingleton().SendToPlayersInRange(&retpack, this);

			ROOT();

			mPlayerEmote = 3;

			break;

	}
}

#ifdef ITEMS

DoubleWord Player::GetAttackSpeed()
{
	DoubleWord Agility = mStats.BAgility;
	Player_Item *pitem = FindPlayerItem(GetInventory(15));

	if (pitem)
	{
		Item *item = Items_Handler::GetSingleton().FindItem(pitem->Entry);
			if (item)
                Agility = (Agility * 200 - ((Agility * 60) + (item->ItemDATA.WeaponSpeed/4)));
	}
	else
		Agility = (Agility * 200 - (Agility * 60));

	if(IsGM())
		Agility = 100;

	return Agility;
}

void Player::Equip(Item *item)
{
	Packet retpack;
	ObjectUpdate objupd;

	Packets::UpdateObjectHeader(this, &retpack);

	mResistances.BPhysical = (mResistances.BPhysical + item->ItemDATA.ResistPhysical);
	mResistances.BHoly = (mResistances.BHoly + item->ItemDATA.ResistHoly);
	mResistances.BFire = (mResistances.BFire + item->ItemDATA.ResistFire);
	mResistances.BNature = (mResistances.BNature + item->ItemDATA.ResistNature);
	mResistances.BFrost = (mResistances.BFrost + item->ItemDATA.ResistFrost);
	mResistances.BShadow = (mResistances.BShadow + item->ItemDATA.ResistShadow);

	if (item->ItemDATA.ResistPhysical >= 0)
		mResistances.PPhysical += item->ItemDATA.ResistPhysical;
	else
		mResistances.NPhysical += item->ItemDATA.ResistPhysical;

	if (item->ItemDATA.ResistHoly >= 0)
		mResistances.PHoly += item->ItemDATA.ResistHoly;
	else
		mResistances.NHoly += item->ItemDATA.ResistHoly;

	if (item->ItemDATA.ResistFire >= 0)
		mResistances.PFire += item->ItemDATA.ResistFire;
	else
		mResistances.NFire += item->ItemDATA.ResistFire;

	if (item->ItemDATA.ResistNature >= 0)
		mResistances.PNature += item->ItemDATA.ResistNature;
	else
		mResistances.NNature += item->ItemDATA.ResistNature;

	if (item->ItemDATA.ResistFrost >= 0)
		mResistances.PFrost += item->ItemDATA.ResistFrost;
	else
		mResistances.NFrost += item->ItemDATA.ResistFrost;

	if (item->ItemDATA.ResistShadow >= 0)
		mResistances.PShadow += item->ItemDATA.ResistShadow;
	else
		mResistances.NShadow += item->ItemDATA.ResistShadow;

	if (item->ItemDATA.DamageStats[0].Min >= 0)
	{
		mDamages.PPhysicalMIN += item->ItemDATA.DamageStats[0].Min;
	}
	else
	{
		mDamages.NPhysicalMIN += item->ItemDATA.DamageStats[0].Min;
	}

	if (item->ItemDATA.DamageStats[0].Max >= 0)
	{
		mDamages.PPhysicalMAX += item->ItemDATA.DamageStats[0].Max;
	}
	else
	{
		mDamages.NPhysicalMAX += item->ItemDATA.DamageStats[0].Max;
	}

	objupd.Clear();
	objupd.Touch(ObjectUpdate::UPDOBJECT);
	objupd.Touch(ObjectUpdate::UPDUNIT);
	objupd.Touch(ObjectUpdate::UPDPLAYER);

	objupd.AddFieldFloat(ObjectUpdate::UPDUNIT, UNIT_FIELD_MINDAMAGE, (Float)mDamages.BPhysicalMIN);
	objupd.AddFieldFloat(ObjectUpdate::UPDUNIT, UNIT_FIELD_MAXDAMAGE, (Float)mDamages.BPhysicalMAX);

	AddStats(&objupd);

	retpack.AddObjectUpdate(&objupd);

	if (GetClient())
		WorldServer::GetSingleton().WriteData(GetClient(),&retpack);

}

void Player::UnEquip(Item *item)
{
	Packet retpack;
	ObjectUpdate objupd;

	mResistances.BPhysical = (mResistances.BPhysical - item->ItemDATA.ResistPhysical);
	mResistances.BHoly = (mResistances.BHoly - item->ItemDATA.ResistHoly);
	mResistances.BFire = (mResistances.BFire - item->ItemDATA.ResistFire);
	mResistances.BNature = (mResistances.BNature - item->ItemDATA.ResistNature);
	mResistances.BFrost = (mResistances.BFrost - item->ItemDATA.ResistFrost);
	mResistances.BShadow = (mResistances.BShadow - item->ItemDATA.ResistShadow); 

	if (item->ItemDATA.ResistPhysical >= 0)
		mResistances.PPhysical -= item->ItemDATA.ResistPhysical;
	else
		mResistances.NPhysical -= item->ItemDATA.ResistPhysical;

	if (item->ItemDATA.ResistHoly >= 0)
		mResistances.PHoly -= item->ItemDATA.ResistHoly;
	else
		mResistances.NHoly -= item->ItemDATA.ResistHoly;

	if (item->ItemDATA.ResistFire >= 0)
		mResistances.PFire -= item->ItemDATA.ResistFire;
	else
		mResistances.NFire -= item->ItemDATA.ResistFire;

	if (item->ItemDATA.ResistNature >= 0)
		mResistances.PNature -= item->ItemDATA.ResistNature;
	else
		mResistances.NNature -= item->ItemDATA.ResistNature;

	if (item->ItemDATA.ResistFrost >= 0)
		mResistances.PFrost -= item->ItemDATA.ResistFrost;
	else
		mResistances.NFrost -= item->ItemDATA.ResistFrost;

	if (item->ItemDATA.ResistShadow >= 0)
		mResistances.PShadow -= item->ItemDATA.ResistShadow;
	else
		mResistances.NShadow -= item->ItemDATA.ResistShadow;

	if (item->ItemDATA.DamageStats[0].Min >= 0)
	{
		mDamages.PPhysicalMIN -= item->ItemDATA.DamageStats[0].Min;
	}
	else
	{
		mDamages.NPhysicalMIN -= item->ItemDATA.DamageStats[0].Min;
	}

	if (item->ItemDATA.DamageStats[0].Max >= 0)
	{
		mDamages.PPhysicalMAX -= item->ItemDATA.DamageStats[0].Max;
	}
	else
	{
		mDamages.NPhysicalMAX -= item->ItemDATA.DamageStats[0].Max;
	}

	Packets::UpdateObjectHeader(this, &retpack);

	objupd.Clear();
	objupd.Touch(ObjectUpdate::UPDOBJECT);
	objupd.Touch(ObjectUpdate::UPDUNIT);
	objupd.Touch(ObjectUpdate::UPDPLAYER);

	objupd.AddFieldFloat(ObjectUpdate::UPDUNIT, UNIT_FIELD_MINDAMAGE, (Float)mDamages.BPhysicalMIN);
	objupd.AddFieldFloat(ObjectUpdate::UPDUNIT, UNIT_FIELD_MAXDAMAGE, (Float)mDamages.BPhysicalMAX);

	AddStats(&objupd);

	retpack.AddObjectUpdate(&objupd);

	if (GetClient())
		WorldServer::GetSingleton().WriteData(GetClient(),&retpack);
}

Player_Item* Player::FindPlayerItem(QuadWord GUID)
{
	list<Player_Item *>::iterator it;

	for (it = mPItem.begin(); it != mPItem.end(); it++)
	{
		if ((*it)->GUID == GUID)
		{
			return (*it);
			break;
		}
	}

	return NULL;
}

void Player::DeletePlayerItem(QuadWord GUID)
{
	list<Player_Item *>::iterator it;

	for (it = mPItem.begin(); it != mPItem.end();)
	{
		if ((*it)->GUID == GUID)
		{
			delete *it;
			mPItem.erase(it++);
			break;
		}
		else
			it++;
	}
}

DoubleWord Player::GetSlot()
{
	list<Player_Item *>::iterator it;

	DoubleWord count = 0;

	for (it = mPItem.begin(); it != mPItem.end(); it++)
	{
		if ((*it)->OwnerGuid == GetObjectGuid() && (*it)->Where == 2)
		{
			QuadWord BKP = GetBackPack(count);
	
			if (BKP == 0x00)
				break;

			count++;
		}
	}

	if (count > 15)
		return 100;

	return count;
}

void Player::CreateItems(Player *ply2, Word Mode)
{
	Packet retpack;
	ObjectUpdate objupd;
	DoubleWord count = 0;

	list<Player_Item *>::iterator it;

	for (it = mPItem.begin(); it != mPItem.end(); it++)
	{
		if ((*it)->OwnerGuid == GetObjectGuid())
		{
			if (GetClient())
			{
				Packets::NewItemHeader((*it),&retpack);
				Packets::NewItemData((*it),&retpack);

				if (Mode == 0) //Update OurSelves.
					WorldServer::GetSingleton().WriteData(GetClient(), &retpack);
				else if (Mode == 1) //Update OurSelves with other Player Items.
					WorldServer::GetSingleton().WriteData(ply2->GetClient(), &retpack);

				count++;

			}
		}
	}

	if (count > 0)
	{
		if (Mode == 0)
			Items_Handler::GetSingleton().SendInventory(GetClient(), 0, 0, 0, true, false, 0, 0, NULL);
		else if (Mode == 1)
			Items_Handler::GetSingleton().SendInventory(GetClient(), 0, 0, 0, true, false, 0, 0, ply2);
	}
		
}

void Player::NewPlayerItem(Player_Item *pitem)
{
	mPItem.push_back(pitem);
}


void Player::GetStartingItems()
{
	switch (mClass)
	{
		case CLASS_WARRIOR:
				
				CreateItem(100011);//Weapon
				CreateItem(100012);//Shield

				switch (mRace)
				{
					case RACE_HUMAN:
					case RACE_DWARF:
					case RACE_GNOME:
						CreateItem(100013);//Shirt
						CreateItem(100014);//Pants
						CreateItem(100015);//Boots
						break;
					case RACE_NIGHT_ELF:
						CreateItem(100016);//Shirt
						CreateItem(100017);//Pants
						CreateItem(100018);//Boots
						break;
					
					default:

						CreateItem(100019);//Shirt
						CreateItem(100020);//Pants
						if (mRace == RACE_ORC || mRace == RACE_UNDEAD)
							CreateItem(100021);//Boots
						break;
				}

				break;

		case CLASS_PALADIN:

				CreateItem(100022);//Weapon
				//CreateItem(100023);//Shield

				switch (mRace)
				{
					case RACE_HUMAN:
						CreateItem(100024);
						CreateItem(100025);
						CreateItem(100026);
						break;
					case RACE_DWARF:
						CreateItem(100027);
						CreateItem(100028);
						CreateItem(100029);
						break;
				}

				break;

		case CLASS_HUNTER:

				CreateItem(100030);//Weapon
				CreateItem(100031);//Shield

				switch(mRace)
				{
				case RACE_ORC:
						CreateItem(100035);//Shirt
						CreateItem(100036);//Pants
						CreateItem(100037);//Boots
					break;
				case RACE_DWARF:
						CreateItem(100032);//Shirt
						CreateItem(100033);//Pants
						CreateItem(100034);//Boots
					break;
				case RACE_NIGHT_ELF:
						CreateItem(100032);//Shirt
						CreateItem(100033);//Pants
						CreateItem(100034);//Boots
					break;
				case RACE_TAUREN:
						CreateItem(100035);//Shirt
						CreateItem(100036);//Pants
					break;
				case RACE_TROLL:
						CreateItem(100038);//Shirt
						CreateItem(100039);//Pants
					break;
				}
				
				break;

		case CLASS_ROGUE:

			CreateItem(100040);//Weapon

			switch(mRace)
			{
				case RACE_HUMAN:
				case RACE_DWARF:
				case RACE_NIGHT_ELF:
				case RACE_GNOME:
					CreateItem(100041);//Shirt
					CreateItem(100042);//Pants
					CreateItem(100043);//Legs
				break;
				case RACE_TROLL:
					CreateItem(100044);//Chest
					CreateItem(100045);//Pants
					CreateItem(100046);//Legs
				break;
			
			default:
					CreateItem(100047);//Chest
					CreateItem(100048);//Pants
					CreateItem(100049);//Legs
				break;
			}
		
			break;

		case CLASS_PRIEST:

			CreateItem(100051);//Weapon
			CreateItem(100052);//Pants
			CreateItem(100053);//Shirts
			
			if(mRace != RACE_TROLL)
				CreateItem(100050);//Boots			

			if(mRace == RACE_HUMAN || mRace == RACE_DWARF)
				CreateItem(100054);//Chest

			else if(mRace == RACE_NIGHT_ELF)
				CreateItem(100055);//Chest
			else
				CreateItem(100056);//Chest

			break;

		case CLASS_SHAMAN:
			
			CreateItem(100051);//Weapon

			switch (mRace)
			{
				case RACE_ORC:
				case RACE_TAUREN:
					
					CreateItem(100058);//Shirt
					CreateItem(100059);//Pants

					break;
				case RACE_TROLL:
		
					CreateItem(100060);//Shirt
					CreateItem(100061);//Pants
					break;
			}
			
			break;

		case CLASS_MAGE:

			CreateItem(100062);//Weapon
			CreateItem(100063);//Shirt
			CreateItem(100064);//Pants
			CreateItem(100065);//Boots

			if(mRace == RACE_HUMAN || mRace == RACE_GNOME)
				CreateItem(100066);//Chest
			else if(mRace == RACE_DWARF)
				CreateItem(100067);//Chest
			else
				CreateItem(100068);//Chest

			break;

		case CLASS_WARLOCK:

			CreateItem(100040);//Weapon
			CreateItem(100071);//Pants
			CreateItem(100072);//Boots

			if(mRace == RACE_HUMAN || mRace == RACE_GNOME)
			{
				CreateItem(100073);//Chest
				CreateItem(100074);//Shirt
			}
			else
				CreateItem(100075);//Chest

			break;

		case CLASS_DRUID:

			CreateItem(100076);//Pants

			if(mRace == RACE_NIGHT_ELF)
			{
				CreateItem(100077);//Weapon
				CreateItem(100078);//Chest
			}
			else if(mRace == RACE_TAUREN)
			{
				CreateItem(100062);//Weapon
				CreateItem(100080);//Chest
			}

			break;

	}
}

void Player::CreateItem(DoubleWord ENTRY)
{
	Item *itm = Items_Handler::GetSingleton().FindItem(ENTRY);
	if (itm)
	{
		Player_Item *pitem = new Player_Item();
		pitem->Entry = ENTRY;
		pitem->GUID = WorldServer::GetSingleton().GetNewItemGUID();
		pitem->OwnerGuid = GetObjectGuid();
		pitem->Where = 1;

		pitem->SlotNumber = Items_Handler::GetSingleton().GetStartingSlotByType(itm->ItemDATA.InvType);
		pitem->CurrentSlot = pitem->SlotNumber;

		SetInventory(pitem->SlotNumber,pitem->GUID);
		NewPlayerItem(pitem);

		Equip(itm);
	}
}
#endif
DoubleWord Player::GetMountModelID(bool flying)
{
	switch (mRace)
	{
		case RACE_HUMAN:
			if (flying)
				return 935;
			else
				return 229;
			break;
		case RACE_ORC:
			if (flying)
				return 935;
			else
				return 207;
			break;
		case RACE_DWARF:
			if (flying)
				return 935;
			else
				return 2785;
			break;
		case RACE_NIGHT_ELF:
			if (flying)
				return 935;
			else
				return 3030;
			break;
		case RACE_UNDEAD:
			if (flying)
				return 935;
			else
				return 2346;
			break;
		case RACE_GNOME:
			if (flying)	
				return 935;
			else
				return 2785;
			break;
		case RACE_TROLL:
			if (flying)
				return 935;
			else
				return 207;
			break;
	}
	
	return 0;
}

void Player::Mount()
{
	Packet retpack;

	ObjectUpdate objupd;

	Packets::UpdateObjectHeader(this,&retpack);

	objupd.Clear();
	objupd.Touch(ObjectUpdate::UPDOBJECT);
	objupd.Touch(ObjectUpdate::UPDUNIT);
	objupd.Touch(ObjectUpdate::UPDPLAYER);

	if (IsMounted())
	{
		SetMountModel(0);
		SetUnitFlags(UNIT_FLAG_SHEATHE);
		SetArmed(false);
		SetMount(false);
	}
	else
	{
		SetMountModel(GetMountModelID(GetFlying()));

		if(GetFlying())
			SetUnitFlags(0x003004);
		else
			SetUnitFlags(UNIT_FLAG_MOUNT);

		SetArmed(true);
		SetMount(true);
	}

	objupd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_FLAGS, GetUnitFlags());
	objupd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_MOUNTDISPLAYID, GetMountModel());
	retpack.AddObjectUpdate(&objupd);

	WorldServer::GetSingleton().SendToPlayersInRange(&retpack,this);

	retpack.Build(SMSG_FORCE_SPEED_CHANGE);
	retpack.AddQuadWord(GetObjectGuid());

	if (IsMounted())
		retpack.AddFloat(RUN_SPEED * 2);
	else
		retpack.AddFloat(RUN_SPEED);
	
	if (GetClient())
		WorldServer::GetSingleton().WriteData(GetClient(), &retpack);
}

#ifdef CHANNELS

void Player::ClearChannels()
{
	while(!mChannels.empty())
		mChannels.front()->RemovePlayer(this);
}

void Player::Replenish()
{
	Packet retpack;
	ObjectUpdate objupd;

	if (GetHealth() < GetMaximumHealth() && GetRegenerationStatus() == true)
	{
		SetHealth(GetHealth() + 10);

		//Sending the new Player Health to Everyone.
		Packets::UpdateObjectHeader(this, &retpack);
		
		objupd.Clear();
		objupd.Touch(ObjectUpdate::UPDOBJECT);
		objupd.Touch(ObjectUpdate::UPDUNIT);
		objupd.Touch(ObjectUpdate::UPDPLAYER);
		
		objupd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_HEALTH, GetHealth());
		
		// Unit Flags
		objupd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_FLAGS, GetUnitFlags());
		
		retpack.AddObjectUpdate(&objupd);
	
		WorldServer::GetSingleton().SendToPlayersInRange(&retpack,this);

#ifdef EVENTSYSTEM
		EventSystem::GetSingleton().AddTimer(10000,RecoverStats,(void *)this, OBJ_PLAYER, GetObjectGuid());
#endif
	} 

	else
		SetRegenerationStatus(false);
}

void Player::Recover()
{
	if (GetDeadState() == false)
	{
		if (GetHealth() < GetMaximumHealth() && GetRegenerationStatus() == false)
		{
			SetRegenerationStatus(true);
#ifdef EVENTSYSTEM
			EventSystem::GetSingleton().AddTimer(10000,RecoverStats,(void *)this, OBJ_PLAYER, GetObjectGuid());
#endif
		}
	}
}

void Player::RessurectionSickness()
{
	Packet retpack;
	ObjectUpdate objupd;
	if (GetResSickness() == true && GetHealth() < GetMaximumHealth()/2)
	{	
		SetHealth(GetHealth() + 10);
#ifdef EVENTSYSTEM
		EventSystem::GetSingleton().AddTimer(10000,ResSickness,(void *)this, OBJ_PLAYER, GetObjectGuid());
#endif
	} 
	else
	{
		SetUnitFlags(UNIT_FLAG_SHEATHE);
		SetArmed(false);
		SetGhost(false);
		SetResSickness(false);

		if(IsGM())
			SetStatus(STATUS_NORMAL | STATUS_GM);
		else
			SetStatus(STATUS_NORMAL);

		Recover();
	}

	//Sending the new Player Health to Everyone.
	Packets::UpdateObjectHeader(this, &retpack);
		
	objupd.Clear();
	objupd.Touch(ObjectUpdate::UPDOBJECT);
	objupd.Touch(ObjectUpdate::UPDUNIT);
	objupd.Touch(ObjectUpdate::UPDPLAYER);
		
	objupd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_HEALTH, GetHealth());
	// Unit Flags
	objupd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_FLAGS, GetUnitFlags());

	// 0xAABBCCDD where AA == rest state(1-5), BB == ?, CC == facialhair, DD == chatflags 1=afk,2=dnd=4=gm
	objupd.AddField(ObjectUpdate::UPDPLAYER, PLAYER_BYTES_2,
		((DoubleWord)GetRestState()       << 24) |
		((DoubleWord)0x00                      << 16) |
		((DoubleWord)GetFacialHairType()  <<  8) |
        ((DoubleWord)GetStatus()          <<  0));
		
	retpack.AddObjectUpdate(&objupd);
	
	WorldServer::GetSingleton().SendToPlayersInRange(&retpack,this);
}

void Player::Sheathe()
{
	Packet retpack;
	ObjectUpdate upd;

	if (GetArmed())
	{
		if (GetGhost())
		{
			SetUnitFlags(UNIT_FLAG_GHOST | UNIT_FLAG_SHEATHE);
			SetArmed(false);
		}
		else
		{
			SetUnitFlags(UNIT_FLAG_SHEATHE);
			SetArmed(false);
		}

	}
	else
	{
		if (GetGhost())
		{
			SetUnitFlags(UNIT_FLAG_GHOST | UNIT_FLAG_STANDART);
			SetArmed(true);
		}
		else
		{
			SetUnitFlags(UNIT_FLAG_STANDART);
			SetArmed(true);
		}
	}

		Packets::UpdateObjectHeader(this,&retpack);

		upd.Clear();
		upd.Touch(ObjectUpdate::UPDOBJECT);
		upd.Touch(ObjectUpdate::UPDUNIT);
		upd.Touch(ObjectUpdate::UPDPLAYER);
		
		// Unit Flags
		upd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_FLAGS, GetUnitFlags());
	
		retpack.AddObjectUpdate(&upd);

		WorldServer::GetSingleton().SendToPlayersInRange(&retpack, this);
}

void Player::AddStats(ObjectUpdate *objupd)
{
	//STATS
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_BASESTAT,   mStats.BStrenght);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_BASESTAT+1, mStats.BAgility);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_BASESTAT+2, mStats.BStamina);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_BASESTAT+3, mStats.BIntellect);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_BASESTAT+4, mStats.BSpirit);

	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_POSSTAT,   mStats.PStrenght);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_POSSTAT+1, mStats.PAgility);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_POSSTAT+2, mStats.PStamina);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_POSSTAT+3, mStats.PIntellect);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_POSSTAT+4, mStats.PSpirit);

	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_NEGSTAT,   mStats.NStrenght);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_NEGSTAT+1, mStats.NAgility);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_NEGSTAT+2, mStats.NStamina);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_NEGSTAT+3, mStats.NIntellect);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_NEGSTAT+4, mStats.NSpirit);

	//RESISTANCES
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCES,   mResistances.BPhysical);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCES+1, mResistances.BHoly);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCES+2, mResistances.BFire);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCES+3, mResistances.BNature);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCES+4, mResistances.BFrost);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCES+5, mResistances.BShadow);

	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE,   mResistances.PPhysical);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE+1, mResistances.PHoly);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE+2, mResistances.PFire);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE+3, mResistances.PNature);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE+4, mResistances.PFrost);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE+5, mResistances.PShadow);

	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE,   mResistances.NPhysical);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE+1, mResistances.NHoly);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE+2, mResistances.NFire);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE+3, mResistances.NNature);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE+4, mResistances.NFrost);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE+5, mResistances.NShadow);

	//ATTACK POWER
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_ATTACKPOWER, mDamages.BPhysicalMAX);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_ATTACKPOWERMODPOS, mDamages.PPhysicalMAX);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_ATTACKPOWERMODNEG, mDamages.NPhysicalMAX);
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_MOD_DAMAGE_DONE_POS, 2 * (mDamages.PPhysicalMAX + (mDamages.PPhysicalMIN / 2)));
	objupd->AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_MOD_DAMAGE_DONE_NEG, 2 * (mDamages.NPhysicalMAX + (mDamages.NPhysicalMIN / 2)));
	objupd->AddFieldFloat(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_MOD_DAMAGE_DONE_PCT, 1.0);//100% of Total Damage
}

void Player::ROOT()
{
	Packet retpack;
	retpack.Build(SMSG_FORCE_MOVE_ROOT);
	retpack.AddQuadWord(GetObjectGuid());
	WorldServer::GetSingleton().WriteData(GetClient(),&retpack);
}

void Player::UnROOT()
{
	Packet retpack;
	retpack.Build(SMSG_FORCE_MOVE_UNROOT);
	retpack.AddQuadWord(GetObjectGuid());
	WorldServer::GetSingleton().WriteData(GetClient(),&retpack);
}

void Player::GmPowers()
{
	Packet gmpack;
	ObjectUpdate upd;

	if (!GetClient())
		return;

	if(IsGM())
	{
		SetLevel(1);

		if(GetGhost())
			SetStatus(STATUS_NORMAL | STATUS_DEAD);
		else
			SetStatus(STATUS_NORMAL);

		SetHealth(100);
		SetMaximumHealth(100);
		SetMoney(10);
		SetGM(false);

		WorldServer::GetSingleton().AnnounceTo(GetClient(), "[World-Server] -> You aren't a Game Master Anymore !");
		WorldServer::GetSingleton().AnnounceToAll("[World-Server] -> %s has left the ranks of Game Masters.", GetName().c_str());
		LogManager::GetSingleton().Log("GMActions.log", "%s isnt a GM Anymore\n",	GetName().c_str());
	}
	else
	{
		SetLevel(100);
		
		if(GetGhost())
			SetStatus(STATUS_GM | STATUS_DEAD);
		else
			SetStatus(STATUS_GM);

		SetHealth(100000);
		SetMaximumHealth(100000);
		SetMoney(1000000);
		SetGM(true);

		WorldServer::GetSingleton().AnnounceTo(GetClient(), "[World-Server] -> You are now a Game Master.");
		WorldServer::GetSingleton().AnnounceToAll("[World-Server] -> %s has joined the ranks of Game Masters.", GetName().c_str());
		LogManager::GetSingleton().Log("GMActions.log", "%s became a GM\n",	GetName().c_str());
	}
	
	Packets::UpdateObjectHeader(this,&gmpack);

	upd.Clear();
	upd.Touch(ObjectUpdate::UPDOBJECT);
	upd.Touch(ObjectUpdate::UPDUNIT);
	upd.Touch(ObjectUpdate::UPDPLAYER);
	
	upd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_LEVEL, GetLevel());

	// current health
	upd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_HEALTH, GetHealth());

	// max health
	upd.AddField(ObjectUpdate::UPDUNIT, UNIT_FIELD_MAXHEALTH, GetMaximumHealth());

	//Money
	upd.AddField(ObjectUpdate::UPDPLAYER, PLAYER_FIELD_COINAGE, GetMoney());

	upd.AddField(ObjectUpdate::UPDPLAYER, PLAYER_BYTES_2, 
		((DoubleWord)GetRestState()       << 24) |
		((DoubleWord)0x00                 << 16) |
		((DoubleWord)GetFacialHairType()  <<  8) |
		((DoubleWord)GetStatus()          <<  0));

	gmpack.AddObjectUpdate(&upd);
	
	WorldServer::GetSingleton().SendToPlayersInRange(&gmpack, this);
}

#endif