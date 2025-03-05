//---------------------------------------------------------------------------

#ifndef ClassPlayersH
#define ClassPlayersH

class XPlayers;

#include "ClassCore.h"
#include "ClassNode.h"

class XEngine {
private:
//	HWND      PHwnd;
//	HSTREAM   PChannel;
//	float     PFFT[512];

	virtual bool              __fastcall GetHaveChannel(void)                = 0;
	virtual float*            __fastcall GetFFT(void)                        = 0;
	virtual float             __fastcall GetVolume(void)                     = 0;
	virtual unsigned __int64  __fastcall GetMsLength(void)                   = 0;
	virtual unsigned __int64  __fastcall GetPosition(void)                   = 0;
	virtual unsigned __int64  __fastcall GetMsBeforeEnd(void)                = 0;
	virtual bool              __fastcall GetIsPlaying(void)                  = 0;
	virtual bool              __fastcall SetNode(XNode *node)                = 0;
	virtual void              __fastcall SetVolume(float volume)             = 0;
	virtual void              __fastcall SetPosition(unsigned __int64 mspos) = 0;

public:
	virtual		   __fastcall ~XEngine() {};

	virtual bool   __fastcall Initialize(HWND hwnd)                          = 0;
	virtual bool   __fastcall Play(void)                                     = 0;
	virtual void   __fastcall Stop(void)                                     = 0;

	__property XNode*           Node     	= {               			write = SetNode};
	__property unsigned __int64 MsLength	= {read = GetMsLength};
	__property unsigned __int64 Position    = {read = GetPosition, 		write = SetPosition};
	__property bool             IsPlaying   = {read = GetIsPlaying};
	__property bool             HaveChannel = {read = GetHaveChannel};
	__property float           *FFT		    = {read = GetFFT};
	__property float            Volume   	= {read = GetVolume, 		write = SetVolume};
	__property unsigned __int64 MsBeforeEnd = {read = GetMsBeforeEnd};
};

#include "ClassBass.h"
#include "ClassDirectShow.h"

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

			   __fastcall XPlayers(XCore *core, HWND hwnd);
			   __fastcall ~XPlayers(void);

	void       __fastcall Play(void);
	void       __fastcall Stop(void);

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
