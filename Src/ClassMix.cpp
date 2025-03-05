//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassMix.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   ClassMix       Unité = ms
//       Durées Mix: QWORD  = 1000 for a second.
//       Volume Mix: float  = 0.0 <= Volume <= 1.0
// ---------------------------------------------------------------------------
__fastcall XMix::XMix(HWND handle, XCore *core) {
	Handle        = handle;
	Core          = core;
	PAdbPercent   = Core->Settings->AdbPercent;
	PLast         = new XPlayers(Core, Handle);
	PCurrent      = new XPlayers(Core, Handle);
	PNext         = new XPlayers(Core, Handle);
	PSpectrum     = new XSpectrum(this);
	PWave         = new XWave(Core);
	PTmp          = NULL;
	PIsMixing     = false;
	PMsMixLength  = 5000;
}

__fastcall XMix::~XMix(void) {
	if (PWave     != NULL) delete PWave;
	if (PSpectrum != NULL) delete PSpectrum;
	if (PLast     != NULL) delete PLast;
	if (PCurrent  != NULL) delete PCurrent;
	if (PNext     != NULL) delete PNext;
};

void __fastcall XMix::Init(void) {
	if (Core->TvList->NRSong->HaveSong) {
		Core->TvList->SetNodeRoot();
		Core->TvPast->Reset();
		Core->TvNext->Fill();
		XNode *cn = Core->TvNext->Pop;
		PCurrent->Node = cn;
		Core->TvNext->Fill();
		PNext->Node = Core->TvNext->Next;
		PCurrent->Play();
		PIsMixing = true;
		Core->TvPast->TNR->Expand(false);
	}
}

void __fastcall XMix::Roll(void) {
	PTmp  = PLast;
	PLast = PCurrent;
	PCurrent = PNext;
	PNext = PTmp;
	PNext->Node = Core->TvNext->Next;
	PWave->NeedRefresh = true;
}

void __fastcall XMix::Auto(void) {
	if ( Core->TvList->NRSong->HaveSong ) {
		if (!PIsMixing) {
			PCurrent->FadeOutAuto(Core->Settings->FadeOutDuration);
			PNext->FadeInAuto(Core->Settings->FadeInDuration);
			if (IsTimeToStartMix) {
				PCurrent->Play();
				Last->Stop();
				Core->TvNext->Pop;
				Roll();
				PNext->Node = Core->TvNext->Next;
				Core->TvNext->Fill();
				PIsMixing = true;
			}
		}
		PSpectrum->Draw();
		PWave->Refresh(Current->Node);
	}
	if (PIsMixing) CrossFade();
}

void __fastcall XMix::NextNow(void) {
	NodeNow(Core->TvNext->Pop);
}

void __fastcall XMix::NodeNow(XNode *node) {
	if (!PCurrent->IsPlaying) return;
	if ( Core->TvList->NRSong->HaveSong ) {
		Last->Stop();
		PCurrent->FadeOutNow(Core->Settings->FadeOutDuration);
		PNext->FadeInNow(Core->Settings->FadeInDuration);
		PNext->Node = node;
		Roll();
		PNext->Node = Core->TvNext->Next;
		Core->TvNext->Fill();
		PCurrent->Play();
		PIsMixing = true;
	}
};

void __fastcall XMix::CrossFade(void) {
	bool a, b;
	a = PLast->FadeOut();
	b = PCurrent->FadeIn();
	if ( a || b ) PIsMixing = true; else PIsMixing = false;
};


bool __fastcall XMix::GetIsTimeToStartMix(void) {
	if ( !PCurrent->IsValid ) return false;
	if ((unsigned __int64) PCurrent->Position > (PCurrent->Node->MsFadeOut - PNext->Node->MsFadeIn) ) return true;
	return false;
}

void __fastcall XMix::Play(void) {
	if (!PCurrent->IsPlaying) {
		PCurrent->Play();
	} else {
		PLast->Stop();
	}
};

void __fastcall XMix::Stop(void) {
	PLast->Stop();
	PCurrent->Stop();
};

unsigned __int64 __fastcall XMix::GetPosition(void) {
	unsigned __int64 l;
	l = PCurrent->MsLength;
	if (l == 0) return 0;
	return PCurrent->Position;
};



