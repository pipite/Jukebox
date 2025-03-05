//---------------------------------------------------------------------------

#ifndef ClassWaveH
#define ClassWaveH

//#include <Vcl.ComCtrls.hpp>

#include <windows.h>
#include <mmsystem.h>
#include <strmif.h>
#include <control.h>
#include <uuids.h>
#include <Qedit.h>

class XWave;
class ThreadScanWave;

#include "ClassPlayers.h"
#include "ClassNode.h"
#include "Bass.h"

//---------------------------------------------------------------------------
class XWave {
	XCore  *Core;
	TImage *PImage;

	void __fastcall SetImage(TImage *image);

public:
	bool NeedRefresh;

	 __fastcall XWave(XCore *core);
	 __fastcall ~XWave(void);

	void __fastcall Scan(XNode *node);
	void __fastcall Refresh(XNode *node);

	__property TImage   *Image = { read = PImage,    write = SetImage};
};

//---------------------------------------------------------------------------
class ThreadWaveScan : public TThread
{
private:
	XCore    *Core;
	XNode    *PNode;

	// Bass
	HSTREAM	  decoder;

	// Dx
	IGraphBuilder *Pigb;
//	IMediaControl *Pimc;
//	IMediaEventEx *Pimex;
//	IBasicAudio   *Piba;
	IMediaSeeking *Pims;

	DWORD     peak0;
	DWORD     peak1;
	DWORD     peakmax;
	DWORD     cpos;
	DWORD     level;
	DWORD     bpp;
	QWORD     pos;
	DWORD     position;
	float     pondpeak;
	unsigned  pondcount;
	unsigned __int64 length;    // ms
	unsigned  counter;
	unsigned  w;
	unsigned  adblevel;
	int       fadein;    // ms
	int       fadeout;   // ms
	DWORD     *wave;


	void __fastcall Done(void);

protected:
	void __fastcall Execute();
	void __fastcall WaveBass(void);
	void __fastcall WaveDirectShow(void);

public:
	__fastcall ThreadWaveScan(XCore *core, XNode *node, bool CreateSuspended);
};
#endif

