//---------------------------------------------------------------------------

#ifndef Boxon1H
#define Boxon1H
//---------------------------------------------------------------------------

class TBoxon;

#include <System.Classes.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.GIFImg.hpp>
#include <Vcl.Imaging.jpeg.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ToolWin.hpp>
#include <Vcl.WinXCtrls.hpp>
#include <algorithm> // min max

#include "ClassCore.h"
#include "ClassIntf.h"
#include "FicheBoxmin.h"

//---------------------------------------------------------------------------
class TBoxon : public TForm
{
__published:	// Composants gérés par l'EDI
	TTimer          *TimerLoop;
	TImageList      *ILTv;
	TPanel          *Panel1;
	TPanel          *PanelInterface;
	TPanel          *PanelList;
	TSplitter       *Splitter1;
	TPanel          *PanelRight;
	TFileOpenDialog *FolderSelect;
	TImageList      *IlBar3;
	TImageList      *IlBar1;
	TPanel          *Panel4;
	TToolBar        *ToolBar2;
	TToolButton     *TbScan;
	TToolButton     *TbArbo;
	TToolButton     *TbAz;
	TImageList      *IlBar2;
	TTimer          *TimerTile;
	TPanel          *Panel3;
	TImage          *ImgWave;
	TPanel          *Panel8;
	TToolBar        *ToolBar3;
	TToolButton     *TbStopPlay;
	TToolButton     *TbMixNextNow;
	TToolButton     *TbRegen;
	TPanel          *Panel9;
	TPanel          *PanelNext;
	TSplitter       *Splitter2;
	TPanel          *PanelPlayList;
	TSplitter       *Splitter3;
	TPanel          *PanelPast;
	TLabel          *LbTimeLeft;
	TPanel          *Panel11;
	TImage          *ImgSpectrum;
	TProgressBar    *PbPosition;
	TScrollBar *SbVolume;
	TToolBar        *ToolBar1;
	TToolButton     *ToolButton4;
	TToolButton     *TbTrash;
	TPanel          *Panel10;
	TImage          *Image5;
	TToolBar        *ToolBar4;
	TToolButton     *ToolButton2;
	TImage          *ImageLoading;
	TLabel          *Label1;
	TLabel          *Label2;
	TLabel          *Label3;
	TLabel          *Label4;
	TActivityIndicator *ActivityIndicator1;

	void __fastcall TimerLoopTimer(TObject *Sender);
	void __fastcall PbPositionMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall TbTrashDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept);
	void __fastcall TbTrashDragDrop(TObject *Sender, TObject *Source, int X, int Y);
	void __fastcall TbScanClick(TObject *Sender);
	void __fastcall TbArboClick(TObject *Sender);
	void __fastcall TbStopPlayClick(TObject *Sender);
	void __fastcall TbMixNextNowClick(TObject *Sender);
	void __fastcall TbRegenClick(TObject *Sender);
	void __fastcall Panel9Resize(TObject *Sender);
	void __fastcall TbAzDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept);
	void __fastcall TbAzDragDrop(TObject *Sender, TObject *Source, int X, int Y);
	void __fastcall TimerTileTimer(TObject *Sender);
	void __fastcall TbAzClick(TObject *Sender);
	void __fastcall ToolButton4Click(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall SbVolumeChange(TObject *Sender);
	void __fastcall TbTrashClick(TObject *Sender);
	void __fastcall ToolButton2Click(TObject *Sender);
	void __fastcall Splitter2Moved(TObject *Sender);
	void __fastcall Splitter3Moved(TObject *Sender);
	void __fastcall Splitter1Moved(TObject *Sender);
	void __fastcall Label1Click(TObject *Sender);

private:	// Déclarations utilisateur
	int OWidth, OHeight;
	bool Created, T1, T2;

	void __fastcall Panel_InitStart(void);
	void __fastcall Panel_Proportion(void);

public:		// Déclarations utilisateur
	XCore *Core;
	XIntf *Intf;

	__fastcall TBoxon(TComponent* Owner);

	void __fastcall SwapMaxMin(void);
	void __fastcall Panel_FontSize(void);
	void __fastcall DisableTimer(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TBoxon *Boxon;
//---------------------------------------------------------------------------
#endif
