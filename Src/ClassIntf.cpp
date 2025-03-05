//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassIntf.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   XIntf
// ---------------------------------------------------------------------------
__fastcall XIntf::XIntf(TBoxon *boxon) {
	ImgDropFolder = NULL;
	ImgDropFolder = new XDropFolder( Boxon->PanelList );
	DropFolder(false);
}

__fastcall XIntf::~XIntf(void) {
	if (ImgDropFolder) delete ImgDropFolder;
}

void __fastcall XIntf::SetWinCaption(UnicodeString text) {
	Boxon->Caption = text;
}

TCustomImageList* __fastcall XIntf::GetTvIcones(void) {
	return Boxon->ILTv;
}

void __fastcall XIntf::SetTimers(bool b) {
	Boxon->TimerLoop->Enabled = b;
	Boxon->TimerTile->Enabled = b;
}

void __fastcall XIntf::LoadingAnimation(bool loading) {
	Boxon->ImageLoading->Visible = loading;
	if (!loading) Boxon->ImageLoading->Align = alNone;
	if (loading)  Boxon->ImageLoading->Align = alClient;
	((TGIFImage*)(Boxon->ImageLoading->Picture->Graphic))->Animate = loading;
}

void __fastcall XIntf::WorkingAnimation(bool work) {
	Boxon->ActivityIndicator1->Visible = work;
	Boxon->ActivityIndicator1->Animate = work;
}

void __fastcall XIntf::DropFolder(bool dropfolder) {
	ImgDropFolder->Visible = dropfolder;
	if (!dropfolder) ImgDropFolder->Align = alNone;
	if (dropfolder)  ImgDropFolder->Align = alClient;
	if (dropfolder)  ImgDropFolder->Img->Align = alClient;
}

void __fastcall XIntf::SetArboButton(int viewmode) {
	if ( viewmode == VIEWMODE::ARBO ) Boxon->TbArbo->ImageIndex = 1;
	if ( viewmode == VIEWMODE::FLAT ) Boxon->TbArbo->ImageIndex = 2;
}

void __fastcall XIntf::SetAzButton(int viewmode) {
	if ( viewmode == AZMODE::NONE ) Boxon->TbAz->ImageIndex = 3;
	if ( viewmode == AZMODE::AZ   ) Boxon->TbAz->ImageIndex = 4;
}

HWND 	__fastcall XIntf::GetWinHandle(void)       { return Boxon->Handle; }
int  	__fastcall XIntf::GetWinTop(void)          { return Boxon->Top; }
int  	__fastcall XIntf::GetWinLeft(void)         { return Boxon->Left; }
int  	__fastcall XIntf::GetWinWidth(void)        { return Boxon->Width; }
int  	__fastcall XIntf::GetWinHeight(void)       { return Boxon->Height; }

int  	__fastcall XIntf::GetMinTop(void)          { return Boxmin->Top; }
int  	__fastcall XIntf::GetMinLeft(void)         { return Boxmin->Left; }

int 	__fastcall XIntf::GetListWidth(void)       { return Boxon->PanelList->Width; }
int 	__fastcall XIntf::GetNextHeight(void)      { return Boxon->PanelNext->Height; }
int 	__fastcall XIntf::GetPastHeight(void)      { return Boxon->PanelPast->Height; }

int 	__fastcall XIntf::GetSbVolume(void)        { return Boxon->SbVolume->Position; }

TPanel* __fastcall XIntf::GetPanelList(void)       { return Boxon->PanelList; }
TPanel* __fastcall XIntf::GetPanelNext(void)       { return Boxon->PanelNext; }
TPanel* __fastcall XIntf::GetPanelPast(void)       { return Boxon->PanelPast; }
TPanel* __fastcall XIntf::GetPanelPlayList(void)   { return Boxon->PanelPlayList; }

TImage* __fastcall XIntf::GetImgSpectrum(void)     { return Boxon->ImgSpectrum; }
TImage* __fastcall XIntf::GetImgWave(void)         { return Boxon->ImgWave; }

void  	__fastcall XIntf::SetMinTop(int top)       { if (Boxmin) Boxmin->Top  = top; PMinTop = top; }
void 	__fastcall XIntf::SetMinLeft(int left)     { if (Boxmin) Boxmin->Left = left; PMinTop = left;  }

void  	__fastcall XIntf::SetWinTop(int top)       { Boxon->Top  = top; }
void 	__fastcall XIntf::SetWinLeft(int left)     { Boxon->Left = left;  }
void  	__fastcall XIntf::SetWinWidth(int width)   { Boxon->Width  = width; }
void 	__fastcall XIntf::SetWinHeight(int height) { Boxon->Height = height;  }

void 	__fastcall XIntf::SetListWidth(int width)   { Boxon->PanelList->Width = width; }
void 	__fastcall XIntf::SetNextHeight(int height) { Boxon->PanelNext->Height = height; }
void 	__fastcall XIntf::SetPastHeight(int height) { Boxon->PanelPast->Height = height; }

