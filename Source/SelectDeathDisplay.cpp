#include "SelectDeathDisplay.h"
#include "Utility.h"
#include <algorithm>
#include <numeric>
#include "FontManager.h"
#include "Master.h"

namespace
{
	const int   kPadding = 16;             // 枠の内側の余白
	const int   kIconGap = 8;              // アイコン同士の間隔
	const int   kIconDrawSize = 52;        // 表示するアイコンの一辺
	const int   kFragSrcSize = 16;         // 破片の切り出し単位（元画像上のpx）
	const int   kHoldFrames = 120;          // 全員出た後、割れ始めるまでの待ち時間
	const int   kBaseSpawnInterval = 12;   // 出現間隔の基準（フレーム）
	const int   kBaseShatterInterval = 10; // 割れる間隔の基準（フレーム）
	const float kScrollLerp = 0.15f;       // スクロールの追従割合（毎フレーム）
	const int   kVerticalInset = 2;        // プレイヤー描画・可動範囲の上下を狭めるpx数

	// 死亡回数が多いほど間隔を短くする（10回で約半分、30回で約4分の1）
	int CalcInterval(int baseInterval, int count)
	{
		const int interval = baseInterval * 10 / (10 + count);
		return (std::max)(1, interval);
	}
}

// 初期状態にする
SelectDeathDisplay::SelectDeathDisplay()
{
	mEffect.SetLifeEnabled(false);
}

// 後始末（画像は借りているだけなので何もしない）
SelectDeathDisplay::~SelectDeathDisplay()
{
}

// 表示する死亡回数と枠の位置・サイズを設定し、演出を最初から始める
void SelectDeathDisplay::Setup(int deathCount, int boxX, int boxY, int boxW, int boxH)
{
	mDeathCount = (std::max)(0, deathCount);
	mBoxX = boxX;
	mBoxY = boxY;
	mBoxW = boxW;
	mBoxH = boxH;

	// 前回の破片が残っていれば消す
	mEffect.Clear();

	// 1行に並べる数と行の高さを計算する（表示サイズ基準）
	mIconsPerRow = (std::max)(1, (mBoxW - kPadding * 2 + kIconGap) / (kIconDrawSize + kIconGap));
	mRowHeight = kIconDrawSize + kIconGap;

	// 行が枠に収まらないときだけスクロールできるようにする
	const int totalRows = (mDeathCount + mIconsPerRow - 1) / mIconsPerRow;
	const float contentH = static_cast<float>(totalRows * mRowHeight - kIconGap);
	const float viewH = static_cast<float>(mBoxH - kPadding * 2);
	mMaxScrollY = (std::max)(0.0f, contentH - viewH);

	// スクロールと進行状況を初期化する
	mScrollY = 0.0f;
	mScrollTarget = 0.0f;
	mSpawnedCount = 0;
	mShatterIndex = 0;
	mTimer = 0;

	// 割れた状態を初期化する
	mShattered.assign(mDeathCount, false);

	// 右側の列から順に割る（同じ列では上の行から）
	mShatterOrder.resize(mDeathCount);
	std::iota(mShatterOrder.begin(), mShatterOrder.end(), 0);
	std::sort(mShatterOrder.begin(), mShatterOrder.end(), [this](int a, int b)
		{
			const int colA = ColOf(a);
			const int colB = ColOf(b);
			if (colA != colB) return colA > colB;
			return RowOf(a) < RowOf(b);
		});

	// 0回なら何も表示せず終了扱いにする
	mState = (mDeathCount > 0) ? State::Appear : State::Done;
}

// 使用する画像ハンドルと1コマの切り出し範囲を設定する（画像の所有はしない）
void SelectDeathDisplay::SetTexture(int graphHandle, int srcX, int srcY, int srcW, int srcH)
{
	mTextureHandle = graphHandle;
	mSrcX = srcX;
	mSrcY = srcY;
	mSrcW = srcW;
	mSrcH = srcH;
}

// 出現・割れる演出・スクロールを更新する
void SelectDeathDisplay::Update()
{
	switch (mState)
	{
	case State::Appear:
		// 左上から順に1つずつ出現させ、必要なら枠をスクロールさせる
		++mTimer;
		if (mTimer >= CalcInterval(kBaseSpawnInterval, mDeathCount))
		{
			mTimer = 0;
			ScrollToRow(RowOf(mSpawnedCount));
			++mSpawnedCount;

			if (mSpawnedCount >= mDeathCount)
			{
				mState = State::Hold;
				mTimer = 0;
			}
		}
		break;

	case State::Hold:
		// 全員出た後、少し待ってから割れ始める
		++mTimer;
		if (mTimer >= kHoldFrames)
		{
			mState = State::Shatter;
			mTimer = 0;

			// 最初の割る対象行へ早めにスクロールを開始し、枠外生成を防ぐ
			if (mShatterIndex < mDeathCount)
			{
				ScrollToRow(RowOf(mShatterOrder[mShatterIndex]));
			}
		}
		break;

	case State::Shatter:
		// 右側の列から順に割る（必要なら割るアイコンが見えるようスクロールする）
		++mTimer;
		if (mTimer >= CalcInterval(kBaseShatterInterval, mDeathCount))
		{
			mTimer = 0;
			const int iconIndex = mShatterOrder[mShatterIndex];
			ScrollToRow(RowOf(iconIndex));
			ShatterIcon(iconIndex);
			++mShatterIndex;

			if (mShatterIndex >= mDeathCount)
			{
				mState = State::Done;
			}
		}
		break;

	default:
		break;
	}

	// 破片の座標計算前にスクロール量を反映し、初期数体のズレを抑える
	mScrollY += (mScrollTarget - mScrollY) * kScrollLerp;

	mEffect.Update([this](float x, float y, float w, float h)
		{
			// 上下を2pxずつ狭めた枠の範囲で左右と下端を判定し、上端の突っかかりを防ぐ
			const float left = static_cast<float>(mBoxX + kPadding);
			const float right = static_cast<float>(mBoxX + mBoxW - kPadding);
			const float bottom = static_cast<float>(mBoxY + mBoxH - kPadding - kVerticalInset);

			return x < left || (x + w) > right || (y + h) > bottom;
		});
}

