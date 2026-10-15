#include "TextureAnimation.h"

TextureAnimation::TextureAnimation(
	VECTOR position,
	std::string filename,
	int allNum,
	int xNum,
	int yNum,
	int interval
)
	: mvPosition(position)
	, mnCounter(0)
	, mnInterval(interval)
	, mnCurrentNum(0)
	, mnAllNum(allNum)
	, mfScale(1.0f)
	, mbReverse(false)
	, mbLoop(true)
	, mbFinished(false)
{
	mnHandleList = new int[allNum];

	// 画像ファイル読み込み
	int handle = LoadGraph(filename.c_str());
	if (handle == -1)
	{
		return; // 読み込み失敗したら以降は処理しない
	}

	// サイズ取得
	int sizeX, sizeY;
	GetGraphSize(handle, &sizeX, &sizeY);

	// ここで mnxNumとmnyNum にサイズを渡している
	// ここで渡さないとサイズ取得ができない
	mnxNum = sizeX / xNum;
	mnyNum = sizeY / yNum;

	int success = LoadDivGraph(
		filename.c_str(),
		allNum,
		xNum,
		yNum,

		// サイズの分割数のやつをとりたいからここ調整必要
		sizeX / xNum,
		sizeY / yNum,

		mnHandleList
	);

	// ここで半径を設定（画像の半分に設定している）
	mfRadius = (float)mnxNum / 2.0f;
	
}

TextureAnimation::~TextureAnimation()
{

}


void TextureAnimation::Update()// 更新
{
	if (mbFinished) return;

	// カウンタをインクリメント
	mnCounter++;
	if (mnCounter % mnInterval == 0)
	{
		mnCounter = 0;  // カウンタを戻す
		if (mbReverse)
		{
			mnCurrentNum--; // テクスチャ番号を戻す（逆再生）
			if (mnCurrentNum < 0)
			{
				if (mbLoop)
				{
					mnCurrentNum = mnAllNum - 1;
				}
				else
				{
					mnCurrentNum = 0;
					mbFinished = true;
				}
			}
		}
		else
		{
			mnCurrentNum++; // テクスチャ番号を進める
			if (mnCurrentNum >= mnAllNum)
			{
				if (mbLoop)
				{
					mnCurrentNum = 0;  // ループさせる
				}
				else
				{
					mnCurrentNum = mnAllNum - 1;
					mbFinished = true;
				}
			}
		}
	}
}

void TextureAnimation::Draw(bool turnFlag)// 描画
{
	if (mfScale != 1.0f)
	{
		DrawRotaGraph(
			static_cast<int>(mvPosition.x),
			static_cast<int>(mvPosition.y),
			static_cast<double>(mfScale),
			0.0,
			mnHandleList[mnCurrentNum],
			TRUE,
			turnFlag ? TRUE : FALSE
		);
	}
	else
	{
		int x = static_cast<int>(mvPosition.x - (mnxNum / 2));
		int y = static_cast<int>(mvPosition.y - (mnyNum / 2));
		if (turnFlag)
		{
			DrawTurnGraph(x, y, mnHandleList[mnCurrentNum], true);
		}
		else
		{
			DrawGraph(x, y, mnHandleList[mnCurrentNum], true);
		}
	}
}