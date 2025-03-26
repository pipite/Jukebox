//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "ThreadScanWave.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

__fastcall ThreadScanWave::ThreadScanWave(XCore *core, XNode *node, bool CreateSuspended) : TThread(CreateSuspended)
{
	FreeOnTerminate    = true;
	Core               = core;
	Core->Working      = true;
	PNode              = node->SongNode;
	PNode->MsFadeOut   = 15000;
	wave               = PNode->Wave;
	adblevel           = (int) (Core->Settings->AdbPercent * 32767 / 100);
	fadein             = -1;
	fadeout            = -1;
	PNode->WaveLoading = true;
}

//---------------------------------------------------------------------------
void __fastcall ThreadScanWave::Execute(void)
{
	if (PNode->WaveLoaded) return;
	GetBassWave();
	Synchronize( Done );
}
//---------------------------------------------------------------------------
void __fastcall ThreadScanWave::Done(void) {
	PNode->MsLength    = length * 1000;
	PNode->WaveLoaded  = true;
	PNode->WaveLoading = false;
	if ( PNode->MsLength > fadein ) PNode->MsFadeIn = fadein  * length; else PNode->MsFadeIn = 0;
	if ( PNode->MsLength > fadeout ) PNode->MsFadeOut = fadeout * length; else PNode->MsFadeOut = 0;
	Core->Working = false;
}

void __fastcall ThreadScanWave::UpdateMsFade(void) {
	PNode->MsFadeIn  = 0;
	if ( length > 2 ) PNode->MsFadeOut = (unsigned) (length*1000) - 2000; else PNode->MsFadeOut = 0;
	PNode->MsLength = length + 1000;
}

void __fastcall ThreadScanWave::GetBassWave(void) {
	DWORD mm;

	decoder = BASS_StreamCreateFile(false,PNode->Path.w_str(),0,0,BASS_STREAM_DECODE | BASS_STREAM_PRESCAN);
	length  = BASS_ChannelBytes2Seconds( decoder, BASS_ChannelGetLength(decoder,BASS_POS_BYTE) );
	UpdateMsFade();
	if (decoder != 0) {
		cpos     = 0;
		peak0    = 0;
		peak1    = 0;
		counter  = 0;
		w        = 1000;
		position = 0;

		bpp = (DWORD) BASS_ChannelSeconds2Bytes(decoder,length) / w;
		pondpeak = 0.0;
		pondcount = 0;

		while ( position < (DWORD) w ) {
			level = BASS_ChannelGetLevel(decoder);

			peak0 = LOWORD(level);
			peak1 = HIWORD(level);
			if ( BASS_ChannelIsActive(decoder) != BASS_ACTIVE_PLAYING ) {
				break;
			} else {
				if (bpp != 0) {
					position = (DWORD) BASS_ChannelGetPosition(decoder,BASS_POS_BYTE) / bpp;
				} else {
					length = 0;
					fadein = 0;
					fadeout = 0;
					return;
				}

			}
			mm = (peak0<peak1)?peak1:peak0; mm = (mm>32767)?32767:mm;
			peakmax = (peakmax<mm)?mm:peakmax; peakmax = (peakmax>32767)?32767:peakmax;
			pondpeak = pondpeak + mm;
			pondcount++;
			if ( position > cpos ) {
				counter++;
				if ( counter < w ) {
					*(wave + counter) = (unsigned) (pondpeak / pondcount);
					if ( *(wave + counter) > adblevel ) {
						if ( fadein == -1) fadein = counter;
						fadeout = counter;
					}
					pondpeak = 0;
					pondcount = 0;
				}
				cpos = position;
			}
		}
	}
	BASS_StreamFree(decoder);
	PNode->Peak = peakmax;
}
