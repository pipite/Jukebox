//---------------------------------------------------------------------------

#ifndef ClassMixH
#define ClassMixH

class XMix;

#include "ClassCore.h"
#include "ClassPlayers.h"
#include "ClassSpectrum.h"
#include "ClassWave.h"
#include "ClassNode.h"

//---------------------------------------------------------------------------
class XMix {
	HWND             Handle;
	XCore           *Core;
	XPlayers        *PLast;
	XPlayers        *PCurrent;
	XPlayers        *PNext;
	XPlayers        *PTmp;
	XPlayers        *PTest;
	XSpectrum       *PSpectrum;
	XWave           *PWave;
	bool             PIsMixing;
	unsigned __int64 PMsMixLength;
	int              PAdbPercent;

	bool             __fastcall GetIsTimeToStartMix(void);
	unsigned __int64 __fastcall GetPosition(void);
	void             __fastcall Roll(void);
	void             __fastcall CrossFade(void);
	void             __fastcall ToggleNormalize(void);

	__property bool             IsTimeToStartMix = { read = GetIsTimeToStartMix};
	__property unsigned __int64 MsMixLength	     = { read = PMsMixLength, write = PMsMixLength};

public:

		__fastcall XMix(HWND handle, XCore *core);
		__fastcall ~XMix(void);

	void __fastcall Init(void);
	void __fastcall Stop(void);
	void __fastcall Auto(void);
	void __fastcall Play(void);
	void __fastcall NextNow(void);
	void __fastcall NodeNow(XNode *node);

	__property bool             IsMixing  = { read = PIsMixing};
	__property unsigned __int64 Position  = { read = GetPosition};
	__property XPlayers        *Last      = { read = PLast};
	__property XPlayers        *Current   = { read = PCurrent};
	__property XPlayers        *Next      = { read = PNext};
	__property XPlayers        *Test      = { read = PTest};
	__property XSpectrum       *Spectrum  = { read = PSpectrum};
	__property XWave           *Wave      = { read = PWave};
};
#endif
