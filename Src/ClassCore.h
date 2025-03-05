//---------------------------------------------------------------------------

#ifndef ClassCoreH
#define ClassCoreH

//#include <System.Classes.hpp>
#include <System.DateUtils.hpp>
#include <System.IniFiles.hpp>
#include <System.IOUtils.hpp>
#include <Vcl.ComCtrls.hpp>
#include <vcl.h>
#include <MMsystem.h>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ImgList.hpp>
#include <System.Win.TaskbarCore.hpp>
#include <Vcl.Taskbar.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.ButtonGroup.hpp>
#include <System.Actions.hpp>
#include <System.Generics.Collections.hpp>
#include <System.SysUtils.hpp>
#include <Vcl.Imaging.GIFImg.hpp>
#include <time.h>
#include <windows.h>

#include <vector>
#include <list>
#include <fstream>

class XCore;

//#include "FicheBoxmin.h"
#include "FicheOptions.h"

#include "ClassMix.h"
#include "ThreadScanDisk.h"
#include "ClassTreeViewList.h"
#include "ClassTreeViewNext.h"
#include "ClassTreeViewPast.h"
#include "ClassTreeViewPlayList.h"
#include "ClassSavFile.h"
#include "ClassLoader.h"
#include "ClassSettings.h"
#include "ClassIntf.h"

//---------------------------------------------------------------------------
class XCore {
	bool               PCreated;
	UnicodeString      PPanelListMode;
	TMutex            *Mtx;
	XMix              *PMix;
	XFile             *PSavFile;
	int                PWorkingLevel;
	int                PVolume;

	void __fastcall SetPanelListMode(UnicodeString visual);
	bool __fastcall GetIsValidFolderPath(void);
	void __fastcall SetFontSize(int size);
	int  __fastcall GetFontSize(void);
	void __fastcall SetVolume(int value);
    void __fastcall SetNodeRoot(void);

public:
	XIntf       *Intf;
	TOptions    *Options;
	XSettings   *Settings;
	XLoader     *Loader;

	XTvList     *TvList;
	XTvNext     *TvNext;
	XTvPast     *TvPast;
	XTvPlayList *TvPlayList;

	int          PFontSize;

	__fastcall XCore(XIntf *intf);
	__fastcall ~XCore(void);

	void __fastcall RefreshTv(void);
	void __fastcall ModifyFolderPath(UnicodeString folderpath);
	void __fastcall LoadPlayList(UnicodeString path);
	void __fastcall SaveSettings(void);
	void __fastcall LoadSettings(void);
	void __fastcall SetWorking(bool work);

	__property	UnicodeString      PanelListMode     = {read  = PPanelListMode, write = SetPanelListMode};
	__property	bool               Created           = {read  = PCreated};
	__property	int                FontSize			 = {read  = PFontSize,      write = SetFontSize};
	__property	XMix              *Mix               = {read  = PMix};
	__property	int                Volume            = {read  = PVolume,        write = SetVolume};
	__property	XFile             *SavFile           = {read  = PSavFile};
	__property	bool               Working           = {write = SetWorking};
	__property	bool               IsValidFolderPath = {read  = GetIsValidFolderPath};
};

#endif
