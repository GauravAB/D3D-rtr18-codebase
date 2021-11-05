#pragma once
#include "sound.h"
#include <comdef.h>


void SoundClass ::trace(const char* txt)
{
	fopen_s(&m_pFile,"soundlog.txt","a+");
	fprintf_s(m_pFile, txt);
	fprintf_s(m_pFile, "\n");
	fclose(m_pFile);
}




//class constructor
SoundClass::SoundClass()
{
	m_DirectSound = NULL;
	m_primaryBuffer = NULL;
	m_secondaryBuffer1 = NULL;

	fopen_s(&m_pFile, "soundlog.txt", "w");
	fclose(m_pFile);
	
}


//copy constr
SoundClass::SoundClass(const SoundClass& other)
{

}


SoundClass::~SoundClass()
{
	fclose(m_pFile);
}


bool SoundClass::Initialize(HWND hwnd)
{
	bool result;

	//init direct sound
	result = InitializeDirectSound(hwnd);
	if (!result)
	{
		trace("InitializeDirectSound Failed");
		return false;
	}
	else
	{
		trace("InitializeDirectSound Success");
	}

	//load a wave audio file in secondary buffer
	result = LoadWaveFile("play.wav", &m_secondaryBuffer1);
	if (!result)
	{
		trace("LoadWaveFile failed");
		return false;
	}
	else
	{
		trace("LoadWaveFile Success");
	}

	result = PlayWaveFile();

	if (!result)
	{
		trace("PlayWaveFile failed");
		return false;
	}
	else
	{
		trace("PlayWaveFile Success");
	}

	return true;
}


void SoundClass :: Shutdown()
{
	//Release secondary buffer
	ShutDownWaveFile(&m_secondaryBuffer1);

	//shutDown Direct Sound
	ShutDownDirectSound();
}


//TO DO
//you can query all primary sound devices and get the primary sound buffer
bool SoundClass::InitializeDirectSound(HWND hwnd)
{
	HRESULT hr;
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;		//extended wave format


	//get the direct sound device interface
	//the default primary sound device taken for now
	//need to query here for more devices here !

	hr = DirectSoundCreate8(NULL, &m_DirectSound, NULL);

	if (FAILED(hr))
	{
		_com_error err(hr);
		MessageBox(NULL, err.ErrorMessage(), L"Direct Sound Error", MB_OK);
		return false;
	}

	//keep primary sound buffer modifiable
	hr = m_DirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);

	if (FAILED(hr))
	{
		trace("DirectSound SetCooperativeLevel failed");
		return false;
	}


	//primary buffer creation

	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	//operations of primary buffer 
	//like sound amplify , reduce etc.
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	hr = m_DirectSound->CreateSoundBuffer(&bufferDesc, &m_primaryBuffer, NULL);

	if (FAILED(hr))
	{
		trace("create sound buffer failed");
		return false;
	}





	//primary buffer on default sound device created
	//change format now 
	//these are uncompressed CD quality settings done by author

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;	//default
	waveFormat.nSamplesPerSec = 44100;	//standard
	waveFormat.wBitsPerSample = 24;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	//set primary buffer now
	hr = m_primaryBuffer->SetFormat(&waveFormat);
	if (FAILED(hr))
	{
		trace("waveFormat properties not set on primary buffer");
		return false;
	}

	return true;
}


void SoundClass::ShutDownDirectSound()
{
	//Release primary sound buffer point

	if (m_primaryBuffer)
	{
		m_primaryBuffer->Release();
		m_primaryBuffer = 0;
	}

	//release direct sound interface

	if (m_DirectSound)
	{
		m_DirectSound->Release();
		m_DirectSound = 0;
	}

}

