//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassLoader.h"
//---------------------------------------------------------------------------
//#pragma package(smart_init)


// ---------------------------------------------------------------------------
//   XLoader
// ---------------------------------------------------------------------------
__fastcall XLoader::XLoader(XCore *core) {
	Core = core;

	Loaded              = false;
	Loading             = false;

	SaveArboNeeded      = false;
	SavePlayListNeeded  = false;
}

__fastcall XLoader::~XLoader(void) {
}

void __fastcall XLoader::Load(void) {
	// Lancement des Thread de chargement
	// Principe :
	// ThreadFastScan
	//    Charge les node NRSong
	//    Crée les NRSong link dans TvList, (NRFlatAz et NRArboAz)
	//    Rempli TvNext
    //    Lance la lecture du premier morceau de musique
	//    Quand ThreadFastScan est terminé, lance ThreadScanPlaylist
	//        Charge les playlists
	//            Crée les NRSong link dans les nodes TvPlayList

	if ( Core->IsValidFolderPath && FileExists(Core->Settings->SettingPath + "\\Arbo.sav") ) {
		new ThreadFastScan(Core,false);
	} else if ( Core->IsValidFolderPath ) {
		new ThreadScanDisk(Core, false);
	} else {
		Core->PanelListMode = "DropFolder";
	}
}


