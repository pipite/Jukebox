//---------------------------------------------------------------------------
#ifndef ClassPlayersH
#define ClassPlayersH

class XPlayers;

#include "VirtualEngine.h"
#include "ClassCore.h"
#include "ClassNode.h"

//---------------------------------------------------------------------------
class XPlayers {
	XEngine  *PEngine;
	XCore    *Core;
	HWND      Hwnd;
	XNode    *PNode;

	bool      PIsFadeInAuto;
	bool      PIsFadeOutAuto;
	QWORD     PMsLength;
	QWORD     PFadeInStart;
	QWORD     PFadeOutStart;
	QWORD     PFadeInMini;
	QWORD     PFadeOutMini;
	bool      PIsFadeIn;
	bool      PIsFadeOut;
	bool      PPlayFlag;

	float*     __fastcall GetFFT(void);
	float      __fastcall GetVolume(void);
	QWORD      __fastcall GetMsLength(void);
	QWORD      __fastcall GetPosition(void);
	QWORD      __fastcall GetMsBeforeEnd(void);
	bool       __fastcall GetIsPlaying(void);
	bool       __fastcall GetIsValid(void);
	void       __fastcall SetNode(XNode *node);
	XNode*     __fastcall GetNode(void);
	void       __fastcall SetVolume(float volume);
	void       __fastcall SetPosition(QWORD mspos);

public:
	bool Working;

			   XPlayers(XCore *core, HWND hwnd);
			   ~XPlayers(void);

	void       __fastcall Play(void);
	void       __fastcall Stop(void);
    bool       __fastcall IsAudioFile(XNode *node);
	bool       __fastcall FadeIn(void);
	bool       __fastcall FadeOut(void);
	void       __fastcall FadeOutNow(QWORD length);
	void       __fastcall FadeInNow(QWORD length);
	void       __fastcall FadeOutAuto(QWORD length);
	void       __fastcall FadeInAuto(QWORD length);

	__property float     *FFT		  =	{read = GetFFT};
	__property QWORD      MsLength	  =	{read = GetMsLength};
	__property QWORD      MsBeforeEnd =	{read = GetMsBeforeEnd};
	__property QWORD      Position    =	{read = GetPosition, 	write = SetPosition};
	__property bool       IsPlaying   =	{read = GetIsPlaying};
	__property bool       IsValid  	  =	{read = GetIsValid};
	__property float      Volume   	  =	{read = GetVolume, 		write = SetVolume};
	__property XNode*     Node     	  =	{read = GetNode,		write = SetNode};

};
#endif
