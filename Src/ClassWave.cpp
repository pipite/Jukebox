//---------------------------------------------------------------------------

#pragma hdrstop


#include "ClassWave.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   ClassWave
// ---------------------------------------------------------------------------
__fastcall XWave::XWave(XCore *core) {
	Core = core;
	NeedRefresh = true;
}

__fastcall XWave::~XWave(void) {
};

void __fastcall XWave::SetImage(TImage *image) {
	PImage = image;
}

void __fastcall XWave::Refresh(XNode *node) {
	if (!NeedRefresh) return;
	if (node == NULL) return;
	if (node->WaveLoaded) {
		node->DrawWave(PImage);
		NeedRefresh = false;
	}
}

void __fastcall XWave::Scan(XNode *node) {
	if ( !node->SongNode->WaveLoaded && !node->SongNode->WaveLoading )
		new ThreadWaveScan(Core, node,false);
}



// ---------------------------------------------------------------------------
//   ThreadWaveScan       	0 < val < 32767     step 2 seconds
// ---------------------------------------------------------------------------
__fastcall ThreadWaveScan::ThreadWaveScan(XCore *core, XNode *node, bool CreateSuspended) : TThread(CreateSuspended)
{
	PNode              = node->SongNode;
	PNode->WaveLoading = true;
	FreeOnTerminate    = true;
	Core               = core;
	Core->Working      = true;
	PNode->MsFadeOut   = 15000;
	wave               = PNode->Wave;
	adblevel           = (int) (Core->Settings->AdbPercent * 32767 / 100);
	fadein             = -1;
	fadeout            = -1;
}

//---------------------------------------------------------------------------
void __fastcall ThreadWaveScan::Execute(void)
{
	if ( PNode->WaveLoaded ) return;
	if ( Core->Settings->Engine == "DirectShow" ) WaveDirectShow();
	if ( Core->Settings->Engine == "Bass" )       WaveBass();
	Synchronize( Done );
}
//---------------------------------------------------------------------------
void __fastcall ThreadWaveScan::Done(void) {
	PNode->WaveLoaded  = true;
	PNode->WaveLoading = false;
	PNode->MsFadeIn = (unsigned __int64) (fadein * length);
	PNode->MsFadeOut = (unsigned __int64) (fadeout * length);
	if (PNode->MsFadeOut > PNode->MsLength - Core->Settings->FadeOutDuration) PNode->MsFadeOut = PNode->MsLength - Core->Settings->FadeOutDuration;
	Core->Working = false;
}

void __fastcall ThreadWaveScan::WaveBass(void) {           // s >> ms
	DWORD mm;
	decoder = BASS_StreamCreateFile(false,PNode->Path.w_str(),0,0,BASS_STREAM_DECODE | BASS_STREAM_PRESCAN);
	length  = BASS_ChannelBytes2Seconds( decoder, BASS_ChannelGetLength(decoder,BASS_POS_BYTE) );
	PNode->MsFadeIn  = 0;
	if ( length > 2 ) PNode->MsFadeOut = (unsigned) (length *1000) - Core->Settings->FadeOutDuration; else PNode->MsFadeOut = 0;
	PNode->MsLength = length * 1000;
	if (decoder != 0) {
		cpos     = 0;
		peak0    = 0;
		peak1    = 0;
		counter  = 0;
		w        = 1000;
		position = 0;

		bpp = (DWORD) BASS_ChannelSeconds2Bytes(decoder,length) / w;
		pondpeak = 0.0;
		pondcount = 0;

		while ( position < (DWORD) w ) {
			level = BASS_ChannelGetLevel(decoder);

			peak0 = LOWORD(level);
			peak1 = HIWORD(level);
			if ( BASS_ChannelIsActive(decoder) != BASS_ACTIVE_PLAYING ) {
				break;
			} else {
				if (bpp != 0) {
					position = (DWORD) BASS_ChannelGetPosition(decoder,BASS_POS_BYTE) / bpp;
				} else {
					length  = 0;
					fadein  = 0;
					fadeout = 0;
					return;
				}

			}
			mm = (peak0<peak1)?peak1:peak0; mm = (mm>32767)?32767:mm;
			peakmax = (peakmax<mm)?mm:peakmax; peakmax = (peakmax>32767)?32767:peakmax;
			pondpeak = pondpeak + mm;
			pondcount++;
			if ( position > cpos ) {
				counter++;
				if ( counter < w ) {
					*(wave + counter) = (unsigned) (pondpeak / pondcount);
					if ( *(wave + counter) > adblevel ) {
						if ( fadein == -1) fadein = counter;
						fadeout = counter;
					}
					pondpeak = 0;
					pondcount = 0;
				}
				cpos = position;
			}
		}
	}
	BASS_StreamFree(decoder);
	PNode->Peak = peakmax;
}

