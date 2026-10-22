#include "Master.h"
#include <stdio.h>
#include <string.h>
#include "GameConstants.h"


// static•Ï”‚ÌÀ‘Ì‚ğ’è‹`
bool Master::gStageCleared[4] = {};
int Master::gDeathCounts[4] = {};
int Master::gCurrentStageIndex = 0;
int Master::gCurrentStageRunDeathCount = 0;
int Master::gPlayerLive = PlayerConstants::MaxLive;