#include "SoundManager.h"
#include "DxLib.h"
#include <cmath>

SoundManager::SoundManager()
	: mnNowPlayingBgm((SOUND_BGM)-1)  // 初期は何も再生されていない状態
	, mnNowPlayingSe((SOUND_SE)-1)    // 初期は何も再生されていない状態
	, mnBgmVolume(255)
{

}

SoundManager::~SoundManager()
{

}

void SoundManager::Initialize()
{
	// BGM の読み込み
	LoadBGM(SOUND_BGM::BGM_TITLE, "Resource/BGM/Title.mp3");
	LoadBGM(SOUND_BGM::BGM_STAGE1, "Resource/BGM/Stage1_BGM.mp3");
	LoadBGM(SOUND_BGM::BGM_STAGE1_BOSS, "Resource/BGM/Stage1_Boss.mp3");
	LoadBGM(SOUND_BGM::BGM_STAGE2, "Resource/BGM/Stage2_BGM.mp3");
	LoadBGM(SOUND_BGM::BGM_STAGE2_BOSS, "Resource/BGM/Stage2_Boss.mp3");
	LoadBGM(SOUND_BGM::BGM_STAGE3, "Resource/BGM/Stage_BGM.mp3");
	LoadBGM(SOUND_BGM::BGM_STAGE3_BOSS, "Resource/BGM/Stage3_Boss.mp3");
	LoadBGM(SOUND_BGM::BGM_GAMEOVER, "Resource/BGM/GameOver.mp3");
	LoadBGM(SOUND_BGM::BGM_ANGEL_BATTLE, "Resource/BGM/Angel_Boss.mp3");

	// SE の読み込み
	LoadSE(SOUND_SE::SE_PLAYER_JUMP, "Resource/SE/Player_SE/Jump.mp3");
	LoadSE(SOUND_SE::SE_PLAYER_BLINK, "Resource/SE/Player_SE/Blink.mp3");
	LoadSE(SOUND_SE::SE_PLAYER_SWORD, "Resource/SE/Player_SE/Attack.mp3");
	LoadSE(SOUND_SE::SE_PLAYER_MAGIC, "Resource/SE/Player_SE/Magic.mp3");
	LoadSE(SOUND_SE::SE_PLAYER_DEATH_FLASHBACK, "Resource/SE/Player_SE/Angel_FlashBack.mp3");
	LoadSE(SOUND_SE::SE_PLAYER_SHATTER, "Resource/SE/Player_SE/Player_Break.mp3");

	LoadSE(SOUND_SE::SE_STAGE1_HEAD_BONK, "Resource/SE/Stage1_SE/Block_Hit.mp3");
	LoadSE(SOUND_SE::SE_STAGE1_SLIME_LAND, "Resource/SE/Stage1_SE/Slime_Stamp.mp3");
	LoadSE(SOUND_SE::SE_STAGE1_LASER_CHARGE, "Resource/SE/Stage1_SE/Charging_Beam.mp3");
	LoadSE(SOUND_SE::SE_STAGE1_BEAM_FIRE, "Resource/SE/Stage1_SE/Beam_Shot.mp3");
	LoadSE(SOUND_SE::SE_STAGE1_CRACKER, "Resource/SE/Stage1_SE/Cracker.mp3");
	LoadSE(SOUND_SE::SE_STAGE1_CHEER, "Resource/SE/Stage1_SE/Praise.mp3");
	LoadSE(SOUND_SE::SE_STAGE1_LOGO_DROP, "Resource/SE/Stage1_SE/Title_Drop.mp3");
	LoadSE(SOUND_SE::SE_STAGE1_LEAF_DEATH, "Resource/SE/Stage1_SE/Leaf.mp3");
	LoadSE(SOUND_SE::SE_STAGE1_SPIKE_DEATH, "Resource/SE/Stage1_SE/Needle.mp3");

	LoadSE(SOUND_SE::SE_STAGE2_EARTHQUAKE, "Resource/SE/Stage2_SE/Earthquake.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_BOULDER_LAND, "Resource/SE/Stage2_SE/Drop_Rock.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_BOULDER_ROLL, "Resource/SE/Stage2_SE/Rock_To roll.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_BOULDER_ROLL_ALT, "Resource/SE/Stage2_SE/Rock_To roll.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_MINECART_START, "Resource/SE/Stage2_SE/Minecart_Dash_Start.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_MINECART_DASH, "Resource/SE/Stage2_SE/Minecart_Dash.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_HOTSPRING_SPLASH, "Resource/SE/Stage2_SE/HotSpring_In.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_HP_HEAL, "Resource/SE/Stage2_SE/Healing.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_BREAKABLE_WALL, "Resource/SE/Stage2_SE/Destruction_Wall.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_DRAGON_ROAR, "Resource/SE/Stage2_SE/Dragon_Roar.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_DRAGON_NORMAL_ATTACK, "Resource/SE/Stage2_SE/Dragon_Slash.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_DRAGON_BREATH, "Resource/SE/Stage2_SE/Dragon_Breath.mp3");
	LoadSE(SOUND_SE::SE_STAGE2_DRAGON_FIREBALL_LAND, "Resource/SE/Stage2_SE/Dragon_FireBall.mp3");

	/*LoadSE(SOUND_SE::SE_3D_PLAYER_COMBO1,            "Resource/SE/3DPlayer_SE/Three-stage_Attack1.mp3", 1.8f);
	LoadSE(SOUND_SE::SE_3D_PLAYER_COMBO2,            "Resource/SE/3DPlayer_SE/Three-stage_Attack2.mp3", 1.8f);
	LoadSE(SOUND_SE::SE_3D_PLAYER_COMBO3,            "Resource/SE/3DPlayer_SE/Three-stage_Attack3.mp3", 1.8f);
	LoadSE(SOUND_SE::SE_3D_PLAYER_ICE_MAGIC,         "Resource/SE/3DPlayer_SE/Ice_Magic.mp3", 2.2f);
	LoadSE(SOUND_SE::SE_3D_PLAYER_TIMESTOP,          "Resource/SE/3DPlayer_SE/Time_Stop.mp3", 3.5f);
	LoadSE(SOUND_SE::SE_3D_PLAYER_FLURRY_HIT,        "Resource/SE/3DPlayer_SE/Wild_Slash.mp3", 2.6f, 12, true);
	LoadSE(SOUND_SE::SE_3D_PLAYER_FLURRY_FINISH,     "Resource/SE/3DPlayer_SE/Vertical_Cut.mp3", 2.5f);
	LoadSE(SOUND_SE::SE_3D_PLAYER_FREEZE_ULT,        "Resource/SE/3DPlayer_SE/Freeze.mp3");
	LoadSE(SOUND_SE::SE_3D_PLAYER_FREEZE_SHATTER,    "Resource/SE/3DPlayer_SE/Freeze_Destruction.mp3");
	LoadSE(SOUND_SE::SE_3D_PLAYER_CUTIN,             "Resource/SE/3DPlayer_SE/Cut_In.mp3", 2.0f);
	LoadSE(SOUND_SE::SE_3D_ANGEL_LASER,              "Resource/SE/3DEnemy_SE/Angel_Beam.mp3", 2.4f);
	LoadSE(SOUND_SE::SE_3D_ANGEL_DASH,               "Resource/SE/3DEnemy_SE/Angel_Charge.mp3", 2.0f);
	LoadSE(SOUND_SE::SE_3D_ANGEL_THUNDER,            "Resource/SE/3DEnemy_SE/Lightning.mp3", 1.2f, 4, true);
	LoadSE(SOUND_SE::SE_3D_ANGEL_CIRCLE_SHOT,        "Resource/SE/3DEnemy_SE/Magic_Boll.mp3", 1.2f, 4, true);*/
}


void SoundManager::Finalize()
{
	// BGMの破棄
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		DeleteSoundMem(it->second);
	}

	// SEの破棄
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		DeleteSoundMem(it->second);
	}
	mnBgmHandleList.clear();
	mnSeHandleList.clear();
	mnOverlapSeList.clear();
	mnNowPlayingBgm = (SOUND_BGM)-1;
	mnNowPlayingSe = (SOUND_SE)-1;
	mnBgmVolume = 255;
}

