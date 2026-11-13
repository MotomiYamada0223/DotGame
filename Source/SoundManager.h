#pragma once
#include <string>
#include <vector>

class SoundManager
{
public:   // enum, struct などの定義

	enum SOUND_BGM  // BGMの種類
	{
		BGM_TITLE = 0,     // タイトル用のBGM
		BGM_STAGE1,        // ステージ1のBGM
		BGM_STAGE1_BOSS,   // ステージ1 ボス戦のBGM
		BGM_STAGE2,        // ステージ2のBGM
		BGM_STAGE2_BOSS,   // ステージ2 ボス戦のBGM
		BGM_STAGE3,        // ステージ3のBGM
		BGM_STAGE3_BOSS,   // ステージ3 ボス戦のBGM
		BGM_GAMEOVER,      // ゲームオーバーのBGM
		BGM_ANGEL_BATTLE,  // 3Dバトル（天使戦）のBGM
	};

	enum SOUND_SE  // SEの種類
	{
		SE_DECIDE = 0,             // 決定音
		SE_PLAYER_JUMP,            // プレイヤー：ジャンプSE
		SE_PLAYER_BLINK,           // プレイヤー：ブリンクSE
		SE_PLAYER_SWORD,           // プレイヤー：剣攻撃SE
		SE_PLAYER_MAGIC,           // プレイヤー：魔法攻撃SE
		SE_PLAYER_DEATH_FLASHBACK, // プレイヤー：死亡前天使前フラッシュバックSE
		SE_PLAYER_SHATTER,         // プレイヤー：砕けるSE
		SE_STAGE1_HEAD_BONK,       // ステージ1：ブロックに頭をぶつけるSE
		SE_STAGE1_SLIME_LAND,      // ステージ1：スライム着地SE
		SE_STAGE1_LASER_CHARGE,    // ステージ1：レーザービーム溜めSE
		SE_STAGE1_BEAM_FIRE,       // ステージ1：ビーム発射SE
		SE_STAGE1_CRACKER,         // ステージ1：クラッカーSE
		SE_STAGE1_CHEER,           // ステージ1：クラッカー後の歓声SE
		SE_STAGE1_LOGO_DROP,       // ステージ1：ロゴ落下SE
		SE_STAGE1_LEAF_DEATH,          // ステージ1：葉っぱ死亡SE
		SE_STAGE1_SPIKE_DEATH,         // ステージ1：針死亡SE
		SE_STAGE2_EARTHQUAKE,          // ステージ2：地震SE
		SE_STAGE2_BOULDER_LAND,        // ステージ2：岩地面落下SE
		SE_STAGE2_BOULDER_ROLL,        // ステージ2：岩転がりSE
		SE_STAGE2_BOULDER_ROLL_ALT,    // ステージ2：岩転がりSE（別Ver）
		SE_STAGE2_MINECART_START,      // ステージ2：トロッコ動きはじめSE
		SE_STAGE2_MINECART_DASH,       // ステージ2：ダッシュトロッコSE
		SE_STAGE2_HOTSPRING_SPLASH,    // ステージ2：温泉着水SE
		SE_STAGE2_HP_HEAL,             // ステージ2：HP回復SE
		SE_STAGE2_BREAKABLE_WALL,      // ステージ2：破壊可能壁破壊SE
		SE_STAGE2_DRAGON_ROAR,         // ステージ2：ドラゴン登場咆哮SE
		SE_STAGE2_DRAGON_NORMAL_ATTACK,// ステージ2：ドラゴン通常攻撃SE
		SE_STAGE2_DRAGON_BREATH,       // ステージ2：ドラゴンブレス攻撃SE
		SE_STAGE2_DRAGON_FIREBALL_LAND,// ステージ2：ドラゴン真ん中火玉着弾SE

	};

public:  // 公開

	SoundManager();
	~SoundManager();

	// 初期化
	void Initialize();

	// 終了処理
	void Finalize();

public:   // サウンド再生系  こっちは関数

	// BGM再生
	// SOUND_BGM... 鳴らしたいBGMの種類
	// isTop... 最初から再生するかどうか
	// bool isTop = true -> デフォルト引数と呼ばれる書き方。館数を呼びだす際に渡さなくても認識される。
	void PlayBGM(SOUND_BGM bgm, bool isTop = true, bool isLoop = true);

	// SE再生
	// SEは短い音なので最初から再生のは入れなくて良い
	void PlaySE(SOUND_SE se);


	// BGMの読み込み
	void LoadBGM(SOUND_BGM bgm, std::string filename);

	// SEの読み込み
	void LoadSE(SOUND_SE se, std::string filename, float gain = 1.0f, int bufferNum = 3, bool allowOverlap = false);

	// BGM音量設定 (0～255)
	void SetBGMVolume(int volume);


	// BGMの停止
	void StopBGM();

	// SEの停止
	void StopSE(SOUND_SE se);

	// 全てのSEを停止
	void StopAllSE();


private:  // メンバ変数

	bool IsOverlapSE(SOUND_SE se) const;

	SOUND_BGM mnNowPlayingBgm;  // 現在再生されているBGMの種類
	SOUND_SE mnNowPlayingSe;  // 現在再生されているSEの種類

	int mnBgmVolume;            // 現在のBGM音量 (0～255)
	std::vector<SOUND_SE> mnOverlapSeList; // 重ね再生を許可するSE一覧

	// pair... 構造体みたいな要素？


	// よみこんだBGMハンドルのリスト
	std::vector<std::pair<SOUND_BGM, int>> mnBgmHandleList;
	// よみこんだSEハンドルのリスト
	std::vector<std::pair<SOUND_SE, int>> mnSeHandleList;

};
