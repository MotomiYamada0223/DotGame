#include "SoundManager.h"
#include "DxLib.h"
#include "Master.h"
#include <fstream>
#include "GameConstants.h"

SoundManager::SoundManager()
	: mnNowPlayingBgm((SOUND_BGM)-1)
	, mnNowPlayingSe((SOUND_SE)-1)
	, mnBgmVolume(SoundSetting::VolumeMax)
	, mnSeVolume(SoundSetting::VolumeMax)
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
	LoadSE(SOUND_SE::SE_DECIDE, "Resource/SE/System/Dicide.mp3");

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

	LoadVolume();
}

// 毎フレーム呼ばれる更新処理
void SoundManager::Update()
{
	// 登録されている遅延SEを全部チェック
	for (auto& s : seQueue)
	{
		s.timer--; // カウントダウン

		// タイマーが0以下になったら再生
		if (s.timer <= 0)
		{
			PlaySE((SOUND_SE)s.se); // SEを再生
		}
	}

	// 再生し終わったものを削除する
	seQueue.erase(
		std::remove_if(seQueue.begin(), seQueue.end(),
			[](DelayedSE& s)
			{
				return s.timer <= 0; // タイマー終わったやつ消す
			}),
		seQueue.end()
	);

	if (mbFadeOut)
	{
		mnFadeVolume -= 1;

		if (mnFadeVolume < 0)
			mnFadeVolume = 0;

		ChangeVolume(mnFadeVolume);

		if (mnFadeVolume == 0)
		{
			StopBGM();
			mbFadeOut = false;
		}
	}
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
}


void SoundManager::PlayBGM(SOUND_BGM bgm, bool isTop, int volume)
{
	mnFadeVolume = mnBgmVolume;
	mbFadeOut = false;

	// 現在再生されているBGMと同じかつ
	// 最初からの再生ではないのであれば return する
	if (mnNowPlayingBgm == bgm && !isTop)
	{
		return;
	}

	//  現在再生中のBGMを止める
	// 今流れているBGMを探して停メル
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		if (it->first == mnNowPlayingBgm)
		{
			if (CheckSoundMem(it->second))
			{
				StopSoundMem(it->second);
			}
			break;
		}
	}

	// 新しいBGMのハンドルを取得して再生
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		if (it->first == bgm)
		{
			// 再生する前に音量を設定する
			ChangeVolumeSoundMem(volume * mnBgmVolume / 255, it->second);
			//ChangeVolumeSoundMem(mnBgmVolume * 255 / 100, it->second);
			// BGMをループ再生
			PlaySoundMem(it->second, DX_PLAYTYPE_LOOP, TRUE); // ループ再生
			// 現在の再生種類を更新
			mnNowPlayingBgm = bgm;
		}
	}
}


// 音量変えたい
void SoundManager::ChangeVolume(int volume)
{
	// 何も再生していないなら何もしない
	if (mnNowPlayingBgm == BgmNone) return;

	// すでに再生中のBGMがあるか確認
	int currentHandle = -1;
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		if (it->first == mnNowPlayingBgm)
		{
			currentHandle = it->second;
			break;
		}
	}

	// 見つかったら音量を変更
	if (currentHandle != -1)
	{
		ChangeVolumeSoundMem(volume, currentHandle);
	}
}

// 音量変えたいSE追加した
void SoundManager::Change_SE_Volume(SOUND_SE se, int volume)
{
	// 指定されたSE探し
	int targetHandle = -1;
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (it->first == se)
		{
			targetHandle = it->second;
			break;
		}
	}

	// 見つかったらその SE の音量だけを変更
	if (targetHandle != -1)
	{
		ChangeVolumeSoundMem(volume, targetHandle);
	}
}


// 指定したSEを個別に止める関数
void SoundManager::StopSE(SOUND_SE se)
{
	// SEのリストから 止めたいSEを探す
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (it->first == se)
		{
			// 再生中かどうか確認して止める
			if (CheckSoundMem(it->second))
			{
				StopSoundMem(it->second);
			}
			break;
		}
	}
}


void SoundManager::PlaySE(SOUND_SE se, int volume)
{
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (it->first == se)
		{
			// 個別音量(0~100) × 全体SE音量(0~100) を DxLib の 0 255 に変換
			int finalVolume = volume * mnSeVolume * 255 / (100 * 100);
			ChangeVolumeSoundMem(finalVolume, it->second);

			PlaySoundMem(it->second, DX_PLAYTYPE_BACK);
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
	//  int handle;
	// }
	// が、std::pair<SOUND_BGM, int>

	mnBgmHandleList.push_back(std::pair<SOUND_BGM, int>(bgm, handle));
}


void SoundManager::LoadSE(SOUND_SE se, std::string filename)
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

	// ファイル読み込み
	int handle = LoadSoundMem(filename.c_str());
	if (handle == -1) // -1だと失敗
	{
		return;   // 読み込み失敗したら何もしない
	}

	mnSeHandleList.push_back(std::pair<SOUND_SE, int>(se, handle));
}


