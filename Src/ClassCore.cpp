//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassCore.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

#define LISTPATH L""

// ---------------------------------------------------------------------------
//   XCore
// ---------------------------------------------------------------------------
__fastcall XCore::XCore(XIntf *intf) {
	int pos;

	PCreated            = false;
	Intf                = intf;

	PWorkingLevel       = 0;

	try {
		Settings              = new XSettings(this);

		PFontSize             = Settings->FontSizeMin;
		Loader                = new XLoader(this);
		PMix                  = new XMix(Intf->WinHandle, this);
		PMix->Spectrum->Image = Intf->ImgSpectrum;
		PMix->Wave->Image     = Intf->ImgWave;
		PSavFile              = new XFile(this);

		TvList                = new XTvList(Intf->PanelList, this);
		TvNext                = new XTvNext(Intf->PanelNext, this);
		TvPast                = new XTvPast(Intf->PanelPast, this);
		TvPlayList            = new XTvPlayList(Intf->PanelPlayList, this);

		srand(time(NULL));
		Randomize();
		PCreated              = true;
	} catch (...) {
	}
	Loader->Load();
}

__fastcall XCore::~XCore(void) {
	try {
		if (TvPlayList != NULL) delete TvPlayList;
		if (TvPast     != NULL) delete TvPast;
		if (TvNext     != NULL) delete TvNext;
		if (TvList     != NULL) delete TvList;    // Bug à la fermeture
		if (PSavFile   != NULL) delete PSavFile;
		if (PMix       != NULL) delete PMix;
		if (Loader     != NULL) delete Loader;
		if (Settings   != NULL) delete Settings;
	} catch (...) {
	}
}

void __fastcall XCore::SetPanelListMode(UnicodeString visual) {
	if (visual == "DropFolder") {
		TvList->Visible = false;
		TvList->Align = alNone;
		Intf->LoadingAnimation(false);
		Intf->DropFolder(true);
	} else if (visual == "List") {
		Intf->DropFolder(false);
		Intf->LoadingAnimation(false);
		TvList->Visible = true;
		TvList->Align = alClient;
	} else { // Loading
		TvList->Visible = false;
		TvList->Align = alNone;
		Intf->DropFolder(false);
		Intf->LoadingAnimation(true);
	}
	PPanelListMode = visual;
}

void __fastcall XCore::SetWorking(bool work) {
	if (!PCreated) return;
	if (work) PWorkingLevel++; else PWorkingLevel--;
	bool b = ( PWorkingLevel > 0 );
	Intf->WorkingAnimation(b);
}

void __fastcall XCore::SetFontSize(int size) {
	if (!PCreated) return;
	if (!Settings->FontResize ) size = Settings->FontSizeMin;
	if (PFontSize != size) {
		PFontSize = size;
		TvList->Font->Size     = size;
		TvNext->Font->Size     = size;
		TvPast->Font->Size     = size;
		TvPlayList->Font->Size = size;
	}
}

void __fastcall XCore::RefreshTv(void) {
	if (!PCreated) return;
	TvList->Refresh();
	TvPast->Refresh();
	TvPlayList->Refresh();
}

bool __fastcall XCore::GetIsValidFolderPath(void) {
	return DirectoryExists( Settings->MusicPath );
}

void __fastcall XCore::ModifyFolderPath(UnicodeString folderpath) {
	if (!PCreated) return;
	if ( !DirectoryExists(folderpath) ) return;
	Intf->Timers = false;
	Mix->Stop();

	TvNext->Reset();
	TvPast->Reset();
	TvPlayList->Reset();
	TvList->Reset();

	Settings->MusicPath = folderpath;
	new ThreadScanDisk(this, false);
}

void __fastcall XCore::LoadPlayList(UnicodeString path) {
	if (!PCreated) return;
	if ( !FileExists(path) ) return;
	PSavFile->ArchivePlayList();
	DeleteFile(Settings->SettingPath + "\\PlayList.sav");
	TFile::Copy(path,Settings->SettingPath + "\\PlayList.sav");

	new ThreadScanPlaylist(this, false);
}

void __fastcall XCore::SetVolume(int value) {
	unsigned v = 65535 - (unsigned) value;
	unsigned Vol = ((v & 0x0000ffff) | (v << 16));
	waveOutSetVolume(NULL, Vol);
	PVolume = value;
}

/*
TStringList* Split(const String& Texte, const WideChar Delimiteur) {
	TStringList* Resultat = new TStringList();

	// Cas d'une chaîne vide
	if (Texte.IsEmpty()) {
		return Resultat;
	}

	int DebutPos = 1;
	int DelimPos = 0;

	while ((DelimPos = Texte.Pos(Delimiteur, DebutPos)) != 0) {
		String SousChaine = Texte.SubString(DebutPos, DelimPos - DebutPos);
		Resultat->Add(SousChaine);
		DebutPos = DelimPos + 1;
	}

	// Ajouter la dernière partie
	if (DebutPos <= Texte.Length()) {
		String DernierePart = Texte.SubString(DebutPos, Texte.Length() - DebutPos + 1);
		Resultat->Add(DernierePart);
	} else if (Texte[Texte.Length()] == Delimiteur) {
		Resultat->Add(L"");
	}

	return Resultat;
}
*/
