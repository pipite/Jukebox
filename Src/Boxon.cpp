//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Boxon.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

using namespace std;

TBoxon *Boxon;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TBoxon::TBoxon(TComponent* Owner) : TForm(Owner)
{
	Created = false;
	Intf = new XIntf(this);
	Core = new XCore(Intf);
	if (!Core->Created) return;
	Intf->ImgDropFolder->Core = Core;

	OWidth             = Width;
	OHeight            = Height;

	SbVolume->Position = Core->Settings->Volume;
	OnResize = FormResize;
	Created = true;
}

void __fastcall TBoxon::FormClose(TObject *Sender, TCloseAction &Action)
{
	Caption = L"   Freeing Memory . . .";
	Core->Mix->Stop();
	DisableTimer();
	Core->Settings->Save();
	Core->SavFile->Save();

	if (Core != NULL) delete Core;
	if (Intf != NULL) delete Intf;
}

void __fastcall TBoxon::Panel_InitStart(void) {
	PanelInterface->Visible = false;
	PanelInterface->Align = alNone;
}

void __fastcall TBoxon::DisableTimer(void) {
	if (TimerLoop->Enabled) T1 = true;
	if (TimerTile->Enabled) T2 = true;
}

//---------------------------------------------------------------------------
//      Timer
//---------------------------------------------------------------------------
void __fastcall TBoxon::TimerLoopTimer(TObject *Sender)
{
	UnicodeString rep, song, title;

	if (T1) { TimerLoop->Enabled = T1 = false; return; }

	try {
		if (Core->TvNext->NR->CountSongAll == 0 ) return;
		Core->Mix->Auto();
		PbPosition->Position = (int) ( (double) (Core->Mix->Position * PbPosition->Max / Core->Mix->Current->MsLength) );

		if (Core->Mix->Current->Node != NULL) {
			rep = Core->Mix->Current->Node->Path;
			song = ExtractFileName(rep);
			rep = ExtractFileName(rep.SubString(1,rep.Length()-1-song.Length()));
			int i = ExtractFileExt(song).Length();
			title = "     [" + rep + "] - " + song.SubString(1,song.Length()-i);
			Caption = title;
			Boxmin->Title = title;
		}
	} catch (...) {
		TimerLoop->Enabled = false;
	}
}
void __fastcall TBoxon::TimerTileTimer(TObject *Sender)
{
	TDateTime duree, reste;

	if (T2) { TimerTile->Enabled = T2 = false; return; }

	try {
		if (Core->Mix->Current == NULL) return;
		duree = IncMilliSecond(TDateTime(0),Core->Mix->Current->MsLength);
		reste = IncMilliSecond(TDateTime(0),Core->Mix->Current->MsLength - Core->Mix->Current->Position);
		LbTimeLeft->Caption = FormatDateTime(L"nn:ss",reste);
//		Label1->Caption = "Song: " + UnicodeString(Core->TvNext->NR->CountSong);
//		Label2->Caption = "SongAll: " + UnicodeString(Core->TvNext->NR->CountSongAll);
//		Label3->Caption = "Folder: " + UnicodeString(Core->TvNext->NR->CountFolder);
//		Label4->Caption = "Child: " + UnicodeString(Core->TvNext->NR->Child->Count);
	} catch (...) {
		TimerTile->Enabled = false;
	}
}

//---------------------------------------------------------------------------
//      Panel Interface
//---------------------------------------------------------------------------
void __fastcall TBoxon::ToolButton2Click(TObject *Sender)
{
    Boxmin->Show();
}

