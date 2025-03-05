//---------------------------------------------------------------------------

#ifndef ClassSettingsH
#define ClassSettingsH
#include <System.Classes.hpp>
#include <System.SyncObjs.hpp>

class XSettings;

#include "ClassCore.h"
#include "ClassIntf.h"

//---------------------------------------------------------------------------
class XSettings {
	bool          PCreated;
	XCore        *Core;
	XIntf        *Intf;
	UnicodeString PSettingPath;
	UnicodeString PBackupPath;
	UnicodeString PSettingFile;
	UnicodeString PAppPath;

	friend XIntf;
public:
	int           FontLimitMin;
	int           FontLimitMax;
	int           AutoSaveToKeep;

	int 		  AutoSelect;
	// [Arbo]
	int 		  ArboViewMode;
	int 		  ArboAzMode;
	// [Mix]
	int           AdbPercent;
	unsigned __int64 FadeInDuration;
	unsigned __int64 FadeOutDuration;
	// [Settings]
	int 		  Volume;
	UnicodeString MusicPath;
	UnicodeString Engine;
	// [Fonts]
	int 		  FontSizeMin;
	int 		  FontSizeMax;
	bool          FontResize;
    bool          PanelResize;

	TColor         ColorList;
	TColor         ColorNext;
	TColor         ColorPast ;
	TColor         ColorPlaylist;

	__fastcall XSettings(XCore *core);
	__fastcall ~XSettings(void);

	bool __fastcall Load(void);
	bool __fastcall Save(void);

	__property bool 			Created		= {read = PCreated};
	__property UnicodeString 	SettingPath	= {read = PSettingPath};
	__property UnicodeString 	BackupPath	= {read = PBackupPath};

};
#endif
