#pragma once

#include <string>
#include <unordered_map>
#include "StepData.h"

class StepLoader
{
public:
	// CSVファイルを読み込むための関数
	void LoadCSV(const std::string& path);

	// 読み込んだステップ
	std::unordered_map<int, StepData> steps;
};