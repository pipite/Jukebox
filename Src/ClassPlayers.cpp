
#pragma hdrstop

#include "ClassPlayers.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


// ---------------------------------------------------------------------------
//   ClassPlayers     Unité: Miliseconde
//     public
// ---------------------------------------------------------------------------
__fastcall XPlayers::XPlayers(XCore *core, HWND hwnd) {
	Core          = core;
	Hwnd          = hwnd;
	Working       = false;

	PNode         = NULL;
	PFadeInStart  = 0;
	PFadeInStart  = 0;
	PFadeOutStart = 0;
	PFadeInMini   = 1000;
	PFadeOutMini  = 1000;
	PIsFadeInAuto = false;
	PIsFadeOut    = false;
	PMsLength     = 0;

	PEngine = NULL;
	if (Core->Settings->Engine == "Bass" )       PEngine = new XBass();
	if (Core->Settings->Engine == "DirectShow" ) PEngine = new XDirectShow();
	if ( PEngine ) PEngine->Initialize(Hwnd);
}

__fastcall XPlayers::~XPlayers(void) {
	Stop();
	if ( PEngine ) delete PEngine;
}

void __fastcall XPlayers::SetNode(XNode *node) {
	if (node == NULL) return;
	Stop();
	PNode = node->SongNode;
	if ( PEngine ) PEngine->Node = PNode;
	PMsLength = GetMsLength();
	Volume = 0;
	PPlayFlag = false;

	Core->Mix->Wave->Scan(node);
}

bool __fastcall XPlayers::GetIsPlaying(void) {
	if ( PEngine ) return PEngine->IsPlaying;

    return false;
}

void __fastcall XPlayers::Play(void) {
	if (PNode == NULL) return;
	if ( IsPlaying ) return;
	if (Position == 0) Volume = 0.0; else Volume = 1.0;
	PFadeInStart  = PNode->MsFadeIn;
	if (!PPlayFlag) {
		PNode->CountPlay += 1;
		PNode->CountSession += 1;
		PPlayFlag = true;
	}
	if ( PEngine ) PEngine->Play();
	Core->RefreshTv();
}

void __fastcall XPlayers::Stop(void) {
	Volume = 0.0;
	if ( PEngine ) PEngine->Stop();
	PIsFadeIn  = false;
	PIsFadeOut = false;
}

bool __fastcall XPlayers::IsAudioFile(XNode *node) {
	if ( PEngine ) return PEngine->IsAudioFile(node);
	return true;
}

float  __fastcall XPlayers::GetVolume(void) {
	if ( PEngine ) return PEngine->Volume;
	return 0.0;
}

void   __fastcall XPlayers::SetVolume(float volume) {
	if ( PEngine ) PEngine->Volume = volume;
}

float* __fastcall XPlayers::GetFFT(void) {
	if ( PEngine ) return PEngine->FFT;
	return NULL;
}

QWORD __fastcall XPlayers::GetMsLength(void) {
	if ( PEngine ) return PEngine->MsLength;
	return 0;
}

QWORD __fastcall XPlayers::GetMsBeforeEnd(void) {
	if ( PEngine ) return PEngine->MsBeforeEnd;
	return 0;
}

QWORD __fastcall XPlayers::GetPosition(void) {
	if ( PEngine ) return PEngine->Position;
	return 0;
}

void __fastcall XPlayers::SetPosition(QWORD mspos) {
	if ( PEngine ) PEngine->Position = mspos;
}

bool __fastcall XPlayers::FadeIn(void) {
	float p;
	float v;

	if (PIsFadeInAuto) v = (float) ( PFadeInStart + PFadeInMini);
	else v = (float) PFadeInMini;

	PIsFadeOut = false;
	p = (float) Position;

	if (!IsPlaying) {
		Volume = 0.0;
		if ( PEngine ) PEngine->Play();
		PIsFadeIn = true;
	}

	if ( p < v ) {
		PIsFadeIn = true;
		Volume    = p / v;
	}

	if ( p > v ) {
		PIsFadeIn = false;
		Volume    = 1.0;
	}
	return PIsFadeIn;
}

// ---------------------------------------------------------------------------
//   Independant du moteur
// ---------------------------------------------------------------------------

XNode* __fastcall XPlayers::GetNode(void) {
	return PNode;
}

bool __fastcall XPlayers::GetIsValid(void) {
	return (PNode != NULL);
}

void __fastcall XPlayers::FadeInAuto(QWORD length) {
	if (PNode == NULL) return;
	PFadeInStart  = PNode->MsFadeIn;
	PFadeInMini = length;
	PIsFadeInAuto = true;
}

void __fastcall XPlayers::FadeOutAuto(QWORD length) {
	if (PNode == NULL) return;
	if ( PNode->MsFadeOut > MsLength - length ) {
		PFadeOutStart = MsLength - length;
	} else {
		PFadeOutStart = PNode->MsFadeOut;
	}
	PFadeOutMini = length;
	PIsFadeOutAuto = true;
}

void __fastcall XPlayers::FadeInNow(QWORD length) {
	PFadeInStart  = 0;
	PFadeInMini = length;
	PIsFadeInAuto = false;
}

void __fastcall XPlayers::FadeOutNow(QWORD length) {
	PFadeOutStart  = Position;
	PFadeOutMini = length;
	PIsFadeOutAuto = false;
}

bool __fastcall XPlayers::FadeOut(void) {
	QWORD ms, p;
	float v;

	PIsFadeIn = false;
	if (!IsPlaying) {
		PIsFadeOut = false;
		Volume = 0.0;
		return false;
	}
	p = Position;

	if ( (p > PFadeOutStart) && (p < PFadeOutStart + PFadeOutMini) ) {
		PIsFadeOut = true;
		ms = p - PFadeOutStart;
		Volume = 1.0 - (float) ms / PFadeOutMini;
	}
	if ( p >= PFadeOutStart + PFadeOutMini) {
		Volume = 0.0;
		Stop();
		PIsFadeOut = false;
	}
	return PIsFadeOut;
};






