//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassSettings.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   XSettings
// ---------------------------------------------------------------------------
__fastcall XSettings::XSettings(XCore *core) {
	int pos;

	PCreated       = false;
	Core           = core;
	Intf           = Core->Intf;
	AutoSaveToKeep = 7;
	FontLimitMin    = 6;
	FontLimitMax    = 40;

	if ( !FileExists("bass.dll") ) {
		MessageDlg("File bass.dll not found." ,mtInformation,  TMsgDlgButtons() << mbOK , 1);
		return;
	}

	PSettingPath   = L"C:\\Users\\Public\\Documents\\Boxon";
	PBackupPath    = PSettingPath + L"\\Backup";
	PSettingFile   = L"Config.ini";
	if ( !DirectoryExists(PSettingPath) ) CreateDir(PSettingPath);
	if ( !DirectoryExists(PBackupPath) ) CreateDir(PBackupPath);
	PAppPath       = ExtractFilePath(Application->ExeName);

	if ( (pos = PAppPath.Pos("\\Source\\.\\Win32\\Debug\\" )) > 0 ) PAppPath = PAppPath.SubString(1,pos-1);
	if ( (pos = PAppPath.Pos("\\Source\\Win32\\Debug\\" )) > 0 )  	PAppPath = PAppPath.SubString(1,pos);
	if ( (pos = PAppPath.Pos("\\Bin\\" )) > 0 )           			PAppPath = PAppPath.SubString(1,pos);

	AutoSelect      = 3;

	//[Arbo]
	ArboViewMode    = 0;
	ArboAzMode      = 0;

	// Mix
	FadeInDuration  = 1000;
	FadeOutDuration = 2000;
	AdbPercent      = 10;

	//[Settings]
	MusicPath       = "";
	Volume          = 6489;
	FontSizeMin     = 8;
	FontSizeMax     = 40;
	FontResize      = true;
	PanelResize     = true;
	Engine          = "Bass";

	ColorList       = clSkyBlue;
	ColorNext       = clLime;
	ColorPast       = clGray;
	ColorPlaylist   = clGray;

	Load();
}

__fastcall XSettings::~XSettings(void) {
}

bool __fastcall XSettings::Save(void) {
	bool result = true;

	if (Core == NULL) return false;

	if ( !DirectoryExists(PSettingPath) ) CreateDir(PSettingPath);
	TIniFile* IniFile = new TIniFile(PSettingPath + L"\\" + PSettingFile);

	try
	{
		IniFile->WriteInteger ("Win",      "Top"             , Intf->WinTop);
		IniFile->WriteInteger ("Win",      "Left"            , Intf->WinLeft);
		IniFile->WriteInteger ("Win",      "Width"           , Intf->WinWidth);
		IniFile->WriteInteger ("Win",      "Height"          , Intf->WinHeight);

		IniFile->WriteInteger ("Min",      "Top"             , Intf->MinTop);
		IniFile->WriteInteger ("Min",      "Left"            , Intf->MinLeft);

		IniFile->WriteInteger ("List",     "Width"           , Intf->ListWidth);

		IniFile->WriteInteger ("Next",     "Height"          , Intf->NextHeight);
		IniFile->WriteInteger ("Next",     "AutoSelect"      , AutoSelect);

		IniFile->WriteInteger ("Past",     "Height"          , Intf->PastHeight);


		IniFile->WriteInteger ("Arbo",     "ViewMode"        , Core->TvList->ViewFlat);
		IniFile->WriteInteger ("Arbo",     "AzMode"          , Core->TvList->ViewAz);

		IniFile->WriteInteger ("Mix",      "AdbPercent"      , AdbPercent);
		IniFile->WriteInteger ("Mix",      "FadeInDuration"  , (int) FadeInDuration);
		IniFile->WriteInteger ("Mix",      "FadeOutDuration" , (int) FadeOutDuration);

		IniFile->WriteString  ("Settings", "Folder"          , MusicPath);
		IniFile->WriteInteger ("Settings", "Volume"          , Intf->SbVolume);
		IniFile->WriteBool    ("Settings", "PanelResize"     , PanelResize);
		IniFile->WriteString  ("Settings", "Engine"          , Engine);

		IniFile->WriteInteger ("Fonts",    "FontSizeMin"     , FontSizeMin);
		IniFile->WriteInteger ("Fonts",    "FontSizeMax"     , FontSizeMax);
		IniFile->WriteBool    ("Fonts",    "FontResize"      , FontResize);
	}
	catch(...)
	{
		result = false;
	}
	delete IniFile;
	return result;
}

bool __fastcall XSettings::Load(void) {
	if ( !DirectoryExists(PSettingPath) ) CreateDir(PSettingPath);
	TIniFile* IniFile = new TIniFile(PSettingPath + L"\\" + PSettingFile);

	try
	{
		  Intf->WinTop     = IniFile->ReadInteger("Win",      "Top"             , 150);
		  Intf->WinLeft    = IniFile->ReadInteger("Win",      "Left"            , 150);
		  Intf->WinWidth   = IniFile->ReadInteger("Win",      "Width"           , 600);
		  Intf->WinHeight  = IniFile->ReadInteger("Win",      "Height"          , 600);

		  Intf->MinTop     = IniFile->ReadInteger("Min",      "Top"             , 100);
		  Intf->MinLeft    = IniFile->ReadInteger("Min",      "Left"            , 100);

		  Intf->ListWidth  = IniFile->ReadInteger("List",     "Width"           , 420);

		  Intf->NextHeight = IniFile->ReadInteger("Next",     "Height"          , 70);
		  AutoSelect       = IniFile->ReadInteger("Next",     "AutoSelect"      , AutoSelect);

		  Intf->PastHeight = IniFile->ReadInteger("Past",     "Height"          , 50);

		  ArboViewMode     = IniFile->ReadInteger("Arbo",     "ViewMode"        , ArboViewMode);
		  ArboAzMode       = IniFile->ReadInteger("Arbo",     "AzMode"          , ArboAzMode);

		  AdbPercent       = IniFile->ReadInteger("Mix",      "AdbPercent"      , AdbPercent);
		  FadeInDuration   = IniFile->ReadInteger("Mix",      "FadeInDuration"  , FadeInDuration);
		  FadeOutDuration  = IniFile->ReadInteger("Mix",      "FadeOutDuration" , FadeOutDuration);

		  MusicPath        = IniFile->ReadString ("Settings", "Folder"          , MusicPath);
		  Volume           = IniFile->ReadInteger("Settings", "Volume"          , Volume);
		  PanelResize      = IniFile->ReadBool   ("Settings", "PanelResize"     , PanelResize);
		  Engine           = IniFile->ReadString ("Settings", "Engine"          , Engine);

		  FontSizeMin      = IniFile->ReadInteger("Fonts",    "FontSizeMin"     , FontSizeMin);
		  FontSizeMax      = IniFile->ReadInteger("Fonts",    "FontSizeMax"     , FontSizeMax);
		  FontResize       = IniFile->ReadBool   ("Fonts",    "FontResize"      , FontResize);
		  PCreated         = true;
	}
	catch(...)
	{
	}

	delete IniFile;
	return PCreated;
}


