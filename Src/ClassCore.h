//---------------------------------------------------------------------------
#ifndef ClassCoreH
#define ClassCoreH

class XCore;

#include "Bass.h"
#include "FicheOptions.h"
#include "ClassMix.h"
#include "ClassTreeViewList.h"
#include "ClassTreeViewNext.h"
#include "ClassTreeViewPast.h"
#include "ClassTreeViewPlayList.h"
#include "ClassSavFile.h"
#include "ClassLoader.h"
#include "ClassSettings.h"
#include "ClassIntf.h"
#include "ThreadScanDisk.h"

//---------------------------------------------------------------------------
class XCore {
	bool               PCreated;
	UnicodeString      PPanelListMode;
	//TMutex            *Mtx;
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

	XCore(XIntf *intf);
	~XCore(void);

	void __fastcall LOG(UnicodeString module, const UnicodeString function, const UnicodeString e);
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
