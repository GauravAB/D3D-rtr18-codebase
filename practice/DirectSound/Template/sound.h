#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>


//some libs
#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"winmm.lib")




class SoundClass
{
private:
	//AUDIO format header
	struct WaveHeaderType
	{
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};

	
public:
	SoundClass();
	SoundClass(const SoundClass&);
	~SoundClass();

	//major functions to open , play and close the sound
	bool Initialize(HWND);
	void Shutdown();
	void trace(const char*);


private:

	bool InitializeDirectSound(HWND);
	void ShutDownDirectSound();


	bool LoadWaveFile(const char*, IDirectSoundBuffer8**);
	void ShutDownWaveFile(IDirectSoundBuffer8**);

	bool PlayWaveFile();

	IDirectSound8* m_DirectSound;
	IDirectSoundBuffer* m_primaryBuffer;

	//for 1 sound 1 secondary buffer
	IDirectSoundBuffer8* m_secondaryBuffer1;
	
	FILE* m_pFile;

};





