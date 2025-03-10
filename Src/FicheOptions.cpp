//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FicheOptions.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOptions *Options;
//---------------------------------------------------------------------------
__fastcall TOptions::TOptions(TComponent* Owner)
	: TForm(Owner)
{
	Core = Boxon->Core;
	Core->Options = this;
	UpDown->Position = Core->Settings->AutoSelect;

	ScrollBarMin->Min      = Core->Settings->FontLimitMin;
	ScrollBarMin->Max      = Core->Settings->FontLimitMax;
	ScrollBarMin->Position = Core->Settings->FontSizeMin;
	ScrollBarMin->OnChange = ScrollBarMinChange;

	ScrollBarMax->Min      = Core->Settings->FontLimitMin;
	ScrollBarMax->Max      = Core->Settings->FontLimitMax;
	ScrollBarMax->Position = Core->Settings->FontSizeMax;
	ScrollBarMax->OnChange = ScrollBarMaxChange;

	CbPanelResize->Checked = Core->Settings->PanelResize;

	SbSensibility->Position = Core->Settings->AdbPercent;
	SbSensibility->OnChange = SbSensibilityChange;

	LbRandomTrack->Caption = UnicodeString(Core->Settings->AutoSelect) + " Random Track(s) preloaded.";

	LbMusicPath->Caption   = Core->Settings->MusicPath;
	LbMinSize->Caption     = "Minimum Size: ";
	LbMaxSize->Caption     = "Maximum Size: ";
	PanelFontMin->Font->Size = Core->Settings->FontSizeMin;
	PanelFontMax->Font->Size = Core->Settings->FontSizeMax;

	if (!Core->Settings->FontResize) RbFixed->Checked = true;
}


//---------------------------------------------------------------------------

void __fastcall TOptions::Button1Click(TObject *Sender)
{
	if (Core->Loader->Loading) {
		MessageDlg("Loading already in progress." ,mtInformation,  TMsgDlgButtons() << mbOK , 1);
		return;

	} else {
		FileOpenDialog->Options << fdoPickFolders;
		FileOpenDialog->Execute();
		if (DirectoryExists(FileOpenDialog->FileName)) {
			Core->Loader->Loaded  = false;
            Core->SavFile->ArchiveArbo();
			Core->ModifyFolderPath(FileOpenDialog->FileName);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TOptions::Button3Click(TObject *Sender)
{
	if (Core->Loader->Loading) {
		MessageDlg("Loading already in progress." ,mtInformation,  TMsgDlgButtons() << mbOK , 1);
		return;
	} else if ( !Core->Loader->Loaded ) {
		MessageDlg("Select Music Folder First..." ,mtInformation,  TMsgDlgButtons() << mbOK , 1);
		return;
	} else {
		FileOpenDialog->Options >> fdoPickFolders;
		if ( FileOpenDialog->Execute() ) {
			if ( FileExists(FileOpenDialog->FileName) && (FileOpenDialog->FileName != Core->Settings->SettingPath+"\\PlayList.sav" ) )
				Core->LoadPlayList(FileOpenDialog->FileName);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TOptions::Button4Click(TObject *Sender)
{
	if (Core->Loader->Loading) {
		MessageDlg("Loading already in progress." ,mtInformation,  TMsgDlgButtons() << mbOK , 1);
		return;

	} else {
		if (DirectoryExists(Core->Settings->MusicPath)) {
			Core->Loader->Loaded  = false;
            Core->SavFile->ArchiveArbo();
			Core->ModifyFolderPath(Core->Settings->MusicPath);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TOptions::Button2Click(TObject *Sender)
{
	if (Core->Loader->Loading) {
		MessageDlg("Loading in progress. Wait finish to load please." ,mtInformation,  TMsgDlgButtons() << mbOK , 1);
		return;
	} else {
		FileOpenDialog->Options >> fdoPickFolders;
		if ( FileOpenDialog->Execute() ) {
				Core->SavFile->SavePlaylist(FileOpenDialog->FileName);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TOptions::RbFixedClick(TObject *Sender)
{
	LbMaxSize->Enabled = false;
	ScrollBarMax->Enabled = false;
	PanelFontMax->Enabled = false;
	Core->Settings->FontResize = false;
	Boxon->Panel_FontSize();
}
//---------------------------------------------------------------------------

void __fastcall TOptions::RbResizeClick(TObject *Sender)
{
	LbMaxSize->Enabled = true;
	ScrollBarMax->Enabled = true;
	PanelFontMax->Enabled = true;
	Core->Settings->FontResize = true;
	Boxon->Panel_FontSize();
}
//---------------------------------------------------------------------------

void __fastcall TOptions::ScrollBarMinChange(TObject *Sender)
{
	PanelFontMin->Font->Size = ScrollBarMin->Position;
	if (ScrollBarMin->Position > ScrollBarMax->Position) ScrollBarMax->Position = ScrollBarMin->Position;
	Core->Settings->FontSizeMin = ScrollBarMin->Position;
	Core->Settings->FontSizeMax = ScrollBarMax->Position;
	Boxon->Panel_FontSize();
}
//---------------------------------------------------------------------------

void __fastcall TOptions::ScrollBarMaxChange(TObject *Sender)
{
	PanelFontMax->Font->Size = ScrollBarMax->Position;
	if (ScrollBarMax->Position < ScrollBarMin->Position) ScrollBarMin->Position = ScrollBarMax->Position;
	Core->Settings->FontSizeMin = ScrollBarMin->Position;
	Core->Settings->FontSizeMax = ScrollBarMax->Position;
	Boxon->Panel_FontSize();
}
//---------------------------------------------------------------------------


void __fastcall TOptions::UpDownChangingEx(TObject *Sender, bool &AllowChange, int NewValue,
          TUpDownDirection Direction)
{
	if ( (NewValue >= UpDown->Min) && (NewValue <= UpDown->Max) ) {
		Core->Settings->AutoSelect = NewValue;
		LbRandomTrack->Caption = UnicodeString(NewValue) + " Random Track(s) preloaded";
		Core->TvNext->Fill();
	}
}
//---------------------------------------------------------------------------

void __fastcall TOptions::SbSensibilityChange(TObject *Sender)
{
	Core->Settings->AdbPercent = SbSensibility->Position;
}
//---------------------------------------------------------------------------

void __fastcall TOptions::CbPanelResizeClick(TObject *Sender)
{
	Core->Settings->PanelResize = CbPanelResize->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TOptions::Button5Click(TObject *Sender)
{
	XNode *node;

	LabelProgressValid->Caption = "Debut du Scan";
	Core->Mix->Stop();
	for (int i = 0; i < (Core->TvList->NRSong->Child->Count); i++) {
		node = Core->TvList->NRSong->ChildAt(i);
		if ( Core->Mix->Test->IsAudioFile(node) ) {
			LabelProgressValid->Caption = UnicodeString(i);
		} else {
			MemoInvalidAudioFiles->Lines->Add( node->Path );
		}

	}
}
//---------------------------------------------------------------------------

