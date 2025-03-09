//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassSavFile.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

__fastcall XFile::XFile(XCore *core) {
	Core = core;
	PFile = new TStringList();
	Core->Loader->Loaded = false;
}

__fastcall XFile::~XFile(void) {
	delete PFile;
}

void __fastcall XFile::MoveToArchive(UnicodeString src) {
	try {
		TFile::Copy(src, Core->Settings->BackupPath + "\\" + ExtractFileName(src) + ExtendDate );
	} catch (...) {
	}
	TFile::Delete(src);
}

bool __fastcall XFile::ConvertPlaylist(void) {
	char s1[1000];
	UnicodeString p1, p2;
	int i;

	// Analyse Playlist V22 ou V23
	//     0,0,0,Playlist = V22
	//     Playlist V2.3  = V23

	if ( HavePlaylist ) {
		p1 = Core->Settings->MusicPath;
		strcpy(s1, "");
		Pf.open( UnicodeString(PathPlaylist).c_str() );
		Pf.getline(s1, 999, '\n');
		p2 = UnicodeString(s1);
		Pf.close();
		if ( p2.Pos(L"Playlist V2.3") ) return true;   // Marqueur V23
		if ( p2.Pos(L"0,0,0,") ) {                     // Marqueur V22
			// Convertir en V23
			PFile->LoadFromFile(PathPlaylist);
			RemoveRootPath(false);
			PFile->Strings[0] = "0,0,0," + p1;
			p1 = Core->Settings->BackupPath;
			MoveToArchive(PathPlaylist);
			PFile->Insert(0,"Playlist V2.3");
			PFile->SaveToFile(PathPlaylist);
			return true;
		}
	}
	return false;
}

void __fastcall XFile::RemoveRootPath(bool bypass) {
	int i,j,k;
	UnicodeString s1,s2;

	for ( i = 0; i < PFile->Count ; i++ ) {
		s1 = s2 = PFile->Strings[i];

		j = s2.Pos(",")+1;
		k = j;
		s2 = s2.SubString(j,s2.Length());

		j = s2.Pos(",")+1;
		k += j;
		s2 = s2.SubString(j,s2.Length());

		if ( (s2.SubString(1, s2.Pos(",")-1) == "2") || bypass ) {
			j = s2.Pos(",")+1;
			k += j-3;
			s2 = s2.SubString(j+1,s2.Length());
			s1 = s1.SubString(1,k);
			s2 = s2.SubString(Core->Settings->MusicPath.Length()+1 ,s2.Length());
			PFile->Strings[i] = s1+s2;
		}
	}
}

void __fastcall XFile::Save(void) {
	if ( !Core->Loader->Loaded ) return;
	if (Core->Loader->SavePlayListNeeded) {
		ArchivePlayList();
		SavePlaylist(PathPlaylist);
	}
}

void __fastcall XFile::SavePlaylist(UnicodeString filename) {
	if ( !Core->Loader->Loaded ) return;
	PFile->Clear();
	PLevel = 0;
    PFile->Add(L"Playlist V2.3");
	SaveRecurseNode(Core->TvPlayList->NR);
	RemoveRootPath(false);
	PFile->SaveToFile(filename);
	Core->Loader->SavePlayListNeeded = false;
}

void __fastcall XFile::ArchiveArbo(void) {
	if ( HaveArbo ) {
		PLevel = 0;
		PFile->Clear();
		MoveToArchive(PathArbo);
		Purge("Arbo");
	}
}

void __fastcall XFile::ArchivePlayList(void) {
	if ( HavePlaylist ) {
		PLevel = 0;
		PFile->Clear();
		MoveToArchive(PathPlaylist);
		Purge("PlayList");
	}
}

void __fastcall XFile::SaveRecurseNode(XNode *node) {
	unsigned i;

	PFile->Add( UnicodeString(PLevel) +L","+ UnicodeString(node->CountPlay) +L","+ UnicodeString(node->NodeType) +L"," + node->Path );
	for (i = 0; i < node->CountChild ; i++) {
		PLevel++;
		SaveRecurseNode(node->ChildAt(i));
		PLevel--;
	}
}

void __fastcall XFile::Purge(UnicodeString filename) {
	int eof;
	TSearchRec SearchRec;
	TDateTime dt, dt2, dt3;
	TStringList *FileToDelete;
	TStringList *FileDate;
	int i, j, k, l;
	int fileminitokeep;
	UnicodeString s;

	fileminitokeep = Core->Settings->AutoSaveToKeep;;

	FileToDelete = new TStringList();
	FileDate     = new TStringList();

	if (!DirectoryExists(Core->Settings->BackupPath)) return;

	FileToDelete->Clear();
	FileDate->Clear();
	eof = FindFirst(Core->Settings->BackupPath + L"\\" + filename + ".sav.*", faAnyFile, SearchRec);
	while (eof == 0) {
		if (SearchRec.Name[1] != '.') {
			s = SearchRec.Name;
			FileDate->Add(DateTimeToStr(SearchRec.TimeStamp));
			FileToDelete->Add(Core->Settings->BackupPath + L"\\" + LowerCase(SearchRec.Name));
		}
		eof = FindNext(SearchRec);
	}
	FindClose(SearchRec);

	if (FileToDelete->Count > 0) {
		l = FileToDelete->Count - fileminitokeep;
		for (i = 0; i < l; i++) {
			k = -1;
			dt2 = IncDay(Now(),0);
			for (j = 0; j < FileToDelete->Count; j++) {
				dt = StrToDateTime(FileDate->Strings[j]);
				if (CompareDateTime(dt,dt2) == LessThanValue) {
					s = FileToDelete->Strings[j];
					k = j;
					dt2 = dt;
				}
			}
			if (k != -1) {
				if (FileExists(FileToDelete->Strings[k])) DeleteFile(FileToDelete->Strings[k]);
				FileToDelete->Delete(k);
				FileDate->Delete(k);
			} else {
				break;
			}
		}
	}
	delete FileToDelete;
	delete FileDate;
}

UnicodeString __fastcall XFile::GetPathArbo(void) {
	return Core->Settings->SettingPath + "\\Arbo.sav";
}

UnicodeString __fastcall XFile::GetPathPlaylist(void) {
	return Core->Settings->SettingPath + "\\PlayList.sav";
}

bool __fastcall XFile::GetHaveArbo(void) {
	return FileExists(PathArbo);
}

bool __fastcall XFile::GetHavePlaylist(void) {
	return FileExists(PathPlaylist);
}

UnicodeString __fastcall XFile::GetExtendDate(void) {
	return FormatDateTime(L".yyyy_mm_dd__hh_nn_ss",Now());
}


/*
 void __fastcall XFile::ConvertArbo(void) {
	char s1[1000];
	UnicodeString p1, p2;
	int i;

	// Analyse Arbo V22 ou V23
	// 0,0,0,chemin
	if ( HaveArbo ) {
		strcpy(s1, "");
		Pf.open( PathArbo.c_str() );
		Pf.getline(s1, 999, '\n');
		i = strlen(s1)-6;
		p1 = UnicodeString(s1).SubString(7,i);
		Pf.getline(s1, 999, '\n');
		Pf.close();
		p2 = UnicodeString(s1).SubString(7,i);
		if ( p1.LowerCase() == p2.LowerCase() ) {
			// Convertir en V23
			PFile->LoadFromFile(PathArbo);
			RemoveRootPath(true);
			PFile->Strings[0] = PFile->Strings[0] + p1;
			PFile->SaveToFile(PathArbo+".Convert");
		}
	}
}

*/