void SoundManager::PlayBGM(SOUND_BGM bgm, bool isTop, bool isLoop)
{
	// 現在再生されているBGMと同じ、かつ、最初からの再生ではないのであればそのまま継続する
	if (mnNowPlayingBgm == bgm && !isTop)
	{
		for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
		{
			if (it->first == bgm && CheckSoundMem(it->second) == 1)
			{
				return;
			}
		}
	}

	// 別のBGMへの切り替え、または頭出し再生時は再生中のBGMをすべて停止する
	StopBGM();

	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		// 一致した種類のBGMがあれば
		if (it->first == bgm)
		{
			ChangeVolumeSoundMem(mnBgmVolume, it->second);
			const int playType = isLoop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK;
			PlaySoundMem(it->second, playType, isTop ? TRUE : FALSE);
			// 現在の再生種類を更新
			mnNowPlayingBgm = bgm;
			break;
		}
	}
}
void SoundManager::PlaySE(SOUND_SE se)
{
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		// 一致した種類のSEがあれば
		if (it->first == se)
		{
			// 重ね再生が許可されていないSEは一度止めてから再生する
			if (!IsOverlapSE(se) && CheckSoundMem(it->second) == 1)
			{
				StopSoundMem(it->second);
			}
			// SEを1回のみバックグラウンド再生する
			PlaySoundMem(it->second, DX_PLAYTYPE_BACK, TRUE);
			// 現在の再生種類を更新
			mnNowPlayingSe = se;
			break;
		}
	}
}

