//---------------------------------------------------------------------------
#ifndef ClassWaveH
#define ClassWaveH

class XWave;

#include "Bass.h"
#include "ClassNode.h"
#include "ClassPlayers.h"

//---------------------------------------------------------------------------
class XWave {
	XCore  *Core;
	TImage *PImage;

	void __fastcall SetImage(TImage *image);

public:
	bool NeedRefresh;

	 XWave(XCore *core);
	 ~XWave(void);

	void __fastcall Scan(XNode *node);
	void __fastcall Refresh(XNode *node);

	__property TImage   *Image = { read = PImage,    write = SetImage};
};

//---------------------------------------------------------------------------
class ThreadWaveScan : public TThread
{
private:
	const DWORD MAX_PEAK = 32767;
	const DWORD WAVE_WIDTH = 1000;

	XCore    *Core;
	XNode    *PNode;

	// Bass
	HSTREAM	  decoder;

	// Dx
	//IGraphBuilder *Pigb;
//	IMediaControl *Pimc;
//	IMediaEventEx *Pimex;
//	IBasicAudio   *Piba;
	//IMediaSeeking *Pims;
    bool      loaded;
	WORD      peak0;
	WORD      peak1;
	WORD      peakmax;
	DWORD     cpos;
	DWORD     level;
	DWORD     bpp;
	QWORD     pos;
	DWORD     position;
	float     pondpeak;
	unsigned  pondcount;
	unsigned __int64 length; // ms
	unsigned  counter;
	unsigned  w;
	unsigned  adblevel;
	int       fadein;  // ms
	int       fadeout; // ms
	DWORD     *wave;
	WCHAR     *path;

	void __fastcall Done(void);
	void __fastcall UpdatePNode(void);

protected:
	void __fastcall Execute();
	void __fastcall WaveBass(void);
	void __fastcall WaveDirectShow(void);

public:
	__fastcall ThreadWaveScan(XCore *core, XNode *node, bool CreateSuspended);
};
#endif

