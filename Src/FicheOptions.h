//---------------------------------------------------------------------------

#ifndef FicheOptionsH
#define FicheOptionsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.GIFImg.hpp>
#include <Vcl.Imaging.jpeg.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Dialogs.hpp>

class TOptions;

#include "ClassCore.h"
#include "ThreadFastScan.h"
//#include "ThreadScanDisk.h"

//---------------------------------------------------------------------------
class TOptions : public TForm
{
__published:	// Composants gérés par l'EDI
	TGroupBox *GroupBox1;
	TRadioButton *RbFixed;
	TRadioButton *RbResize;
	TScrollBar *ScrollBarMin;
	TScrollBar *ScrollBarMax;
	TPanel *PanelFontMin;
	TPanel *PanelFontMax;
	TLabel *LbMinSize;
	TLabel *LbMaxSize;
	TButton *Button1;
	TButton *Button2;
	TButton *Button3;
	TButton *Button4;
	TLabel *Label3;
	TLabel *LbMusicPath;
	TLabel *Label5;
	TUpDown *UpDown;
	TLabel *LbRandomTrack;
	TFileOpenDialog *FileOpenDialog;
	TGroupBox *GroupBox2;
	TCheckBox *CbPanelResize;
	TGroupBox *GroupBox3;
	TScrollBar *SbSensibility;
	TLabel *Label1;
	TLabel *Label2;
	TButton *Button5;
	TMemo *MemoInvalidAudioFiles;
	TLabel *LabelProgressValid;
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall Button4Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall RbFixedClick(TObject *Sender);
	void __fastcall RbResizeClick(TObject *Sender);
	void __fastcall ScrollBarMinChange(TObject *Sender);
	void __fastcall ScrollBarMaxChange(TObject *Sender);
	void __fastcall UpDownChangingEx(TObject *Sender, bool &AllowChange, int NewValue,
          TUpDownDirection Direction);
	void __fastcall SbSensibilityChange(TObject *Sender);
	void __fastcall CbPanelResizeClick(TObject *Sender);
	void __fastcall Button5Click(TObject *Sender);

private:	// Déclarations utilisateur
public:		// Déclarations utilisateur
	XCore *Core;

	__fastcall TOptions(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TOptions *Options;
//---------------------------------------------------------------------------
#endif
