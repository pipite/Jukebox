//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassDirectShow.h"
//---------------------------------------------------------------------------
#pragma comment(lib, "strmiids.lib")
#pragma package(smart_init)

//	IGraphBuilder *Pigb;
//	IMediaControl *Pimc;
//	IMediaEventEx *Pimex;
//	IBasicAudio   *Piba;
//	IMediaSeeking *Pims;

// ---------------------------------------------------------------------------
//   ClassDx
//       Durées DX: __int64 = 10,000,000 for a second.
//       Volume DX: long    = -10000 <= Volume <= 0
//   Classmix
//       Durées Mix: unsigned __int64  = 1000 for a second.
//       Volume Mix: float  = 0.0 <= Volume <= 1.0
// ---------------------------------------------------------------------------
__fastcall XDirectShow::XDirectShow() : XEngine() {
	Pigb         = NULL;
	Pimc         = NULL;
//	Pimex        = NULL;
	Piba         = NULL;
	Pims         = NULL;
	Pisg         = NULL;
	PMsLength    = 0;
	PHaveChannel = false;
	PIsPlaying   = false;
	fft          = (long*) malloc(1024);
}

__fastcall XDirectShow::~XDirectShow(void) {
	Cleanup();
	free (fft);
}

bool __fastcall XDirectShow::Initialize(HWND hwnd) {
	return true;
}

void __fastcall XDirectShow::Cleanup(void) {
	if (Pimc)  { Pimc->Stop(); }
	if (Pigb)  { Pigb->Release();  Pigb  = NULL; }
	if (Pimc)  { Pimc->Release();  Pimc  = NULL; }
//	if (Pimex) { Pimex->Release(); Pimex = NULL; }
	if (Piba)  { Piba->Release();  Piba  = NULL; }
	if (Pims)  { Pims->Release();  Pims  = NULL; }
	if (Pisg)  { Pisg->Release();  Pisg  = NULL; }
	PMsLength    = 0;
	PHaveChannel = false;
	PIsPlaying   = false;
}

bool __fastcall XDirectShow::SetNode(XNode *node) {
	Cleanup();
	::CoInitialize(NULL);
	if (SUCCEEDED(CoCreateInstance( CLSID_FilterGraph,
	NULL,
	CLSCTX_INPROC_SERVER,
	IID_IGraphBuilder,
	(void **)&this->Pigb))) {
		Pigb->QueryInterface(IID_IMediaControl,  (void**) &Pimc);
//		Pigb->QueryInterface(IID_IMediaEventEx,  (void**) &Pimex);
		Pigb->QueryInterface(IID_IBasicAudio,    (void**) &Piba);
		Pigb->QueryInterface(IID_IMediaSeeking,  (void**) &Pims);
		Pigb->QueryInterface(IID_ISampleGrabber, (void**) &Pisg);

		if (SUCCEEDED( Pigb->RenderFile(node->Path.c_str(), NULL) )) {
			PHaveChannel = true;
			if( Pims ) {
				Pims->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
				Pims->GetDuration(&PMsLength);
			}
		}
	}
	::CoUninitialize();
	return PHaveChannel;
}

bool __fastcall XDirectShow::GetHaveChannel(void) {
	return PHaveChannel;
}

bool __fastcall XDirectShow::GetIsPlaying(void) {
	return PIsPlaying;
}

bool __fastcall XDirectShow::Play(void) {
	PIsPlaying = false;
	if ( PHaveChannel && Pimc ) {
		PIsPlaying = SUCCEEDED( Pimc->Run() );
    }
	return PIsPlaying;
}

void __fastcall XDirectShow::Stop(void) {
	if ( PHaveChannel && Pimc )
		Pimc->Stop();
	PIsPlaying = false;
}

float  __fastcall XDirectShow::GetVolume(void) {
	if ( PHaveChannel && Piba ) {
        long vol = -1;
		if( SUCCEEDED( Piba->get_Volume(&vol) ) )
			return (float) (DXVOLUMEFACTOR + (float)vol)/DXVOLUMEFACTOR;
	}
	return (float) 0;
}

void   __fastcall XDirectShow::SetVolume(float volume) {
	long v;
	v = (long) (volume*DXVOLUMEFACTOR - DXVOLUMEFACTOR);
	if ( PHaveChannel && Piba )
		Piba->put_Volume((long) v);
}

float* __fastcall XDirectShow::GetFFT(void) {
//	long bufsize = 512;
//	Pisg->GetCurrentBuffer(&bufsize,fft);
//	for (int i = 0; i < 512; i++) PFFT[i] = (float) fft[i];      // fake fft
	return PFFT;
}

unsigned __int64 __fastcall XDirectShow::GetMsLength(void) {
	return (unsigned __int64) (PMsLength / DXTIMEFACTOR );
}

unsigned __int64 __fastcall XDirectShow::GetMsBeforeEnd(void) {
	REFERENCE_TIME pos = -1;
	Pims->GetCurrentPosition(&pos);
	return (unsigned __int64) ( (PMsLength - pos) / DXTIMEFACTOR);
}

unsigned __int64 __fastcall XDirectShow::GetPosition(void) {
	if ( PHaveChannel && Pims ) {
		REFERENCE_TIME pos = -1;
		HRESULT hr = Pims->GetCurrentPosition(&pos);
//		unsigned __int64 p = (unsigned __int64) (pos / DXTIMEFACTOR);
		if(SUCCEEDED(hr)) return (unsigned __int64) (pos / DXTIMEFACTOR);
	}
	return 0;
}

void __fastcall XDirectShow::SetPosition(unsigned __int64 mspos) {
	if ( PHaveChannel && Pims ) {
		REFERENCE_TIME pCurrent = (REFERENCE_TIME) (mspos * DXTIMEFACTOR);
		REFERENCE_TIME pStop = PMsLength; //(__int64) PMsLength;
		DWORD flags = AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame;
		Pims->SetPositions(&pCurrent, flags, &pStop , AM_SEEKING_AbsolutePositioning);
	}
}


