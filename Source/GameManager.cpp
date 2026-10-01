#include "GameManager.h"
#include "Master.h"

GameManager::GameManager()
{
	mpSceneManager = std::make_unique<SceneManager>();
	mpSoundManager = std::make_unique<SoundManager>();
	mpResourceManager = std::make_unique<ResourceManager>();
	mpFontManager = std::make_unique<FontManager>();

	// 初期化
	mpFontManager->Initialize();
	// サウンドの初期化
	mpSoundManager->Initialize();
	mpSceneManager->Initialize();
}


GameManager::~GameManager()
{
	mpSoundManager->Finalize();
	mpFontManager->Finalize();
	mpSceneManager->Finalize();

	mpSoundManager.reset();
	mpFontManager.reset();
	mpResourceManager.reset();
	mpSceneManager.reset();
}


void GameManager::Update()
{
	// シーンの更新
	Master::mpGameManager->GetSceneManager()->Update();
}

SceneManager* GameManager::GetSceneManager()
{
	return mpSceneManager.get();
}

SoundManager* GameManager::GetSoundManager()
{
	return mpSoundManager.get();
}

ResourceManager* GameManager::GetResourceManager()
{
	return mpResourceManager.get();
}

FontManager* GameManager::GetFontManager()
{
	return mpFontManager.get();
}