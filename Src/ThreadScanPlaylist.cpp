//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "ThreadScanPlaylist.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

__fastcall ThreadScanPlaylist::ThreadScanPlaylist(XCore *core, bool CreateSuspended) : TThread(CreateSuspended) {
	Core = core;
	Core->TvPlayList->TNR->Text = "Loading Playlist, Please wait...";
	Core->TvPlayList->Enabled = false;
	PFile = new TStringList();
	PFile->Clear();
}

void __fastcall ThreadScanPlaylist::Done(void) {
	Core->TvPlayList->ExpandRoot();
	Core->TvPlayList->Enabled = true;
	Core->TvPlayList->TNR->Text = "PlayList";
	delete PFile;
}

void __fastcall ThreadScanPlaylist::Abort(void) {
	Core->TvPlayList->ExpandRoot();
	Core->TvPlayList->Enabled = true;
	Core->TvPlayList->TNR->Text = "PlayList";
	delete PFile;
}

//---------------------------------------------------------------------------
void __fastcall ThreadScanPlaylist::Execute() {
	int i, j, letter;
	char a;
	XNode *na[50], *naaz[50], *ns;
	UnicodeString s, f;
	unsigned p, nt, cpttile /*, cp*/ ;

	na[0] = NULL;

	if ( !FileExists(Core->Settings->SettingPath + "\\PlayList.sav") ) { Synchronize ( Abort ); return; }
	Core->TvPlayList->Reset();
	if ( !Core->SavFile->ConvertPlaylist() ) { Synchronize ( Abort ); return; }
	PFile->LoadFromFile(Core->Settings->SettingPath + "\\PlayList.sav");
	for (i = 1; i < PFile->Count ; i++) {
		s  = PFile->Strings[i];
		p  = s.Pos(',');
		PLevel = (unsigned) s.SubString(1,p-1).ToInt() ;
		s      = s.SubString(p+1,s.Length()) ;
		p      = s.Pos(',');
		//cp     = (unsigned) s.SubString(1,p-1).ToInt();
		s      = s.SubString(p+1,s.Length());
		p      = s.Pos(',');
		nt     = (unsigned) s.SubString(1,p-1).ToInt();
		s      = s.SubString(p+1,s.Length());
		f      = UpperCase(ExtractFileName(s));

		switch (nt) {
			case NODETYPE::ROOT   : {
				Core->TvPlayList->NR->Path = s;
				na[0] = Core->TvPlayList->NR;
				break;
			}
			case NODETYPE::FOLDER : {
				na[PLevel] = na[PLevel-1]->Add(s,nt);
				break;
			}
			case NODETYPE::SONG : {
				s = Core->Settings->MusicPath + "\\" + s;
				ns = Core->TvList->NRSong->Find(s);
				if (ns == NULL) break;                         // marquer les liens perdus dans les treeview
				na[PLevel-1]->Link(ns);
				ns->CountPlayList += 1;
				break;
			}
		}
	}
	Synchronize ( Done );
}
//---------------------------------------------------------------------------