void SoundManager::StopBGM()
{
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		// 今現在再生しているBGMと比較させる
		if (it->first == mnNowPlayingBgm)
		{
			// BGMが再生されているかのif文
			//          もし再生されていたら
			if (CheckSoundMem(it->second))
			{
				StopSoundMem(it->second); // BGMの停止
				break;
			}
		}
	}

	// チュートリアルから戻ったときにBGMが再生されないから
	mnNowPlayingBgm = BgmNone;
}

void SoundManager::PlaySE_Loop(SOUND_SE se, int volume)
{
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (it->first == se)
		{
			// すでに鳴っていたら二重に鳴らさない
			if (CheckSoundMem(it->second))
			{
				return;
			}

			// ボリューム
			ChangeVolumeSoundMem(volume, it->second);
			PlaySoundMem(it->second, DX_PLAYTYPE_LOOP);
			break;
		}
	}
}

// 菊池
// 遅延してSEを再生する
void SoundManager::PlaySEDelay(SOUND_SE se, int delay)
{
	// 「このSEを〇フレーム後に鳴らす」という予約を追加
	seQueue.push_back({ se, delay });
}


void SoundManager::StartFadeOut()
{
	if (mbFadeOut) return;

	mbFadeOut = true;
}

// BGMの音量変えるやつ
void SoundManager::SetBGMVolume(int volume)
{
	if (volume < SoundSetting::VolumeMin)
	{
		volume = SoundSetting::VolumeMin;
	}
	if (volume > SoundSetting::VolumeMax)
	{
		volume = SoundSetting::VolumeMax;
	}

	mnBgmVolume = volume;

	for (auto& bgm : mnBgmHandleList)
	{
		if (bgm.first == mnNowPlayingBgm)
		{
			ChangeVolumeSoundMem(
				mnBgmVolume,
				bgm.second
			);
		}
	}
}

// SEの音量を変えるやつ
void SoundManager::SetSEVolume(int volume)
{
	if (volume < SoundSetting::VolumeMin)
	{
		volume = SoundSetting::VolumeMin;
	}

	if (volume > SoundSetting::VolumeMax)
	{
		volume = SoundSetting::VolumeMax;
	}

	mnSeVolume = volume;
}
// ボリュームのロード
void SoundManager::SaveVolume()
{
	std::ofstream ofs("Volume.txt");

	if (ofs)
	{
		ofs << "BGM=" << mnBgmVolume << std::endl;
		ofs << "SE=" << mnSeVolume << std::endl;
	}
}


void SoundManager::LoadVolume()
{
	// デフォルト値
	mnBgmVolume = SoundSetting::VolumeMax;
	mnSeVolume = SoundSetting::VolumeMax;

	std::ifstream ifs("Volume.txt");

	// ファイルが存在しない場合
	if (!ifs)
	{
		SaveVolume();
		return;
	}

	std::string line;

	// BGM
	if (std::getline(ifs, line))
	{
		size_t pos = line.find('=');

		if (pos != std::string::npos)
		{
			try
			{
				int volume = std::stoi(line.substr(pos + 1));

				if (volume >= SoundSetting::VolumeMin &&
					volume <= SoundSetting::VolumeMax)
				{
					mnBgmVolume = volume;
				}
			}
			catch (...)
			{
				// 数値として読めなかったらデフォルト100のまま
			}
		}
	}

	// SE
	if (std::getline(ifs, line))
	{
		size_t pos = line.find('=');

		if (pos != std::string::npos)
		{
			try
			{
				int volume = std::stoi(line.substr(pos + 1));

				// 0～100なら採用
				if (volume >= SoundSetting::VolumeMin && volume <= SoundSetting::VolumeMax)
				{
					mnSeVolume = volume;
				}
			}
			catch (...)
			{
				// 数値として読めなかったらデフォルト100のまま
			}
		}
	}
}



// ボリュームのセーブ
int SoundManager::GetBGMVolume()
{
	return mnBgmVolume;
}

// SEのセーブ
int SoundManager::GetSEVolume()
{
	return mnSeVolume;
}


