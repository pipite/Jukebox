//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
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
	adblevel           = (int) (Core->Settings->AdbPercent * MAX_PEAK / 100);
	fadein             = -1;
	fadeout            = -1;
	loaded             = false;
	path               = PNode->Path.w_str();
	decoder            = NULL;
}

//---------------------------------------------------------------------------
void __fastcall ThreadWaveScan::Execute(void)
{
	if ( PNode->WaveLoaded ) {
		PNode->WaveLoading = false;
		Core->Working = false;
	} else {
		WaveBass();
		Synchronize( Done );
		if ( decoder != NULL ) {
			BASS_StreamFree(decoder);
		}
	}
}

void __fastcall ThreadWaveScan::UpdatePNode(void)
{
	PNode->MsFadeIn  = 0;
	PNode->MsLength  = length * 1000;
	PNode->MsFadeOut = (length > 2) ? static_cast<unsigned>(PNode->MsLength) - Core->Settings->FadeOutDuration : 0;
}

//---------------------------------------------------------------------------
void __fastcall ThreadWaveScan::Done(void) {
	PNode->WaveLoaded  = loaded;
	PNode->WaveLoading = false;
	if (loaded) {
		PNode->MsFadeIn    = (unsigned __int64) (fadein * length);
		PNode->MsFadeOut   = (unsigned __int64) (fadeout * length);
		PNode->Peak        = peakmax;
		if ( (PNode->MsFadeOut > PNode->MsLength - Core->Settings->FadeOutDuration) && loaded ) PNode->MsFadeOut = PNode->MsLength - Core->Settings->FadeOutDuration;
	}
	Core->Working = false;
}

void __fastcall ThreadWaveScan::WaveBass(void) {
	// Création du flux et vérification de sa validité
	decoder = BASS_StreamCreateFile(false,path,0,0,BASS_STREAM_DECODE | BASS_STREAM_PRESCAN);
	if ( !decoder ) return;

	// Calcul de la durée et définition des propriétés temporelles
	length  = BASS_ChannelBytes2Seconds( decoder, BASS_ChannelGetLength(decoder,BASS_POS_BYTE) );

	Synchronize ( UpdatePNode );

	// Calcul des octets par point et validation
	const DWORD bpp = static_cast<DWORD>(BASS_ChannelSeconds2Bytes(decoder, length) / WAVE_WIDTH);
	if ( !bpp ) {
		BASS_StreamFree(decoder);
		return;
	}

	// Initialisation des variables de suivi
	DWORD position = 0;
	DWORD cpos     = 0;
	DWORD counter  = 0;
	WORD peakmax  = 0;
	float pondpeak = 0.0f;
	int pondcount  = 0;
	// Traitement des données audio
	while ( position < WAVE_WIDTH ) {
		// Obtention du niveau actuel et vérification si la lecture est toujours en cours
		const DWORD level = BASS_ChannelGetLevel(decoder);
		if ( BASS_ChannelIsActive(decoder) != BASS_ACTIVE_PLAYING )
			break;

		// Extraction et traitement des valeurs de crête
		const WORD peak0 = LOWORD(level);
		const WORD peak1 = HIWORD(level);
		const WORD mm    = (peak0 < peak1) ? peak1 : peak0;
		const WORD peak        = (mm > MAX_PEAK) ? MAX_PEAK : mm;

		// Mise à jour du suivi des crêtes
		peakmax = (peakmax < mm) ? peak : peakmax;
		pondpeak += peak;
		pondcount++;

		// Calcul de la position actuelle
		position = static_cast<DWORD>(BASS_ChannelGetPosition(decoder,BASS_POS_BYTE) / bpp);

		// Traitement lorsque la position change
		if ( position > cpos ) {
			counter++;
			if ( counter < WAVE_WIDTH ) {
				// Stockage de la crête moyenne pour ce segment
				*(wave + counter) = static_cast<unsigned>(pondpeak / pondcount);

				// Mise à jour des points de fondu si nécessaire
				if ( *(wave + counter) > adblevel ) {
					if ( fadein == -1) fadein = counter;
					fadeout = counter;
				}

				// Réinitialisation des accumulateurs
				pondpeak = 0.0f;
				pondcount = 0;
			}
			cpos = position;
		}
	}

	// Nettoyage et stockage de la valeur de crête finale
	if ( BASS_StreamFree(decoder) ) decoder = NULL;
	loaded = true;
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
