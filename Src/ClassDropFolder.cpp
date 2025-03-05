//---------------------------------------------------------------------------

#pragma hdrstop

class XDropFolder;

#include "ClassDropFolder.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

__fastcall XDropFolder::XDropFolder(TComponent *owner) : TPanel(owner) {
	Core = NULL;
	Parent = (TPanel*) owner;
	Img = new TImage(this);
	Img->Parent = this;
	Img->Align = alClient;
	Img->Center = true;
    Img->Picture->Bitmap->LoadFromFile("..\\Icon\\Drop.bmp");
	DragAcceptFiles(Handle,true);
}

__fastcall XDropFolder::~XDropFolder(void) {
	delete Img;
};

void __fastcall XDropFolder::WMDropFiles(TMessage msg) {
	HDROP hDrop;
	hDrop = (HDROP) msg.WParam;

	UINT nCnt  = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
	for(UINT i = 0; i < nCnt; ++i) {
		UINT nSize;
		if(0 == (nSize = DragQueryFile(hDrop, i, NULL, 0)))
			continue;

		TCHAR *pszFileName = new TCHAR[++nSize];
		if(DragQueryFile(hDrop, i, pszFileName, nSize)) {
			//Caption = UnicodeString(pszFileName);
		}
		Core->Settings->MusicPath = UnicodeString(pszFileName);
		delete [] pszFileName;
	}
	Core->Loader->Load();
	DragFinish(hDrop);
}


