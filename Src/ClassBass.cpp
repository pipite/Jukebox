//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassBass.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma comment(lib, "Bassbcb.lib")

// ---------------------------------------------------------------------------
//   ClassBass              Unité: ms
//       Durées Bass: float = second.
//       Volume Bass: float = 0.0 <= Volume <= 1.0
//   Classmix
//       Durées Mix:  nsigned __int64 = 1000 for a second.
//       Volume Mix:  float = 0.0 <= Volume <= 1.0
// ---------------------------------------------------------------------------
__fastcall XBass::XBass() : XEngine() {
	PHwnd = NULL;
}

__fastcall XBass::~XBass(void) {
	if (PHwnd != NULL) BASS_Free();
}

bool __fastcall XBass::Initialize(HWND hwnd) {
	PHwnd = hwnd;
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		MessageDlg("Bass.dll Version 2.4.15.0 needed.",	mtInformation, TMsgDlgButtons() << mbOK, 0);
		return false;
	}
	BASS_Init( -1, 44100, 0	, PHwnd, NULL );
	PChannel = NULL;
	return true;
}

bool __fastcall XBass::SetNode(XNode *node) {
	if (PChannel != NULL) BASS_StreamFree(PChannel);
	PChannel  = BASS_StreamCreateFile( false, node->Path.w_str(), 0, 0, 0 );
	return GetHaveChannel();
}

bool __fastcall XBass::GetHaveChannel(void) {
	return (PChannel == NULL);
};

bool __fastcall XBass::GetIsPlaying(void) {
	switch (BASS_ChannelIsActive(PChannel) ) {
		case BASS_ACTIVE_STOPPED :
		case BASS_ACTIVE_PAUSED  : return false;
		case BASS_ACTIVE_PLAYING : return true;
	}
	return false;
}

bool __fastcall XBass::Play(void) {
	return BASS_ChannelPlay( PChannel, false );
};

void __fastcall XBass::Stop(void) {
	if (PChannel != NULL) BASS_ChannelStop( PChannel );
};

float  __fastcall XBass::GetVolume(void) {
	float volume = 0;

	BASS_ChannelGetAttribute(PChannel,BASS_ATTRIB_VOL,&volume);
	return volume;
}

void   __fastcall XBass::SetVolume(float volume) {
	BASS_ChannelSetAttribute(PChannel,BASS_ATTRIB_VOL,volume);
}

float* __fastcall XBass::GetFFT(void) {
	BASS_ChannelGetData( PChannel, PFFT, BASS_DATA_FFT1024|BASS_DATA_FFT_NOWINDOW );
	return PFFT;
}

unsigned __int64 __fastcall XBass::GetMsLength(void) {
	unsigned __int64 l;
	l = BASS_ChannelGetLength( PChannel, BASS_POS_BYTE );
	return (unsigned __int64) ( BASS_ChannelBytes2Seconds( PChannel, l ) * 1000 );
}

unsigned __int64 __fastcall XBass::GetMsBeforeEnd(void) {
	unsigned __int64 l, p,mp,ml;
	l  = BASS_ChannelGetLength  ( PChannel, BASS_POS_BYTE );
	p  = BASS_ChannelGetPosition( PChannel, BASS_POS_BYTE );
	mp = (unsigned __int64) BASS_ChannelBytes2Seconds(PChannel,p)*1000;
	ml = (unsigned __int64) BASS_ChannelBytes2Seconds(PChannel,l)*1000;
	return ml-mp;
}

unsigned __int64 __fastcall XBass::GetPosition(void) {
	unsigned __int64 p;
	p = BASS_ChannelGetPosition( PChannel, BASS_POS_BYTE );
	return (unsigned __int64) ( BASS_ChannelBytes2Seconds( PChannel, p ) * 1000 );
}

void __fastcall XBass::SetPosition(unsigned __int64 mspos) {
	unsigned __int64 p;
	p = BASS_ChannelSeconds2Bytes(PChannel,double (mspos/1000));
	BASS_ChannelSetPosition(PChannel,p,BASS_POS_BYTE);
}