void __fastcall TBoxon::FormResize(TObject *Sender)
{
	static bool max;
	static int LWidth, oLWidth;
	static int RWidth, oRWidth;

	static int WHeight, oWHeight;
	static int WWidth, oWWidth;

	static int NHeight, oNHeight;
	static int PHeight, oPHeight;

	if (max) {
		max = false;
		PanelList->Width = LWidth;
		PanelRight->Width = RWidth;
		PanelNext->Height = NHeight;
		PanelPast->Height = PHeight;
		Width = WWidth;
		Height = WHeight;
		Panel_FontSize();
		return;
	}

	if ( WindowState == wsMaximized ) {
		LWidth = oLWidth;
		RWidth = oRWidth;

		WHeight = oWHeight;
		WWidth = oWWidth;
		NHeight = oNHeight;
		PHeight = oPHeight;
		max = true;
	}
	Panel_Proportion();
	Panel_FontSize();
	oLWidth = PanelList->Width;
	oRWidth = PanelRight->Width;
	oWWidth = Width;
	oWHeight = Height;
	oNHeight = PanelNext->Height;
	oPHeight = PanelPast->Height;
}

void __fastcall TBoxon::Panel_Proportion(void) {
	static int w = Width;
	static int h = PanelList->Height;
	float px, py;
	int fontsize;
	int hh;

	if (!Created) return;
	if (!Core->Settings->PanelResize) return;

	if (OWidth != Width) {
		px = float(Width) / OWidth;
		PanelList->Width = int(PanelList->Width * px);
		OWidth = Width;
	}

	if (OHeight != Height) {
		py = float(Height) / OHeight;
		PanelNext->Height = int(PanelNext->Height * py + 0.5);
		PanelPast->Height = int(PanelPast->Height * py + 0.5);
		OHeight = Height;
	}
}

void __fastcall TBoxon::Panel_FontSize(void) {
	static int w = Width;
	int fontsize;

	fontsize = max(Core->Settings->FontSizeMin ,int (Width/110) + 1);
	fontsize = min(Core->Settings->FontSizeMax ,fontsize);
	Core->FontSize = fontsize;
}

void __fastcall TBoxon::Panel9Resize(TObject *Sender)
{
	Core->Mix->Wave->NeedRefresh = true;
}

//---------------------------------------------------------------------------
//      Button
//-------------------------------------------------------------------------

void __fastcall TBoxon::TbScanClick(TObject *Sender)
{
	Options->Visible = !Options->Visible;
}
void __fastcall TBoxon::TbArboClick(TObject *Sender)
{
	Core->TvList->RollArboFlat();
}

void __fastcall TBoxon::TbAzClick(TObject *Sender)
{
	Core->TvList->RollAz();
}

void __fastcall TBoxon::TbStopPlayClick(TObject *Sender)
{
	if (Core->Mix->Current->IsPlaying) {
		Core->Mix->Stop();
		TbStopPlay->ImageIndex = 1;
	} else {
		Core->Mix->Play();
		TbStopPlay->ImageIndex = 0;
	}
}

void __fastcall TBoxon::TbMixNextNowClick(TObject *Sender)
{
	TbStopPlay->ImageIndex = 0;
	Core->Mix->NextNow();
}

void __fastcall TBoxon::TbRegenClick(TObject *Sender)
{
	Core->TvNext->Shuffle();
}

void __fastcall TBoxon::TbTrashClick(TObject *Sender)
{
	Core->TvNext->Randomize();
}

void __fastcall TBoxon::ToolButton4Click(TObject *Sender)
{
	Core->TvPlayList->AddPlayList();
}

//---------------------------------------------------------------------------
//          Interface
//---------------------------------------------------------------------------
void __fastcall TBoxon::SbVolumeChange(TObject *Sender)
{
	Core->Volume = 65535 - Boxon->SbVolume->Position;
}

void __fastcall TBoxon::PbPositionMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
	Core->Mix->Current->Position = (int) ( (double) (X * Core->Mix->Current->MsLength / PbPosition->Width) );
}

