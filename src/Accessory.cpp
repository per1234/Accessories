/*************************************************************
project: <Accessories>
author: <Thierry PARIS>
description: <Class for a generic accessory>
*************************************************************/

#include "Accessories.h"
#ifndef NO_EEPROM
#include "EEPROM.h"
#endif

Accessory::Accessory()
{
	this->pPort = NULL;
	this->movingPositionsSize = 0;
	this->movingPositionsAddCounter = 0;
	this->pMovingPositions = NULL;
	this->lastMovingPosition = 255;
	this->SetLastMoveTime();

	this->duration = 0;
	this->startingMillis = 0;
	this->useStateNone = STATE_NONE;
	this->type = ACCESSORYUNDEFINED;
	Accessories::Add(this);
}

void Accessory::AdjustMovingPositionsSize(uint8_t inNewSize)
{
	if (inNewSize <= this->movingPositionsSize)
		return;

	MovingPosition *pNewList = new MovingPosition[inNewSize];
	int i = 0;
	for (; i < this->movingPositionsSize; i++)
		pNewList[i] = this->pMovingPositions[i];

	for (; i < inNewSize; i++)
		pNewList[i].Id = UNDEFINED_ID;	//empty

	this->movingPositionsSize = inNewSize;
	if (this->pMovingPositions != NULL)
		delete[] this->pMovingPositions;
	this->pMovingPositions = pNewList;
}

// Returns the index of the new added position.
uint8_t Accessory::AddMovingPosition(unsigned long inId, int inPosition)
{
	Accessory::AdjustMovingPositionsSize(this->movingPositionsAddCounter + 1);

	this->pMovingPositions[this->movingPositionsAddCounter].Id = inId;
	this->pMovingPositions[this->movingPositionsAddCounter++].Position = inPosition;

	return this->movingPositionsAddCounter - 1;
}

uint8_t Accessory::IndexOfMovingPosition(unsigned long inId) const
{
	for (int i = 0; i < this->movingPositionsSize; i++)
		if (this->pMovingPositions[i].Id == inId)
			return i;

	return 255;
}

int Accessory::GetMovingPosition(unsigned long inId) const
{
	for (int i = 0; i < this->movingPositionsSize; i++)
		if (this->pMovingPositions[i].Id == inId)
			return this->pMovingPositions[i].Position;

	return UNDEFINED_POS;
}

void Accessory::StartAction()
{
	if (this->duration > 0)
		this->startingMillis = millis();

#ifdef ACCESSORIES_DEBUG_MODE
#ifdef ACCESSORIES_DEBUG_VERBOSE_MODE
	Serial.print(F("Accessory start action "));
	Serial.println(this->startingMillis);
#endif
#endif
}

void Accessory::StartAction(ACC_STATE inState)
{
	if (this->duration > 0)
		this->startingMillis = millis();
	this->SetState(inState);

#ifdef ACCESSORIES_DEBUG_MODE
#ifdef ACCESSORIES_DEBUG_VERBOSE_MODE
	Serial.print(F("Accessory start action at "));
	Serial.print(this->startingMillis);
	Serial.print(F("ms for state "));
	Serial.print(inState);

	if (this->startingMillis == 0)
		Serial.println(" ended");
	else
		Serial.println("");
#endif
#endif
}

#ifdef ACCESSORIES_DEBUG_MODE
void Accessory::CheckPort() const
{
	if (this->GetPort() == NULL)
	{
		Serial.println(F("One accessory have no port !"));
	}
}
#endif

#ifdef ACCESSORIES_DEBUG_VERBOSE_MODE
void Accessory::ResetAction()
{
	Serial.print(F("End (reset) action at "));
	Serial.print(millis() - this->startingMillis);
	Serial.print(F("ms for "));
	Serial.print(this->duration);
	Serial.println(F("ms"));

	this->startingMillis = 0;
}
#endif

bool Accessory::ActionEnded()
{
	if (this->startingMillis <= 0)
		return false;

	if ((unsigned long)(millis() - this->startingMillis) > this->duration)
	{
#ifdef ACCESSORIES_DEBUG_MODE
#ifdef ACCESSORIES_DEBUG_VERBOSE_MODE
		Serial.print(F("End action at "));
		Serial.print(millis() - this->startingMillis);
		Serial.print(F("ms for "));
		Serial.print(this->duration);
		Serial.println(F("ms"));
#endif
#endif
		this->startingMillis = 0;
		return true;
	}

	return false;
}

void Accessory::SetLastMovingPosition(uint8_t inLastPositionIndex)
{
	this->lastMovingPosition = inLastPositionIndex; 
#ifndef NO_EEPROM
	Accessories::EEPROMSave();
#endif
}

void Accessory::SetStateRaw(ACC_STATE inNewState)
{
	if (this->state != inNewState)
	{
		this->state = inNewState;
#ifndef NO_EEPROM
		Accessories::EEPROMSave();
#endif
	}
}

#ifndef NO_EEPROM
int Accessory::EEPROMSave(int inPos, bool inSimulate)
{
	if (!inSimulate)
	{
		EEPROM.write(inPos, this->state);
		EEPROM.write(inPos+1, this->GetLastMovingPosition());
		EEPROM.write(inPos+2, this->pPort->GetSpeed());
	}

	return inPos + 3;
}

int Accessory::EEPROMLoad(int inPos)
{
	this->state = (ACC_STATE)EEPROM.read(inPos++);
	this->SetLastMovingPosition(EEPROM.read(inPos++));
	this->pPort->SetSpeed(EEPROM.read(inPos++));

	return inPos;
}
#endif