// 枠の中だけに描画する（0回のときは「死亡回数なし」を中央に表示）
void SelectDeathDisplay::Draw()
{
	RECT oldArea;
	GetDrawArea(&oldArea);
	SetDrawArea(mBoxX, mBoxY, mBoxX + mBoxW, mBoxY + mBoxH);

	if (mDeathCount <= 0)
	{
		// 死亡0回：枠の中央に「死亡回数なし」を表示する
		FontManager* fontManager = Master::mpGameManager->GetFontManager();
		const char* text = "死亡回数なし";
		const int fontSize = 36;
		const int textWidth = fontManager->GetDotStringWidth(fontSize, text);
		const int x = mBoxX + (mBoxW - textWidth) / 2;
		const int y = mBoxY + (mBoxH - fontSize) / 2;
		fontManager->DrawDotString(x, y, fontSize, GetColor(255, 255, 255), text);
	}
	else
	{
		// プレイヤー表示領域の上下を2pxずつ狭めてクリップする
		const int drawTop = mBoxY + kVerticalInset;
		const int drawBottom = mBoxY + mBoxH - kVerticalInset;
		SetDrawArea(mBoxX, drawTop, mBoxX + mBoxW, drawBottom);

		if (mTextureHandle != -1)
		{
			for (int i = 0; i < mSpawnedCount; ++i)
			{
				if (mShattered[i]) continue;

				const int left = static_cast<int>(IconScreenLeft(i));
				const int top = static_cast<int>(IconScreenTop(i));

				// 1コマ分の切り出し範囲を、表示サイズに縮小して描画する
				DrawRectExtendGraph(
					left,
					top,
					left + kIconDrawSize,
					top + kIconDrawSize,
					mSrcX, mSrcY, mSrcW, mSrcH,
					mTextureHandle,
					TRUE);
			}
		}
		mEffect.Draw();
	}

	SetDrawArea(oldArea.left, oldArea.top, oldArea.right, oldArea.bottom);
}

// 演出が全て終わったかどうか
bool SelectDeathDisplay::IsFinished() const
{
	return mState == State::Done && mEffect.IsFinished();
}

// アイコンの列番号
int SelectDeathDisplay::ColOf(int idx) const
{
	return idx % mIconsPerRow;
}

// アイコンの行番号
int SelectDeathDisplay::RowOf(int idx) const
{
	return idx / mIconsPerRow;
}

// アイコンの画面上の左端X
float SelectDeathDisplay::IconScreenLeft(int idx) const
{
	return static_cast<float>(mBoxX + kPadding + ColOf(idx) * (kIconDrawSize + kIconGap));
}

// アイコンの画面上の上端Y（スクロール反映）
float SelectDeathDisplay::IconScreenTop(int idx) const
{
	return static_cast<float>(mBoxY + kPadding + RowOf(idx) * mRowHeight) - mScrollY;
}

// 指定の行が枠内に見えるよう目標スクロールを設定する
void SelectDeathDisplay::ScrollToRow(int row)
{
	const float rowTop = static_cast<float>(row * mRowHeight);
	const float rowBottom = rowTop + static_cast<float>(kIconDrawSize);
	const float viewH = static_cast<float>(mBoxH - kPadding * 2);

	// 行が上に隠れていれば上端に合わせ、下に隠れていれば下端に合わせる
	if (rowTop < mScrollTarget)
	{
		mScrollTarget = rowTop;
	}
	else if (rowBottom > mScrollTarget + viewH)
	{
		mScrollTarget = rowBottom - viewH;
	}

	// スクロール範囲を超えないように制限する
	mScrollTarget = (std::max)(0.0f, (std::min)(mScrollTarget, mMaxScrollY));
}

// 指定のアイコンを破片にする（破片の生成は DeathCountEffect に任せる）
void SelectDeathDisplay::ShatterIcon(int idx)
{
	if (mShattered[idx]) return;
	mShattered[idx] = true;

	DeathCountEffect::PlayerSpriteSource spriteSource{ mTextureHandle, mSrcX, mSrcY, mSrcW, mSrcH };

	// 元画像の幅に対する表示サイズの倍率（128px → 48px なら 0.375）
	const float scale = static_cast<float>(kIconDrawSize) / static_cast<float>((std::max)(1, mSrcW));

	// 表示上のアイコンの中心から破片を飛び散らせる
	const float centerX = IconScreenLeft(idx) + kIconDrawSize / 2.0f;
	const float centerY = IconScreenTop(idx) + kIconDrawSize / 2.0f;
	mEffect.AppendShatter(spriteSource, centerX, centerY, kFragSrcSize, scale);
}