void __fastcall ThreadWaveScan::WaveDirectShow(void) {
	DWORD           mm;
	REFERENCE_TIME  i64length;
	IGraphBuilder  *Pigb;
	ISampleGrabber *Pisg;
//	IMediaControl  *Pimc;
//	IMediaEventEx  *Pimex;
//	IBasicAudio    *Piba;
	IMediaSeeking  *Pims;

	::CoInitialize(NULL);
	if (SUCCEEDED(CoCreateInstance( CLSID_FilterGraph,
	NULL,
	CLSCTX_INPROC_SERVER,
	IID_IGraphBuilder,
	(void **)&Pigb))) {
		Pigb->QueryInterface(IID_IMediaSeeking,  (void**) &Pims);
		Pigb->QueryInterface(IID_ISampleGrabber, (void**) &Pisg);
//		Pigb->QueryInterface(IID_IMediaControl,  (void **)&Pimc);
//		Pigb->QueryInterface(IID_IMediaEventEx,  (void **)&Pimex);
//		Pigb->QueryInterface(IID_IBasicAudio,    (void**) &Piba);

		HRESULT hr = Pigb->RenderFile(PNode->Path.c_str(), NULL);
		if (SUCCEEDED(hr)) {
			if( Pims ) {
				Pims->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
				Pims->GetDuration(&i64length);
				length = (unsigned __int64) i64length / 10000;
			}
		}
	}
	::CoUninitialize();
	PNode->MsFadeIn  = 0;
	if ( length > 2 ) PNode->MsFadeOut = (unsigned __int64) (length) - 2000; else PNode->MsFadeOut = 0;
	PNode->MsLength = length;
	length /=1000;

	fadein = 1;
    fadeout = 999;
	if (Pigb)  { Pigb->Release();  Pigb  = NULL; }
//	if (Pimc)  { Pimc->Release();  Pimc  = NULL; }
//	if (Pimex) { Pimex->Release(); Pimex = NULL; }
//	if (Piba)  { Piba->Release();  Piba  = NULL; }
	if (Pims)  { Pims->Release();  Pims  = NULL;  }
	if (Pisg)  { Pisg->Release();  Pisg  = NULL;  }


//	decoder = BASS_StreamCreateFile(false,PNode->Path.w_str(),0,0,BASS_STREAM_DECODE | BASS_STREAM_PRESCAN);
//	length  = BASS_ChannelBytes2Seconds( decoder, BASS_ChannelGetLength(decoder,BASS_POS_BYTE) );
//	UpdateMsFade();
//	if (decoder != 0) {
//		cpos     = 0;
//		peak0    = 0;
//		peak1    = 0;
//		counter  = 0;
//		w        = 1000;
//		position = 0;
//
//		bpp = (DWORD) BASS_ChannelSeconds2Bytes(decoder,length) / w;
//		pondpeak = 0.0;
//		pondcount = 0;
//
//		while ( position < (DWORD) w ) {
//			level = BASS_ChannelGetLevel(decoder);
//
//			peak0 = LOWORD(level);
//			peak1 = HIWORD(level);
//			if ( BASS_ChannelIsActive(decoder) != BASS_ACTIVE_PLAYING ) {
//				break;
//			} else {
//				if (bpp != 0) {
//					position = (DWORD) BASS_ChannelGetPosition(decoder,BASS_POS_BYTE) / bpp;
//				} else {
//					length  = 0;
//					fadein  = 0;
//					fadeout = 0;
//					return;
//				}
//
//			}
//			mm = (peak0<peak1)?peak1:peak0; mm = (mm>32767)?32767:mm;
//			peakmax = (peakmax<mm)?mm:peakmax; peakmax = (peakmax>32767)?32767:peakmax;
//			pondpeak = pondpeak + mm;
//			pondcount++;
//			if ( position > cpos ) {
//				counter++;
//				if ( counter < w ) {
//					*(wave + counter) = (unsigned) (pondpeak / pondcount);
//					if ( *(wave + counter) > adblevel ) {
//						if ( fadein == -1) fadein = counter;
//						fadeout = counter;
//					}
//					pondpeak = 0;
//					pondcount = 0;
//				}
//				cpos = position;
//			}
//		}
//	}
//	BASS_StreamFree(decoder);
//	PNode->Peak = peakmax;
}
