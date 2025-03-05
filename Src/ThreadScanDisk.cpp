//---------------------------------------------------------------------------

#pragma hdrstop

#include "ThreadScanDisk.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   ThreadScanDisk
// ---------------------------------------------------------------------------
__fastcall ThreadScanDisk::ThreadScanDisk(XCore *core, bool CreateSuspended) : TThread(CreateSuspended)
{
	Core                  = core;
	FreeOnTerminate       = true;
	Core->Loader->Loading = true;
	Core->Loader->Loaded  = false;
	Folder                = Core->Settings->MusicPath;
	Core->TvList->NRArbo->Path = Folder;
	Level                 = 0;
	c                     = 0;
	Core->PanelListMode = "Loading";
}

//---------------------------------------------------------------------------
void __fastcall ThreadScanDisk::Execute(void)
{
	char a;
	if ( DirectoryExists(Folder) ) {
		Core->TvList->Reset();
		Core->TvNext->Reset();
		Core->TvPast->Reset();
		RecurseScan( Core->TvList->NRArbo, Core->TvList->NRFlat, Core->TvList->NRFlatAz, Core->TvList->NRArboAz);
		Synchronize( Done );
	} else {
		Abort();
	}
}
//---------------------------------------------------------------------------
void __fastcall ThreadScanDisk::Done(void) {
	Core->TvList->NRArbo->Sort();
	Core->TvList->NRFlat->Sort();
	Core->SavFile->Save();
	Core->Loader->Loading = false;
	Core->Loader->Loaded  = true;
	Core->Mix->Init();
	Core->Intf->Timers = true;
	Core->Loader->SaveArboNeeded = true;
	new ThreadScanPlaylist(Core,false);
	Core->PanelListMode = "List";
}

void __fastcall ThreadScanDisk::Abort(void) {
	Core->Loader->Loading = false;
	Core->Loader->Loaded  = false;
	Core->PanelListMode = "DropFolder";
}

void __fastcall ThreadScanDisk::TileInfo(void) {
	Core->Intf->WinCaption = L"  S c a n n i n g   T r a c k s     " + UnicodeString(Core->TvList->NRSong->CountSongAll);
}

void __fastcall ThreadScanDisk::SetFolder(UnicodeString folder) {
}

void __fastcall ThreadScanDisk::RecurseScan(XNode *arbofolder, XNode *flatfolder, XNode *flatazfolder, XNode *arboazfolder) {
	int validres;
	TSearchRec SearchRec;
	UnicodeString FullName;
	UnicodeString s, f;
	XNode *newsongnode;
	XNode *newarbofolder, *newflatfolder, *newflatazfolder, *newarboazfolder;
	bool isfolder;
	char a;

	if (!DirectoryExists(Folder)) { Abort(); return; }

	Level++;
	validres = FindFirst(arbofolder->Path + L"\\*.*", faAnyFile, SearchRec);
	while (validres == 0) {
		if ( SearchRec.Name[1] != '.' ) {
			if (arbofolder->Level > 10) {
				arbofolder->Peak  = arbofolder->Peak;
			}
			isfolder = ( (SearchRec.Attr&0xFF) == 16 );
			FullName = arbofolder->Path + L"\\" + SearchRec.Name;
			s = LowerCase(ExtractFileExt(SearchRec.Name));
			f = UpperCase(ExtractFileName(SearchRec.Name));
			if ( ( (s == ".mp3") || (s == ".mp4") || (s == ".wma") || (s == ".m4a")) || isfolder) {
				if (!isfolder) {
					newsongnode  = Core->TvList->NRSong->Add(FullName, NODETYPE::SONG);
					arbofolder->Link(newsongnode);
					flatfolder->Link(newsongnode);
					flatazfolder->Link(newsongnode);
					arboazfolder->Link(newsongnode);
					if (c++ == 50) { Synchronize (TileInfo); c = 0; }
				} else {
					newflatfolder = Core->TvList->NRFlat->Add(FullName, NODETYPE::FOLDER);
					newarbofolder = arbofolder->Add(FullName, NODETYPE::FOLDER);
					// FlatAz
					a = f[1];
					if ( (a >= 'A') && (a <= 'Z') ) {
						newflatazfolder = Core->TvList->NRFlatAz->ChildAt(a - 'A' + 1)->Add(FullName,NODETYPE::FOLDER);
					} else {
						newflatazfolder = Core->TvList->NRFlatAz->ChildAt(0)->Add(FullName,NODETYPE::FOLDER);
					}
					// ArboAz
					if (Level == 1) {
						if ( (a >= 'A') && (a <= 'Z') ) {
							newarboazfolder = Core->TvList->NRArboAz->ChildAt(a - 'A' + 1)->Add(FullName,NODETYPE::FOLDER);
						} else {
							newarboazfolder = Core->TvList->NRArboAz->ChildAt(0)->Add(FullName,NODETYPE::FOLDER);
						}
					} else {
						newarboazfolder = arboazfolder->Add(FullName,NODETYPE::FOLDER);
					}
				}
			}
			if (isfolder) {
				RecurseScan(newarbofolder, newflatfolder, newflatazfolder, newarboazfolder);
				if (newflatfolder->CountSong      == 0) delete newflatfolder;   // Core->TvList->NRFlat->Extract(newflatfolder);
				if (newarbofolder->CountSongAll   == 0) delete newarbofolder;   // newarbofolder->Parent->Extract(newarbofolder);
				if (newflatazfolder->CountSongAll == 0) delete newflatazfolder; // newflatazfolder->Parent->Extract(newflatazfolder);
				if (newarboazfolder->CountSongAll == 0) delete newarboazfolder; // newarboazfolder->Parent->Extract(newarboazfolder);
			}
		}
		validres = FindNext(SearchRec);
	}
	FindClose(SearchRec);
	Level--;
	return;
}