//---------------------------------------------------------------------------
//               DRAG DROP Corbeille
//---------------------------------------------------------------------------
void __fastcall TBoxon::TbTrashDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	XNode *n;
	if (Source->InheritsFrom(__classid(XTvNext))) {
		TTreeNode *source = ((XTvNext*)Source)->Selected;
		if (source != NULL) {
			if ( ( Core->TvNext->GetNode(source)->NodeType == NODETYPE::SONG ) || ( Core->TvNext->GetNode(source)->NodeType == NODETYPE::FOLDER ) ){
				Accept = true;
				return;
			}
		}
	}
	if (Source->InheritsFrom(__classid(XTvPlayList))) {
		TTreeNode *source = ((XTvPlayList*)Source)->Selected;
		if (source != NULL) {
			if ( ( Core->TvPlayList->GetNode(source)->NodeType == NODETYPE::SONG ) || ( Core->TvPlayList->GetNode(source)->NodeType == NODETYPE::FOLDER ) ){
				Accept = true;
				return;
			}
		}
	}
	Accept = false;
}

void __fastcall TBoxon::TbTrashDragDrop(TObject *Sender, TObject *Source, int X, int Y)
{
	TTreeNode *source;
	unsigned i;

	if (Source->InheritsFrom(__classid(XTvNext))) {
		Core->TvNext->Items->BeginUpdate();
		for ( i = ((XTvNext*)Source)->SelectionCount; i > 0  ; i--) {
			source = ((XTvNext*)Source)->Selections[i-1];
			if (source == NULL) return;
			Core->TvNext->Remove(source);
			Core->Mix->Next->Node = Core->TvNext->Next;
		}
		Core->TvNext->Fill();
		Core->TvNext->Items->EndUpdate();
	}
	if (Source->InheritsFrom(__classid(XTvPlayList))) {
		Core->TvPlayList->Items->BeginUpdate();
		for ( i = ((XTvNext*)Source)->SelectionCount; i > 0  ; i--) {
			source = ((XTvPlayList*)Source)->Selections[i-1];
			if (source == NULL) return;
			Core->TvPlayList->Remove(source);
		}
		Core->TvPlayList->Items->EndUpdate();
	}
}

//---------------------------------------------------------------------------
//               DRAG DROP Search
//---------------------------------------------------------------------------
void __fastcall TBoxon::TbAzDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept) {
	XNode *n;
	if (Source->InheritsFrom(__classid(XTvNext))) {
		TTreeNode *source = ((XTvNext*)Source)->Selected;
		if (source != NULL) {
			if (Core->TvNext->GetNode(source)->NodeType == NODETYPE::SONG ) {
				Accept = true;
				return;
			}
		}
	}
	Accept = false;
}

void __fastcall TBoxon::TbAzDragDrop(TObject *Sender, TObject *Source, int X, int Y)
{
	TTreeNode *source = ((XTvNext*)Source)->Selected;
	if (source == NULL) return;
}

//---------------------------------------------------------------------------

void __fastcall TBoxon::Splitter2Moved(TObject *Sender)
{
	ShowScrollBar(Core->TvNext->Handle, SB_HORZ, false);
	ShowScrollBar(Core->TvPlayList->Handle, SB_HORZ, false);
}
//---------------------------------------------------------------------------

void __fastcall TBoxon::Splitter3Moved(TObject *Sender)
{
	ShowScrollBar(Core->TvPast->Handle, SB_HORZ, false);
	ShowScrollBar(Core->TvPlayList->Handle, SB_HORZ, false);
}
//---------------------------------------------------------------------------


void __fastcall TBoxon::Splitter1Moved(TObject *Sender)
{
	ShowScrollBar(Core->TvNext->Handle, SB_HORZ, false);
	ShowScrollBar(Core->TvPlayList->Handle, SB_HORZ, false);
	ShowScrollBar(Core->TvList->Handle, SB_HORZ, false);
	ShowScrollBar(Core->TvPast->Handle, SB_HORZ, false);
}
//---------------------------------------------------------------------------

void __fastcall TBoxon::Label1Click(TObject *Sender)
{
//    Core->Mix->ToggleNormalize();
}
//---------------------------------------------------------------------------

