//---------------------------------------------------------------------------
#ifndef ClassEngineH
#define ClassEngineH

class XNode; // Forward declaration

//---------------------------------------------------------------------------
class XEngine {
private:
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
	XEngine() {};
	virtual ~XEngine() {};

	virtual bool   __fastcall Initialize(HWND hwnd)                          = 0;
	virtual bool   __fastcall Play(void)                                     = 0;
	virtual void   __fastcall Stop(void)                                     = 0;
	virtual bool   __fastcall IsAudioFile(XNode *node)                       = 0;

	__property XNode*           Node     	= {               			write = SetNode     };
	__property unsigned __int64 MsLength	= {read = GetMsLength                           };
	__property unsigned __int64 Position    = {read = GetPosition, 		write = SetPosition };
	__property bool             IsPlaying   = {read = GetIsPlaying                          };
	__property bool             HaveChannel = {read = GetHaveChannel                        };
	__property float           *FFT		    = {read = GetFFT                                };
	__property float            Volume   	= {read = GetVolume, 		write = SetVolume   };
	__property unsigned __int64 MsBeforeEnd = {read = GetMsBeforeEnd                        };
};

#endif