#pragma once

#include <windows.h> 
#include <xaudio2.h>

#include <string>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>


class SoundManager
{
public :
	SoundManager() = default;
	~SoundManager();

	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;

public :
	HRESULT Initialize();
	void shutdown();

	HRESULT LoadSound(const std::filesystem::path& FilePath, const std::string& SoundName);
	HRESULT PlaySound(const std::string& SoundName, float Volume = 1.0f, bool Loop = false);
	HRESULT StopSound(const std::string& SoundName);

	void Update();


private :
	struct SoundData
	{
		std::vector<BYTE> AudioBuffer;
		WAVEFORMATEXTENSIBLE  WaveFormat;
	};

	// 현재 재생 중인 음성
	struct ActiveVoice
	{
		IXAudio2SourceVoice* SourceVoice = nullptr;

		std::shared_ptr<SoundData> Data;

		std::string SoundName;

	};

private :
	HRESULT LoadWavFile(const std::filesystem::path& FilePath, std::shared_ptr<SoundData>& OutSoundData);

private :
	IXAudio2* mXAudio2 = nullptr;

	IXAudio2MasteringVoice* mMasteringVoice = nullptr;
	
	std::unordered_map<std::string, std::shared_ptr<SoundData>> mSoundMap;

	// 현재 재생 중인 ActiveVoice를 저장하는 벡터
	std::vector<ActiveVoice> mActiveVoices;

};
