#pragma once
// ステップデータの定義 ここを基準にCSVを作成して読み取る

#include <string>

struct StepData
{
	int id; // ステップID
	std::string name; // ステップ名前
	std::string text; // 画面に表示するテキスト

	std::string completeType; // 完了条件 (時間カウントなのかプレイヤーの行動になのか)
	float completeValue; // 完了条件の値　(Time,3なら３秒待機、Jump,5ならジャンプが5回されたら)

	int nextID; // 条件達成後に進む次のID

};
