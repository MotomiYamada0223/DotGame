#pragma once

#include <map>

class FontManager
{
public:
    FontManager();
    ~FontManager();

    void Initialize();
    void Finalize();

    // ドットフォントにおける文字列の描画幅(px)を取得する
    int GetDotStringWidth(int size, const char* str);

    // ドットフォントで文字列を描画する
    // size : フォントサイズ
    // color : 色
    // format : printfと同様に指定
    void DrawDotString(
        int x,
        int y,
        int size,
        unsigned int color,
        const char* format,
        ...
    );

<<<<<<< HEAD
private:
    // 指定されたサイズのドットフォントハンドルを取得
    // まだ存在しなければ新規作成
    int GetDotFontHandle(int size);

    // ドットフォントを作成
=======
    // �w�肳�ꂽ�T�C�Y�̃h�b�g�t�H���g�n���h�����擾����
    // �܂����݂��Ȃ���ΐV�����쐬����
    int GetDotFontHandle(int size);

private:
   
    // �h�b�g�t�H���g���쐬����
>>>>>>> ChangeText-ayame
    int CreateDotFont(int size);

private:
    // key   : フォントサイズ
    // value : フォントハンドル
    std::map<int, int> mDotFontHandles;
};