bool SoundClass::LoadWaveFile(const char* filename, IDirectSoundBuffer8** secondaryBuffer)
{
	int error;
	FILE* fileptr;
	unsigned int count;
	WaveHeaderType	waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT hr;
	IDirectSoundBuffer* tempBuffer;
	unsigned char* waveData;
	unsigned char* bufferPtr;
	unsigned long bufferSize;


	//load header and check the format
	//if format is valid then load the audio in secondary buffer

	//open binary
	error = fopen_s(&fileptr, filename, "rb");

	if (error != 0)
	{
		trace("SoundClass:LoadWaveFile failed to open target file");
		return false;
	}
	
	//read wave file header
	count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, fileptr);
	
	if (count != 1)
	{
		trace("SoundClass::LoadWaveFile failed to read the wave file header");
		return false;
	}

	//check the chunkID is RIFF format
	if( (waveFileHeader.chunkId[0] != 'R') ||
		(waveFileHeader.chunkId[1] != 'I') ||
		(waveFileHeader.chunkId[2] != 'F') ||
		(waveFileHeader.chunkId[3] != 'F') 
		)
	{
		trace("SoundClass::LoadWaveFile chunkID not riff");
		return false;
	}
	
	
	//check the file format
	if ((waveFileHeader.format[0] != 'W') ||
		(waveFileHeader.format[1] != 'A') ||
		(waveFileHeader.format[2] != 'V') ||
		(waveFileHeader.format[3] != 'E')
		)
	{
		trace("SoundClass::LoadWaveFile format not wave");
		return false;
	}


	//check sub chunk format it should be fmt format
	//check the chuckID is RIFF format
	if ((waveFileHeader.subChunkId[0] != 'f') ||
		(waveFileHeader.subChunkId[1] != 'm') ||
		(waveFileHeader.subChunkId[2] != 't') 
		)
	{
		trace("SoundClass::LoadWaveFile subchunkID not fmt");
		return false;
	}

	//check audio format is wave format PCM
	if (waveFileHeader.audioFormat != WAVE_FORMAT_PCM)
	{
		trace("SoundClass:LoadWaveFile audio format not pcm");
		return false;
	}

	//check rec as stereo
	if (waveFileHeader.numChannels != 2)
	{
		trace("SoundClass:LoadWaveFile recording not stereo");
		return false;
	}

	//check sample rate 44.1 khz
	if (waveFileHeader.sampleRate != 44100)
	{
		trace("SoundClass:LoadWaveFile sample rate not 44100");
		return false;
	}

	//ensure rec is 16 bit
	if (waveFileHeader.bitsPerSample != 16)
	{
		trace("SoundClass:LoadWaveFile sample bits not 16");
		return false;
	}

	//check data chunk header
	if ((waveFileHeader.dataChunkId[0] != 'd') ||
		(waveFileHeader.dataChunkId[1] != 'a') ||
		(waveFileHeader.dataChunkId[2] != 't') ||
		(waveFileHeader.dataChunkId[3] != 'a')
		)
	{
		trace("SoundClass::LoadWaveFile datachunkID not data");
		return false;
	}
	

	trace("file format matched successfully");

	
	//now header file is verified
	//setup secondary buffer to load the audio
	//set bufferdesc and wave format for secondary buffer
	//some changes in flags for secondary buffer than primary buffer


	waveFormat.wFormatTag = WAVE_FORMAT_PCM;	//default
	waveFormat.nSamplesPerSec = 44100;	//standard
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;


	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	//operations of primary buffer 
	//like sound amplify , reduce etc.
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME; //only volume control
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;


	hr = m_DirectSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);

	if (FAILED(hr))
	{
		trace("SoundClass::LoadWaveFile :create secondary sound buffer failed");
		return false;
	}

	hr = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&*secondaryBuffer);
	if (FAILED(hr))
	{
		trace("SoundClass::LoadWavFile tempBuffer query interface to IID_IDirectSoundBuffer8 iterface failed");
	}
	else
	{
		trace("SoundClass::LoadWavFile tempBuffer query interface to IID_IDirectSoundBuffer8 iterface success");

	}
	tempBuffer->Release();
	tempBuffer = 0;

	//now secondary buffer is ready to be used to load the data 
	

	//move to the beginning of the wave data which starts at the end of the data chunk header.
	fseek(fileptr, sizeof(WaveHeaderType), SEEK_SET);

	//create temp buffet to hold file data
	waveData = new unsigned char[waveFileHeader.dataSize];

	if (!waveData)
	{
		trace("SoundClass:LoadWaveFile failed to create wave data temp buffer");
		return false;
	}

	//read file data into the buffer
	count = fread(waveData, 1, waveFileHeader.dataSize, fileptr);

	if (count != waveFileHeader.dataSize)
	{
		trace("SoundClass: LoadWaveFile fread failed ");
		return false;
	}
	else
	{
		trace("fread success");
	}

	//close file once done reading
	error = fclose(fileptr);

	if (error != 0)
	{
		trace("fileptr not closed");
		return false;
	}

	//lock secondary buffer to write wave data into it
	hr = (*secondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0);


	if (FAILED(hr))
	{
		trace("SoundClass: LoadWaveFile: Secondary buffer lock failed");
		return false;
	}
	else
	{
		trace("Secondary Buffer lock success");
	}
	//copy wave data into buffer now
	memcpy(bufferPtr, waveData, waveFileHeader.dataSize);

	trace("data copied in secondary buffer successfully");

	//Unlock
	hr = (*secondaryBuffer)->Unlock((void*)bufferPtr, (DWORD)bufferSize, NULL, 0);

	if (FAILED(hr))
	{
		trace("SoundClass: LoadWaveFile secondarybuffer unlock failed");
		return false;
	}

	//Release the wave data since it was copied into the secondary buffer
	delete[] waveData;
	waveData = 0;

	return true;
}


void SoundClass::ShutDownWaveFile(IDirectSoundBuffer8** secondaryBuffer)
{
	//Release the secondary buffer

	if (*secondaryBuffer)
	{
		(*secondaryBuffer)->Release();
		*secondaryBuffer = 0;
	}
}


//play wave file
//it plays the audio in secondary buffer

bool SoundClass::PlayWaveFile()
{
	HRESULT hr;

	//set position at the beginning of the sound buffer
	hr = m_secondaryBuffer1->SetCurrentPosition(0);
	if (FAILED(hr))
	{
		trace("SoundClass: PlayWaveFile set pos for secondary buffer failed");
		return false;
	}

	//set volume to 100%

	hr = m_secondaryBuffer1->SetVolume(DSBVOLUME_MAX);
	if (FAILED(hr))
	{
		trace("SoundClass: set volume failed");
		return false;
	}

	hr = m_secondaryBuffer1->Play(0, 0, DSBPLAY_LOOPING);
	if (FAILED(hr))
	{
		trace("SoundClass: failed to play");

		return false;
	}

	return true;
}

























