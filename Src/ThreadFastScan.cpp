//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "ThreadFastScan.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

__fastcall ThreadFastScan::ThreadFastScan(XCore *core, bool CreateSuspended) : TThread(CreateSuspended) {
	Core = core;
	Core->TvList->Enabled = false;
	Core->Loader->Loading = true;
	Core->Loader->Loaded  = false;
	PFile = new TStringList();
	Core->PanelListMode = "List";
	Core->TvList->Reset();
	Core->TvList->TNR->Text = "Loading, Please wait...";
}

void __fastcall ThreadFastScan::Done(void) {
	Core->TvList->Enabled = true;
	Core->TvList->TNR->Text = Core->Settings->MusicPath;
	Core->Loader->Loading = false;
	Core->Loader->Loaded  = true;
	Core->Mix->Init();
	Core->Intf->Timers    = true;
	Core->Loader->SaveArboNeeded = false;
	TileInfo();
	delete PFile;
	new ThreadScanPlaylist(Core,false);
	Core->PanelListMode = "List";
}

void __fastcall ThreadFastScan::Abort(void) {
	Core->TvList->Enabled = true;
	Core->TvList->TNR->Text = "Loading failed...";
	Core->Loader->Loading = false;
	delete PFile;
	Core->PanelListMode = "DropFolder";
}

void __fastcall ThreadFastScan::TileInfo(void) {
	Core->Intf->WinCaption = L"  L o a d i n g   T r a c k s    " + UnicodeString(Core->TvList->NRSong->CountSongAll);
}

//---------------------------------------------------------------------------
void __fastcall ThreadFastScan::Execute() {
	int i, j, letter;
	char a;
	XNode *na[50], *naaz[50], *nf, *ns, *nfaz;
	UnicodeString s, f;
	unsigned p, cp, nt, cpttile;

	nf = ns = nfaz = NULL;

	if ( !FileExists(Core->Settings->SettingPath + "\\Arbo.sav") ) { Abort(); return; }
	Core->TvNext->Reset();
	Core->TvPast->Reset();

	PLevel = 0;
	cpttile = 0;
	PFile->Clear();
	PFile->LoadFromFile(Core->Settings->SettingPath + "\\Arbo.sav");
	for (i = 0; i < PFile->Count ; i++) {
		s  = PFile->Strings[i];
		p  = s.Pos(',');
		PLevel = (unsigned) s.SubString(1,p-1).ToInt() ;
		s      = s.SubString(p+1,s.Length()) ;
		p      = s.Pos(',');
		cp     = (unsigned) s.SubString(1,p-1).ToInt();
		s      = s.SubString(p+1,s.Length());
		p      = s.Pos(',');
		nt     = (unsigned) s.SubString(1,p-1).ToInt();
		s      = s.SubString(p+1,s.Length());
		f      = UpperCase(ExtractFileName(s));
		if ( (i == 0) && (s != Core->Settings->MusicPath) )  {
			Synchronize( Abort );
			return;
		}

		switch (nt) {
			case NODETYPE::ROOT   : {
				Core->TvList->NRArbo->Path = s;
				Core->TvList->NRFlat->Path = s;
				Core->TvList->NRSong->Path = s;

				//Core->TvList->Items->Clear();
				na[0] = Core->TvList->NRArbo;
				break;
			}
			case NODETYPE::FOLDER : {
				// Arbo
				na[PLevel] = na[PLevel-1]->Add(s,nt);
				// Flat
				if (nf != NULL) if (nf->CountSongAll == 0) delete nf;// nf->Parent->Extract(nf);
				nf = Core->TvList->NRFlat->Add(s,nt);

				// FlatAz
				a = f[1];
				if (nfaz != NULL) if (nfaz->CountSongAll == 0) delete nfaz; //nfaz->Parent->Extract(nfaz);
				if ( (a >= 'A') && (a <= 'Z') ) {
					nfaz = Core->TvList->NRFlatAz->ChildAt(a - 'A' + 1)->Add(s,nt);
				} else {
					nfaz = Core->TvList->NRFlatAz->ChildAt(0)->Add(s,nt);
				}
				// ArboAz
				if (PLevel == 1) {
					if ( (a >= 'A') && (a <= 'Z') ) {
						naaz[2] = Core->TvList->NRArboAz->ChildAt(a - 'A' + 1)->Add(s,nt);
					} else {
						naaz[2] = Core->TvList->NRArboAz->ChildAt(0)->Add(s,nt);
					}
				} else {
					naaz[PLevel+1] = naaz[PLevel]->Add(s,nt);
				}
				break;
			}
			case NODETYPE::SONG : {
				// Song
				ns = Core->TvList->NRSong->Add(s,nt);
				ns->CountPlay = cp;
				// Arbo
				na[PLevel-1]->Link(ns);
				// Flat
				nf->Link(ns);
				// FlatAz
				nfaz->Link(ns);
				// ArboAz
				naaz[PLevel]->Link(ns);
				cpttile++;
				break;
			}
		}

		if (cpttile == 300) {
			cpttile = 0;
			Synchronize( TileInfo );
		}
	}
	Core->TvList->NRArbo->Sort();
	Core->TvList->NRFlat->Sort();
	Synchronize ( Done );
}
//---------------------------------------------------------------------------

