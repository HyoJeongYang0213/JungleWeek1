#include "SoundManager.h"


#pragma comment(lib, "xaudio2.lib")


#include <algorithm>
#include <unordered_map>
#include <cstring>
#include <fstream>

namespace
{
	bool IsFourCC(
		const char* value,
		const char* expected)
	{
		return std::memcmp(value, expected, 4) == 0;
	}

}

SoundManager::~SoundManager()
{
	shutdown();
}

// XAudio2 초기화

HRESULT SoundManager::Initialize()
{
	HRESULT hr = S_OK;

	// XAudio2 초기화
	hr = XAudio2Create(&mXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	
	if (FAILED(hr))
	{
		return hr;
	}
	
	// 마스터 보이스 생성
	hr = mXAudio2->CreateMasteringVoice(&mMasteringVoice);
	
	if (FAILED(hr))
	{
		return hr;
	}
	
	return S_OK;
}

void SoundManager::shutdown()
{
	// 모든 활성화된 보이스를 정리
	for (auto& activeVoice : mActiveVoices)
	{
		if (activeVoice.SourceVoice)
		{
			activeVoice.SourceVoice->Stop();
			activeVoice.SourceVoice->DestroyVoice();
			activeVoice.SourceVoice = nullptr;
		}
	}
	mActiveVoices.clear();
	
	// 마스터 보이스 정리
	if (mMasteringVoice)
	{
		mMasteringVoice->DestroyVoice();
		mMasteringVoice = nullptr;
	}
	// XAudio2 정리
	if (mXAudio2)
	{
		mXAudio2->Release();
		mXAudio2 = nullptr;
	}
}


HRESULT SoundManager::LoadSound(const std::filesystem::path& FilePath, const std::string& SoundName)
{
	std::shared_ptr<SoundData> soundData = std::make_shared<SoundData>();
	HRESULT hr = LoadWavFile(FilePath, soundData);
	if (FAILED(hr))
	{
		return hr;
	}
	mSoundMap[SoundName] = std::move(soundData);
	return S_OK;
}

HRESULT SoundManager::LoadWavFile(const std::filesystem::path& FilePath, std::shared_ptr<SoundData>& OutSoundData)
{
	std::ifstream file(FilePath, std::ios::binary);

	if(!file)
	{
		return HRESULT_FROM_WIN32(
			ERROR_FILE_NOT_FOUND);
	}


// ----------------------------------------
// RIFF Header
//
// RIFF
// xxxx
// WAVE
// ----------------------------------------

    char riff[4];
    uint32_t riffSize;
    char wave[4];


    file.read(riff, 4);
    file.read(
        reinterpret_cast<char*>(&riffSize),
        sizeof(riffSize));

    file.read(wave, 4);


    if (!file ||
        !IsFourCC(riff, "RIFF") ||
        !IsFourCC(wave, "WAVE"))
    {
        return HRESULT_FROM_WIN32(
            ERROR_BAD_FORMAT);
    }

    auto Data = std::make_shared<SoundData>();

    bool bFoundFormat = false;
    bool bFoundData = false;


    // RIFF Chunk 검색

    while (file && (!bFoundFormat || !bFoundData))
    {
        char chunkID[4];
        uint32_t chunkSize = 0;


        file.read(chunkID, 4);

        if (!file)
            break;


        file.read(
            reinterpret_cast<char*>(&chunkSize),
            sizeof(chunkSize));


        if (!file)
            break;


        // fmt Chunk

        if (IsFourCC(chunkID, "fmt "))
        {
            if (chunkSize < 16)
            {
                return HRESULT_FROM_WIN32(
                    ERROR_BAD_FORMAT);
            }


            if (chunkSize >
                sizeof(WAVEFORMATEXTENSIBLE))
            {
                return HRESULT_FROM_WIN32(
                    ERROR_NOT_SUPPORTED);
            }


            file.read(
                reinterpret_cast<char*>(
                    &Data->WaveFormat),
                chunkSize);


            if (!file)
            {
                return E_FAIL;
            }


            bFoundFormat = true;
        }

        
        // data Chunk

        else if (IsFourCC(chunkID, "data"))
        {
            Data->AudioBuffer.resize(
                chunkSize);


            file.read(
                reinterpret_cast<char*>(
                    Data->AudioBuffer.data()),
                chunkSize);


            if (!file)
            {
                return E_FAIL;
            }


            bFoundData = true;
        }

        // 사용하지 않는 Chunk

        else
        {
            file.seekg(
                chunkSize,
                std::ios::cur);
        }


        // RIFF chunk는 WORD alignment
        if (chunkSize & 1)
        {
            file.seekg(
                1,
                std::ios::cur);
        }
    }


    if (!bFoundFormat || !bFoundData)
    {
        return HRESULT_FROM_WIN32(
            ERROR_BAD_FORMAT);
    }


    if (Data->AudioBuffer.empty())
    {
        return HRESULT_FROM_WIN32(
            ERROR_BAD_FORMAT);
    }


    // XAUDIO2_BUFFER::AudioBytes가 UINT32
    if (Data->AudioBuffer.size() >
        (std::numeric_limits<UINT32>::max)() )
    {
        return HRESULT_FROM_WIN32(
            ERROR_FILE_TOO_LARGE);
    }


    OutSoundData = std::move(Data);


    return S_OK;
}


// Sound Play

HRESULT SoundManager::PlaySound(const std::string& SoundName, float Volume, bool Loop)
{
    auto it = mSoundMap.find(SoundName);


    if (it == mSoundMap.end())
    {
        return HRESULT_FROM_WIN32(
            ERROR_NOT_FOUND);
    }


    const std::shared_ptr<SoundData>& resource =
        it->second;


    IXAudio2SourceVoice* sourceVoice = nullptr;


    auto* format =
        reinterpret_cast<WAVEFORMATEX*>(
            &resource->WaveFormat);


    // SourceVoice 생성

    HRESULT hr =
        mXAudio2->CreateSourceVoice(
            &sourceVoice,
            format);


    if (FAILED(hr))
    {
        return hr;
    }


    // 볼륨

    Volume = std::clamp(
        Volume,
        0.0f,
        1.0f);


    hr = sourceVoice->SetVolume(Volume);


    if (FAILED(hr))
    {
        sourceVoice->DestroyVoice();
        return hr;
    }


    // Buffer
  

    XAUDIO2_BUFFER buffer{};

    buffer.AudioBytes =
        static_cast<UINT32>(
            resource->AudioBuffer.size());

    buffer.pAudioData =
        resource->AudioBuffer.data();

    buffer.Flags =
        XAUDIO2_END_OF_STREAM;

    //반복 재생이 true 일 경우
    if (Loop)
    {
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    }


    // SourceVoice에 데이터 제출
    
    hr = sourceVoice->SubmitSourceBuffer(
        &buffer);


    if (FAILED(hr))
    {
        sourceVoice->DestroyVoice();
        return hr;
    }


    // 재생

    hr = sourceVoice->Start();


    if (FAILED(hr))
    {
        sourceVoice->DestroyVoice();
        return hr;
    }


    // 현재 재생 목록에 보관

    ActiveVoice active;

    active.SourceVoice = sourceVoice;
    active.Data = resource;
    active.SoundName = SoundName;


    mActiveVoices.push_back(
        std::move(active));


    return S_OK;
}


//Sound Stop
HRESULT SoundManager::StopSound(const std::string& SoundName)
{
    bool bFound = false;

    auto it = mActiveVoices.begin();

    while (it != mActiveVoices.end())
    {
        if (it->SoundName == SoundName)
        {
            bFound = true;

            if (it->SourceVoice)
            {
                HRESULT hr = it->SourceVoice->Stop();

                if (FAILED(hr))
                {
                    return hr;
                }

                it->SourceVoice->DestroyVoice();
                it->SourceVoice = nullptr;
            }

            it = mActiveVoices.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (!bFound)
    {
        return HRESULT_FROM_WIN32(
            ERROR_NOT_FOUND);
    }

    return S_OK;
}


// 매 프레임 호출
// 끝난 SourceVoice 정리

void SoundManager::Update()
{
    auto it = mActiveVoices.begin();


    while (it != mActiveVoices.end())
    {
        XAUDIO2_VOICE_STATE state{};


        it->SourceVoice->GetState(
            &state,
            XAUDIO2_VOICE_NOSAMPLESPLAYED);


        // Buffer가 없으면 재생 종료
        if (state.BuffersQueued == 0)
        {
            it->SourceVoice->DestroyVoice();

            it = mActiveVoices.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
