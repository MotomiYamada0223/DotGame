#include "Master.h"
#include <stdio.h>
#include <string.h>
#include "GameConstants.h"


// static•Ï”‚ÌÀ‘Ì‚ğ’è‹`
bool Master::gStageCleared[3] = {};
int Master::gDeathCounts[3] = {};
int Master::gTotalDeathCounts[3] = {};
int Master::gOldDeathCount[3] = {};

int Master::gCurrentStageIndex = 0;
int Master::gCurrentStageRunDeathCount = 0;
int Master::gPlayerLive = PlayerConstants::MaxLive;
bool Master::gCaveOpeningCompleted = false;
int Master::gCavePostBossPhase = 0;