void SoundManager::LoadBGM(SOUND_BGM bgm, std::string filename)
{
	bool check = false; // 重複して読み込んでいるかどうか
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		if (it->first == bgm)
		{
			check = true;
			break;
		}
	}

	// 重複して読み込まれていたら何もしない
	if (check)
	{
		return;
	}

	// ファイル読み込み
	int handle = LoadSoundMem(filename.c_str());
	if (handle == -1) // -1だと失敗
	{
		return;   // 読み込み失敗したら何もしない
	}

	// 読み込んだハンドルをリストに追加
	// 読み込ん札ハンドルと、タイプ（種類）を一緒に追加

	// std::pair の一個めの変数（種類）がfirst とされている
	// pair の一つ目の要素を表していた

	// strust
	// {
	//  SOUND_BGM bgm;
	// int handle;
	// }
	// が、std::pair<SOUND_BGM, int>

	mnBgmHandleList.push_back(std::pair<SOUND_BGM, int>(bgm, handle));
}


namespace
{
	int BoostSample16SoftClip(int sample, float gain)
	{
		float x = (static_cast<float>(sample) / 32768.0f) * gain;
		float ax = fabsf(x);
		// 0.70まではリニア増幅し、それ以上はC1連続のtanhカーブで滑らかに抑えて音圧を稼ぐ
		if (ax > 0.70f)
		{
			float over = (ax - 0.70f) / 0.30f;
			float comp = 0.70f + 0.30f * tanhf(over);
			x = (x >= 0.0f) ? comp : -comp;
		}
		int out = static_cast<int>(x * 32767.0f);
		if (out > 32767) out = 32767;
		if (out < -32767) out = -32767;
		return out;
	}
}

void SoundManager::LoadSE(SOUND_SE se, std::string filename, float gain, int bufferNum, bool allowOverlap)
{
	bool check = false; // 重複して読み込んでいるかどうか
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (it->first == se)
		{
			check = true;
			break;
		}
	}

	// 重複して読み込まれていたら何もしない
	if (check)
	{
		return;
	}

	int handle = -1;
	if (gain > 1.001f)
	{
		int softHandle = LoadSoftSound(filename.c_str());
		if (softHandle != -1)
		{
			int channels = 0;
			int bitsPerSample = 0;
			int samplesPerSec = 0;
			int isFloatType = 0;
			LONGLONG sampleNum = GetSoftSoundSampleNum(softHandle);
			if (GetSoftSoundFormat(softHandle, &channels, &bitsPerSample, &samplesPerSec, &isFloatType) == 0 &&
				isFloatType == 0 && bitsPerSample == 16 && sampleNum > 0)
			{
				for (LONGLONG i = 0; i < sampleNum; ++i)
				{
					int ch1 = 0;
					int ch2 = 0;
					ReadSoftSoundData(softHandle, i, &ch1, &ch2);
					ch1 = BoostSample16SoftClip(ch1, gain);
					if (channels >= 2)
					{
						ch2 = BoostSample16SoftClip(ch2, gain);
					}
					WriteSoftSoundData(softHandle, i, ch1, ch2);
				}
				handle = LoadSoundMemFromSoftSound(softHandle, bufferNum);
			}
			DeleteSoftSound(softHandle);
		}
	}

	// 通常読み込み（またはSoftSound増幅からのフォールバック）
	if (handle == -1)
	{
		handle = LoadSoundMem(filename.c_str(), bufferNum);
	}
	if (handle == -1) // -1だと失敗
	{
		return;   // 読み込み失敗したら何もしない
	}

	mnSeHandleList.push_back(std::pair<SOUND_SE, int>(se, handle));
	if (allowOverlap)
	{
		mnOverlapSeList.push_back(se);
	}
}

void SoundManager::StopBGM()
{
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		if (CheckSoundMem(it->second))
		{
			StopSoundMem(it->second); // BGMの停止
		}
		ChangeVolumeSoundMem(255, it->second);
	}
	mnNowPlayingBgm = (SOUND_BGM)-1;
	mnBgmVolume = 255;
}

void SoundManager::StopSE(SOUND_SE se)
{
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (it->first == se)
		{
			if (CheckSoundMem(it->second))
			{
				StopSoundMem(it->second);
			}
			break;
		}
	}
}

void SoundManager::StopAllSE()
{
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (CheckSoundMem(it->second))
		{
			StopSoundMem(it->second);
		}
	}
	mnNowPlayingSe = (SOUND_SE)-1;
}

void SoundManager::SetBGMVolume(int volume)
{
	if (volume < 0) volume = 0;
	if (volume > 255) volume = 255;
	if (mnBgmVolume == volume)
	{
		return;
	}
	mnBgmVolume = volume;
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		ChangeVolumeSoundMem(mnBgmVolume, it->second);
	}
}

bool SoundManager::IsOverlapSE(SOUND_SE se) const
{
	for (auto it = mnOverlapSeList.begin(); it != mnOverlapSeList.end(); it++)
	{
		if (*it == se)
		{
			return true;
		}
	}
	return false;
}