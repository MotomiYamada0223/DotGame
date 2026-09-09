#include "StepLoader.h"

// ファイルを読み込むために必要なインクルード
#include <fstream>
#include <sstream>
#include <iostream>

void StepLoader::LoadCSV(const std::string& path)
{
	std::ifstream file(path);

	// ファイルが開けなかった場合
	if (!file.is_open())
	{
		std::cout << "CSVァイルが開けませんでした。" << std::endl;
		return;
	}


	std::string line;
	// 1行目は読み込む必要がない見出しなので飛ばす
	std::getline(file, line);

	// 2行目以降の読み込み
	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		std::stringstream ss(line);

		StepData data;
		std::string tmp;


        //CSVはカンマごとに読み込む
        // ID
        std::getline(ss, tmp, ',');
        data.id = std::stoi(tmp);

        // Name
        std::getline(ss, data.name, ',');

        // Text
        std::getline(ss, data.text, ',');

        // CompleteType
        std::getline(ss, data.completeType, ',');

        // CompleteValue
        std::getline(ss, tmp, ',');
        data.completeValue = std::stof(tmp);

        // NextID
        std::getline(ss, tmp, ',');
        data.nextID = std::stoi(tmp);

        // IDをキーにして保存
        data.id = data.id;
        steps[data.id] = data;
    }
}