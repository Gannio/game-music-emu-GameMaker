
// Buffer size can be any multiple of 2
#define BUFFER_SIZE 760 // This was 8192 in version 1.0

// Defines and includes
#if defined(_WIN32)
# define WIN32_LEAN_AND_MEAN
# define EXPORT extern "C" __declspec ( dllexport )
# include <windows.h>
#else
# define EXPORT extern "C"
#endif

#include "Music_Emu.h"
#include "helpers.h" // Currently only a string to unsigned integer conversion function

// Empty string
char* _undefined = "";

// The global music emu and info objects
Music_Emu* emu;
track_info_t tinfo = { 0 };

// Create the DllMain function if building for Windows
// ---------------------------------------------------------------------------------
#if defined(_WIN32)
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

// Load a song from file
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_LoadFile(char* fileName)
{
	//&tinfo = NULL; // Clear pointer to info struct

	gme_err_t error;
	gme_type_t file_type;

	error = gme_identify_file(fileName, &file_type);
	if (error)
		return 2; // File identify error
	else {
		emu = file_type->new_emu();
		if (!emu)
			return 3; // Could not create player (out of memory?)
		else {
			error = emu->set_sample_rate(44100);
			if (error)
				return 4; // Could not set samplerate
			else {
				error = emu->load_file(fileName);
				if (error)
					return 5; // Could not load file
				else
					return 1; // OK
			}
		}
	}
}

// Load a song from data already in memory (previously created buffer in GMS)
// Data address pointer supplied as a real pointer
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_LoadBuffer(double size, const void* data_address)
{
	gme_err_t error = gme_open_data(data_address, (long)size, &emu, 44100);
	if (error == NULL)
		return 1; // OK
	else
		return 0; // Load Failed
}

// Load a song from data already in memory (previously created buffer in GMS)
// Data address supplied as a string (char*)
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_LoadBufferStr(double size, char* data_address)
{
	unsigned long addr = hex2uint(data_address);
	return GameMusicEmu_LoadBuffer(size, (const void*)addr);
}

// Get the number of tracks in the loaded file
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_NumTracks()
{
	if (emu)
		return (double)emu->track_count();
	else
		return 0;
}

// Initialize a track, this function needs to be called before playback can start
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_StartTrack(double trackNumber)
{
	blargg_err_t error;

	int track_nr = (int)trackNumber;
	if (track_nr >= emu->track_count())
		track_nr = emu->track_count() - 1;
	error = emu->start_track(track_nr);
	if (error) {
		//tinfo = NULL;
		return 0; // Could not set track number
	}
	else {
		emu->track_info(&tinfo);
		return 1; // OK
	}
}

// Read song data into a memory buffer
// Buffer address supplied as a real pointer
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_Read(char* buffer_address)
{
	emu->play(BUFFER_SIZE, (short*)buffer_address);
	if (emu->track_ended())
		return 0; // Track ended
	else
		return 1;
}

// Read song data into a memory buffer
// Buffer address supplied as a string (char*)
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_ReadStr(char* buffer_address)
{
	unsigned long addr = hex2uint(buffer_address);
	emu->play(BUFFER_SIZE, (short*)addr);
	if (emu->track_ended())
		return 0; // Track ended
	else
		return 1;
}

// Get the number of voices used in the current song
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_NumVoices()
{
	if (emu)
		return (double)emu->voice_count();
	else
		return 0;
}

// Mute / unmute a voice
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_MuteVoice(double index, double mute)
{
	bool m = false;
	if (mute > 0)
		m = true;

	if (emu) {
		emu->mute_voice((int)index, m);
		return 0;
	}
	else
		return 1;
}

// Mute / unmute voices according to a bitmask
// Ex: mask = 5 = B00000101 will mute voices 0 and 2
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_MuteVoices(double mask)
{
	int m = (int)mask;

	if (emu) {
		emu->mute_voices(m);
		return 1;
	}
	else
		return 0;
}

// Set tempo
// Ex: 1 = normal speed, 1.5 = 150% speed, 2 = 200% speed
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_SetTempo(double tempo)
{
	if (tempo > 0 && tempo <= 5)
		emu->set_tempo(tempo);
	return 1;
}

/*EXPORT double GameMusicEmu_GetTrackLength()
{
	if(emu->
}*/

// Get Position in milliseconds
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_GetPosition()
{
	if (emu) {
		return (double)emu->tell();
	}
	else
		return 0;
}

// Set Position in milliseconds
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_SetPosition(double msec)
{
	if (emu) {
		if (emu->seek((int)msec) == NULL)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

// Get track length in milliseconds
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_GetTrackLength()
{
	if (emu)
		return (double)tinfo.length;
	else
		return -1;
}

// Get Song name
// ---------------------------------------------------------------------------------
EXPORT char* GameMusicEmu_GetName()
{
	if (emu)
		return tinfo.song;
	else
		return _undefined;
}

// Get Song author
// ---------------------------------------------------------------------------------
EXPORT char* GameMusicEmu_GetAuthor()
{
	if (emu)
		return tinfo.author;
	else
		return _undefined;
}

// Get Song copyright
// ---------------------------------------------------------------------------------
EXPORT char* GameMusicEmu_GetCopyright()
{
	if (emu)
		return tinfo.copyright;
	else
		return _undefined;
}

// Get Song comment
// ---------------------------------------------------------------------------------
EXPORT char* GameMusicEmu_GetComment()
{
	if (emu)
		return tinfo.comment;
	else
		return _undefined;
}

// Free the memory associated with the music emu object
// ---------------------------------------------------------------------------------
EXPORT double GameMusicEmu_Free()
{
	if (emu) {
		delete emu;
		return 1;
	}
	else
		return 0;
}