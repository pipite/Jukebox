//---------------------------------------------------------------------------

#ifndef ThreadScanWaveH
#define ThreadScanWaveH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>

class ThreadScanWave;

#include "ClassCore.h"

//---------------------------------------------------------------------------
class ThreadScanWave : public TThread
{
private:
	XCore    *Core;
	XNode    *PNode;

	HSTREAM	  decoder;  // Bass

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
	QWORD     length;
	unsigned  counter;
	unsigned  w;
	unsigned  adblevel;
	int       fadein;
	int       fadeout;
	DWORD     *wave;


	void __fastcall Done(void);

protected:
	void __fastcall Execute();
	void __fastcall GetBassWave(void);
	void __fastcall UpdateMsFade(void);

public:
	__fastcall ThreadScanWave(XCore *core, XNode *node, bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
