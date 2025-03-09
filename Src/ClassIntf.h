//---------------------------------------------------------------------------

#ifndef ClassIntfH
#define ClassIntfH

class XIntf;

#include "ClassCore.h"
#include "Boxon.h"
#include "ClassDropFolder.h"
//---------------------------------------------------------------------------

class XIntf {
	int    PMinTop;
	int    PMinLeft;
	TBoxon *Boxon;

	HWND 	__fastcall GetWinHandle(void);
	int  	__fastcall GetWinTop(void);
	int  	__fastcall GetWinLeft(void);
	int  	__fastcall GetWinWidth(void);
	int  	__fastcall GetWinHeight(void);

	int  	__fastcall GetMinTop(void);
	int  	__fastcall GetMinLeft(void);

	int  	__fastcall GetListWidth(void);
	int  	__fastcall GetNextHeight(void);
	int  	__fastcall GetPastHeight(void);

	int  	__fastcall GetSbVolume(void);

	TPanel* __fastcall GetPanelList(void);
	TPanel* __fastcall GetPanelNext(void);
	TPanel* __fastcall GetPanelPast(void);
	TPanel* __fastcall GetPanelPlayList(void);

	TImage* __fastcall GetImgSpectrum(void);
	TImage* __fastcall GetImgWave(void);

	TCustomImageList* __fastcall GetTvIcones(void);

	void    __fastcall SetWinCaption(UnicodeString text);
	void    __fastcall SetTimers(bool b);
	void 	__fastcall SetArboButton(int mode);
	void 	__fastcall SetAzButton(int mode);
	void  	__fastcall SetMinTop(int top);
	void  	__fastcall SetMinLeft(int left);

	void  	__fastcall SetWinTop(int top);
	void  	__fastcall SetWinLeft(int left);
	void  	__fastcall SetWinWidth(int width);
	void  	__fastcall SetWinHeight(int height);

	void 	__fastcall SetListWidth(int width);
	void 	__fastcall SetNextHeight(int height);
	void 	__fastcall SetPastHeight(int height);


public:
	XCore  *Core;
	XDropFolder *ImgDropFolder;

	__fastcall XIntf(TBoxon *boxon);
	__fastcall ~XIntf(void);

	void __fastcall LoadingAnimation(bool loading);
	void __fastcall WorkingAnimation(bool work);
	void __fastcall DropFolder(bool dropfolder);
	void __fastcall StopPlay(void);

	__property TCustomImageList*  TvIcones      = { read = GetTvIcones};
	__property bool               Timers        = {                       	write = SetTimers};

	__property HWND               WinHandle     = { read = GetWinHandle};
	__property int                WinTop        = { read = GetWinTop,       write = SetWinTop};
	__property int                WinLeft       = { read = GetWinLeft,      write = SetWinLeft};
	__property int                WinWidth      = { read = GetWinWidth,     write = SetWinWidth};
	__property int                WinHeight     = { read = GetWinHeight,    write = SetWinHeight};
	__property UnicodeString      WinCaption    = {                       	write = SetWinCaption};

	__property int                MinTop        = { read = GetMinTop, 		write = SetMinTop};
	__property int                MinLeft       = { read = GetMinLeft,		write = SetMinLeft};

	__property int                ListWidth     = { read = GetListWidth,    write = SetListWidth};
	__property int                NextHeight    = { read = GetNextHeight,   write = SetNextHeight};
	__property int                PastHeight    = { read = GetPastHeight,   write = SetPastHeight};

	__property TPanel*            PanelList     = { read = GetPanelList};
	__property TPanel*            PanelNext     = { read = GetPanelNext};
	__property TPanel*            PanelPast     = { read = GetPanelPast};
	__property TPanel*            PanelPlayList = { read = GetPanelPlayList};

	__property TImage*            ImgSpectrum   = { read = GetImgSpectrum};
	__property TImage*            ImgWave       = { read = GetImgWave};

	__property int                SbVolume      = { read = GetSbVolume};
	__property int                ArboButton    = { 	  					write = SetArboButton};
	__property int                AzButton      = { 	  	  				write = SetAzButton};
};

#endif

