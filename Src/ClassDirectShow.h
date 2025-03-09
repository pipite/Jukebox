//---------------------------------------------------------------------------

#ifndef ClassDirectShowH
#define ClassDirectShowH

#include <windows.h>
#include <mmsystem.h>
#include <strmif.h>
#include <control.h>
#include <uuids.h>
#include <QEdit.h>

class XDx;

#include "ClassCore.h"

#define DXTIMEFACTOR 10000
#define DXVOLUMEFACTOR 10000.0

//---------------------------------------------------------------------------
class XDirectShow  : public XEngine {
	HWND            PHwnd;
	float           PFFT[512];
	REFERENCE_TIME  PMsLength;
	long           *fft;
	bool            PHaveChannel;
	bool            PIsPlaying;

	IGraphBuilder  *Pigb;
	IMediaControl  *Pimc;
//	IMediaEventEx  *Pimex;
	IBasicAudio    *Piba;
	IMediaSeeking  *Pims;
	ISampleGrabber *Pisg;

	void              __fastcall Cleanup(void);

	bool              __fastcall GetHaveChannel(void);
	float*            __fastcall GetFFT(void);
	float             __fastcall GetVolume(void);
	unsigned __int64  __fastcall GetMsLength(void);
	unsigned __int64  __fastcall GetPosition(void);
	unsigned __int64  __fastcall GetMsBeforeEnd(void);
	bool              __fastcall GetIsPlaying(void);
	bool              __fastcall SetNode(XNode *node);
	void              __fastcall SetVolume(float volume);
	void              __fastcall SetPosition(unsigned __int64 mspos);

public:
		 __fastcall XDirectShow();
		 __fastcall ~XDirectShow(void);

	bool __fastcall Initialize(HWND hwnd);
	bool __fastcall Play(void);
	void __fastcall Stop(void);
	bool __fastcall IsAudioFile(XNode *node);

	__property XNode*           Node        =	{               		write = SetNode};
	__property unsigned __int64 MsLength    =	{read = GetMsLength};
	__property unsigned __int64 Position    =	{read = GetPosition, 	write = SetPosition};
	__property bool             IsPlaying   =	{read = GetIsPlaying};
	__property bool             HaveChannel =	{read = GetHaveChannel};
	__property float           *FFT		    =	{read = GetFFT};
	__property float            Volume      =	{read = GetVolume, 		write = SetVolume};
	__property unsigned __int64 MsBeforeEnd =	{read = GetMsBeforeEnd};
};
#endif
