//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassLoader.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


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
	new ThreadLoader(Core,false);
}


//// ---------------------------------------------------------------------------
////   ThreadLoader
//// ---------------------------------------------------------------------------
__fastcall ThreadLoader::ThreadLoader(XCore *core, bool CreateSuspended) : TThread(CreateSuspended)
{
	Core = core;
	FreeOnTerminate = true;
}

////---------------------------------------------------------------------------
void __fastcall ThreadLoader::Execute(void)
{
	if ( Core->IsValidFolderPath && FileExists(Core->Settings->SettingPath + "\\Arbo.sav") ) {
		new ThreadFastScan(Core,false);
	} else if ( Core->IsValidFolderPath ) {
		new ThreadScanDisk(Core, false);
	} else {
		Core->PanelListMode = "DropFolder";
	}
}




