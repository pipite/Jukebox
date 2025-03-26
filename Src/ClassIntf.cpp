//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

//---------------------------------------------------------------------------
#include "ClassIntf.h"

// ---------------------------------------------------------------------------
//   XIntf
// ---------------------------------------------------------------------------
XIntf::XIntf(TMain *main) {
	Main = main;
	ImgDropFolder = NULL;
	ImgDropFolder = new XDropFolder( Main->PanelList );
	DropFolder(false);
}

XIntf::~XIntf(void) {
	if (ImgDropFolder) delete ImgDropFolder;
}

void __fastcall XIntf::StopPlay(void)
{
	if (Core->Mix->Current->IsPlaying) {
		Core->Mix->Stop();
		Main->TbStopPlay->ImageIndex = 1;
	} else {
		Core->Mix->Play();
		Main->TbStopPlay->ImageIndex = 0;
	}
}

void __fastcall XIntf::SetWinCaption(UnicodeString text) {
	Main->Caption = text;
}

TCustomImageList* __fastcall XIntf::GetTvIcones(void) {
	return Main->ILTv;
}

void __fastcall XIntf::SetTimers(bool b) {
	Main->TimerLoop->Enabled = b;
	Main->TimerTile->Enabled = b;
}

void __fastcall XIntf::LoadingAnimation(bool loading) {
	Main->ImageLoading->Visible = loading;
	if (!loading) Main->ImageLoading->Align = alNone;
	if (loading)  Main->ImageLoading->Align = alClient;
	((TGIFImage*)(Main->ImageLoading->Picture->Graphic))->Animate = loading;
}

void __fastcall XIntf::WorkingAnimation(bool work) {
	Main->ActivityIndicator1->Visible = work;
	Main->ActivityIndicator1->Animate = work;
}

void __fastcall XIntf::DropFolder(bool dropfolder) {
	ImgDropFolder->Visible = dropfolder;
	if (!dropfolder) ImgDropFolder->Align = alNone;
	if (dropfolder)  ImgDropFolder->Align = alClient;
	if (dropfolder)  ImgDropFolder->Img->Align = alClient;
}

void __fastcall XIntf::SetArboButton(int viewmode) {
	if ( viewmode == VIEWMODE::ARBO ) Main->TbArbo->ImageIndex = 1;
	if ( viewmode == VIEWMODE::FLAT ) Main->TbArbo->ImageIndex = 2;
}

void __fastcall XIntf::SetAzButton(int viewmode) {
	if ( viewmode == AZMODE::NONE ) Main->TbAz->ImageIndex = 3;
	if ( viewmode == AZMODE::AZ   ) Main->TbAz->ImageIndex = 4;
}

HWND 	__fastcall XIntf::GetWinHandle(void)       { return Main->Handle; }
int  	__fastcall XIntf::GetWinTop(void)          { return Main->Top; }
int  	__fastcall XIntf::GetWinLeft(void)         { return Main->Left; }
int  	__fastcall XIntf::GetWinWidth(void)        { return Main->Width; }
int  	__fastcall XIntf::GetWinHeight(void)       { return Main->Height; }

int  	__fastcall XIntf::GetMinTop(void)          { return Main->Top; }
int  	__fastcall XIntf::GetMinLeft(void)         { return Main->Left; }

int 	__fastcall XIntf::GetListWidth(void)       { return Main->PanelList->Width; }
int 	__fastcall XIntf::GetNextHeight(void)      { return Main->PanelNext->Height; }
int 	__fastcall XIntf::GetPastHeight(void)      { return Main->PanelPast->Height; }

int 	__fastcall XIntf::GetSbVolume(void)        { return Main->SbVolume->Position; }

TPanel* __fastcall XIntf::GetPanelList(void)       { return Main->PanelList; }
TPanel* __fastcall XIntf::GetPanelNext(void)       { return Main->PanelNext; }
TPanel* __fastcall XIntf::GetPanelPast(void)       { return Main->PanelPast; }
TPanel* __fastcall XIntf::GetPanelPlayList(void)   { return Main->PanelPlayList; }

TImage* __fastcall XIntf::GetImgSpectrum(void)     { return Main->ImgSpectrum; }
TImage* __fastcall XIntf::GetImgWave(void)         { return Main->ImgWave; }

void  	__fastcall XIntf::SetMinTop(int top)       { if (Mini) Mini->Top  = top; PMinTop = top; }
void 	__fastcall XIntf::SetMinLeft(int left)     { if (Mini) Mini->Left = left; PMinTop = left;  }

void  	__fastcall XIntf::SetWinTop(int top)       { Main->Top  = top; }
void 	__fastcall XIntf::SetWinLeft(int left)     { Main->Left = left;  }
void  	__fastcall XIntf::SetWinWidth(int width)   { Main->Width  = width; }
void 	__fastcall XIntf::SetWinHeight(int height) { Main->Height = height;  }

void 	__fastcall XIntf::SetListWidth(int width)   { Main->PanelList->Width = width; }
void 	__fastcall XIntf::SetNextHeight(int height) { Main->PanelNext->Height = height; }
void 	__fastcall XIntf::SetPastHeight(int height) { Main->PanelPast->Height = height; }

void    __fastcall XIntf::ShowMessage(const UnicodeString &message) {
	// Utilisation de la fonction globale ShowMessage de VCL
	::ShowMessage(message);
}

