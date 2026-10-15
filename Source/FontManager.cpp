#include "FontManager.h"

#include <DxLib.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
}

void FontManager::Initialize()
{
    // Resourceにあるフォントを読み込んで使用できるようにする
    AddFontResourceEx(
        "Resource/Fonts/dotFont/maruMonica.ttf",
        FR_PRIVATE,
        NULL
    );
}

void FontManager::Finalize()
{
    // 作成したフォントをすべて削除
    for (auto& font : mDotFontHandles)
    {
        DeleteFontToHandle(font.second);
    }

    mDotFontHandles.clear();
}

int FontManager::GetDotFontHandle(int size)
{
    // すでに同じサイズのフォントを持っているか確認
    auto it = mDotFontHandles.find(size);

    if (it != mDotFontHandles.end())
    {
        // すでに存在するので、そのハンドルを返す
        return it->second;
    }

    // まだ存在しないので新規作成
    int handle = CreateDotFont(size);

    if (handle == -1)
    {
        return -1;
    }

    // サイズとフォントハンドルを保存
    mDotFontHandles[size] = handle;

    return handle;
}

int FontManager::CreateDotFont(int size)
{
    int handle = CreateFontToHandle(
        "x12y16pxMaruMonica",
        size,
        5
    );

    // フォントの作成に失敗した場合
    if (handle == -1)
    {
        printfDx("ドットフォントの作成に失敗しました。\n");

        // 標準フォントで代用
        handle = CreateFontToHandle(
            NULL,
            size,
            5
        );
    }

    return handle;
}

int FontManager::GetDotStringWidth(int size, const char* str)
{
    int fontHandle = GetDotFontHandle(size);

    if (fontHandle == -1 || str == nullptr)
    {
        return 0;
    }

    return GetDrawStringWidthToHandle(str, static_cast<int>(strlen(str)), fontHandle);
}

void FontManager::DrawDotString(
    int x,
    int y,
    int size,
    unsigned int color,
    const char* format,
    ...
    )
{
    // 指定されたサイズのフォントを取得
    int fontHandle = GetDotFontHandle(size);

    if (fontHandle == -1)
    {
        return;
    }

    // printf形式で文字列を作成
    char buffer[1024];

    va_list args;
    va_start(args, format);

    vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args
    );

    va_end(args);

    // 作成した文字列を描画
    DrawStringToHandle(
        x,
        y,
        buffer,
        color,
        fontHandle
    );
}
