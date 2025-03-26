//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include <algorithm> // Pour les fonctions min et max
#include <math.h>     // Pour la fonction fabsf
#include "ClassFFmpeg.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")

// ---------------------------------------------------------------------------
//   ClassFFmpeg
//       Durées FFmpeg: int64_t = microsecondes
//       Volume FFmpeg: float = 0.0 <= Volume <= 1.0
//   Classmix
//       Durées Mix: unsigned __int64 = 1000 for a second.
//       Volume Mix: float = 0.0 <= Volume <= 1.0
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//   public
// ---------------------------------------------------------------------------

__fastcall XFFmpeg::XFFmpeg() : XEngine() {
    // Initialisation DirectSound
    PHwnd = NULL;
    PDirectSound = NULL;
    PPrimaryBuffer = NULL;
    PSecondaryBuffer = NULL;
    PNotify = NULL;
    
    PNotifyEvents[0] = NULL;
    PNotifyEvents[1] = NULL;
    PThread = NULL;
    PThreadID = 0;
    PThreadActive = false;
    
    ZeroMemory(&PWaveFormat, sizeof(PWaveFormat));
    PBufferSize = 0;
    PReadPosition = 0;
    PWritePosition = 0;
    
    // Initialisation FFmpeg
    PFormatContext = NULL;
    PCodecContext = NULL;
    PSwrContext = NULL;
    PAudioStreamIndex = -1;
    PPacket = NULL;
    PFrame = NULL;
    PAudioBuffer = NULL;
    PAudioBufferSize = 0;
    PAudioBufferPos = 0;
    
    PDuration = 0;
    PVolume = 1.0f;
    PIsPlaying = false;
    PIsInitialized = false;
    
    // Initialiser les tableaux à zéro
    ZeroMemory(PFFT, sizeof(PFFT));
    ZeroMemory(PRawData, sizeof(PRawData));
    
    // Initialiser FFmpeg
    #if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
    #endif
}

__fastcall XFFmpeg::~XFFmpeg(void) {
    // Arrêter la lecture
    Stop();
    
    // Nettoyer DirectSound
    CleanupDirectSound();
    
    // Nettoyer FFmpeg
    CleanupFFmpeg();
}

bool __fastcall XFFmpeg::Initialize(HWND hwnd) {
    PHwnd = hwnd;
    
    // Initialiser DirectSound
    HRESULT hr = InitializeDirectSound();
    if (FAILED(hr)) {
        MessageDlg("Failed to initialize DirectSound.", mtError, TMsgDlgButtons() << mbOK, 0);
        return false;
    }
    
    PIsInitialized = true;
    return true;
}

bool __fastcall XFFmpeg::Play(void) {
    if (!PSecondaryBuffer || !PFormatContext) {
        return false;
    }
    
    // Vérifier si la lecture est déjà en cours
    if (PIsPlaying) {
        return true;
    }
    
    // Remplir le buffer avec les premières données avant de démarrer la lecture
    ProcessAudio();
    
    // Démarrer la lecture
    HRESULT hr = PSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
    if (FAILED(hr)) {
        return false;
    }
    
    // Démarrer le thread de lecture
    PThreadActive = true;
    PThread = CreateThread(NULL, 0, PlaybackThread, this, 0, &PThreadID);
    if (!PThread) {
        PSecondaryBuffer->Stop();
        PThreadActive = false;
        return false;
    }
    
    // Définir la priorité du thread
    SetThreadPriority(PThread, THREAD_PRIORITY_TIME_CRITICAL);
    
    PIsPlaying = true;
    return true;
}

void __fastcall XFFmpeg::Stop(void) {
    // Arrêter le thread de lecture
    if (PThreadActive) {
        PThreadActive = false;
        
        // Attendre la fin du thread
        if (PThread) {
            WaitForSingleObject(PThread, 1000);
            CloseHandle(PThread);
            PThread = NULL;
        }
    }
    
    // Arrêter la lecture
    if (PSecondaryBuffer) {
        PSecondaryBuffer->Stop();
    }
    
    PIsPlaying = false;
}

bool __fastcall XFFmpeg::IsAudioFile(XNode *node) {
    if (!node) {
        return false;
    }
    
    // Ouvrir le fichier avec FFmpeg
    AVFormatContext* formatContext = NULL;
    int result = avformat_open_input(&formatContext, AnsiString(node->Path).c_str(), NULL, NULL);
    if (result < 0) {
        return false;
    }
    
    // Trouver les informations sur les flux
    result = avformat_find_stream_info(formatContext, NULL);
    if (result < 0) {
        avformat_close_input(&formatContext);
        return false;
    }
    
    // Vérifier s'il y a un flux audio
    bool hasAudio = false;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            hasAudio = true;
            break;
        }
    }
    
    // Fermer le fichier
    avformat_close_input(&formatContext);
    
    return hasAudio;
}

// ---------------------------------------------------------------------------
//   property
// ---------------------------------------------------------------------------

bool __fastcall XFFmpeg::SetNode(XNode *node) {
    if (!node) {
        return false;
    }
    
    // Arrêter la lecture en cours
    Stop();
    
    // Nettoyer FFmpeg
    CleanupFFmpeg();
    
    try {
        // Ouvrir le fichier avec FFmpeg
        int result = avformat_open_input(&PFormatContext, AnsiString(node->Path).c_str(), NULL, NULL);
        if (result < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
            throw Exception("Failed to open file: " + String(errbuf));
        }
        
        // Trouver les informations sur les flux
        result = avformat_find_stream_info(PFormatContext, NULL);
        if (result < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
            throw Exception("Failed to find stream info: " + String(errbuf));
        }
        
        // Trouver le flux audio
        PAudioStreamIndex = -1;
        for (unsigned int i = 0; i < PFormatContext->nb_streams; i++) {
            if (PFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                PAudioStreamIndex = i;
                break;
            }
        }
        
        if (PAudioStreamIndex == -1) {
            throw Exception("No audio stream found");
        }
        
        // Obtenir le codec
        AVCodecParameters* codecParams = PFormatContext->streams[PAudioStreamIndex]->codecpar;
        AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
        if (!codec) {
            throw Exception("Codec not found");
        }
        
        // Allouer le contexte de codec
        PCodecContext = avcodec_alloc_context3(codec);
        if (!PCodecContext) {
            throw Exception("Failed to allocate codec context");
        }
        
        // Copier les paramètres du codec
        result = avcodec_parameters_to_context(PCodecContext, codecParams);
        if (result < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
            throw Exception("Failed to copy codec parameters: " + String(errbuf));
        }
        
        // Ouvrir le codec
        result = avcodec_open2(PCodecContext, codec, NULL);
        if (result < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
            throw Exception("Failed to open codec: " + String(errbuf));
        }
        
        // Configurer le format audio pour DirectSound
        ZeroMemory(&PWaveFormat, sizeof(PWaveFormat));
        PWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
        PWaveFormat.nChannels = PCodecContext->channels > 2 ? 2 : PCodecContext->channels; // Limiter à 2 canaux
        PWaveFormat.nSamplesPerSec = PCodecContext->sample_rate;
        PWaveFormat.wBitsPerSample = 16; // Toujours utiliser 16 bits
        PWaveFormat.nBlockAlign = (PWaveFormat.nChannels * PWaveFormat.wBitsPerSample) / 8;
        PWaveFormat.nAvgBytesPerSec = PWaveFormat.nSamplesPerSec * PWaveFormat.nBlockAlign;
        
        // Configurer le resampler si nécessaire
        PSwrContext = swr_alloc_set_opts(NULL,
            av_get_default_channel_layout(PWaveFormat.nChannels),  // Sortie: canal layout
            AV_SAMPLE_FMT_S16,                                     // Sortie: format d'échantillon
            PWaveFormat.nSamplesPerSec,                            // Sortie: taux d'échantillonnage
            av_get_default_channel_layout(PCodecContext->channels),// Entrée: canal layout
            PCodecContext->sample_fmt,                             // Entrée: format d'échantillon
            PCodecContext->sample_rate,                            // Entrée: taux d'échantillonnage
            0, NULL);
        
        if (!PSwrContext) {
            throw Exception("Failed to allocate resampler context");
        }
        
        result = swr_init(PSwrContext);
        if (result < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
            throw Exception("Failed to initialize resampler: " + String(errbuf));
        }
        
        // Allouer le paquet et la trame
        PPacket = av_packet_alloc();
        if (!PPacket) {
            throw Exception("Failed to allocate packet");
        }
        
        PFrame = av_frame_alloc();
        if (!PFrame) {
            throw Exception("Failed to allocate frame");
        }
        
        // Allouer le buffer audio
        PAudioBufferSize = 192000; // Taille arbitraire, ajuster si nécessaire
        PAudioBuffer = (uint8_t*)av_malloc(PAudioBufferSize);
        if (!PAudioBuffer) {
            throw Exception("Failed to allocate audio buffer");
        }
        
        PAudioBufferPos = 0;
        
        // Calculer la durée du fichier
        if (PFormatContext->duration != AV_NOPTS_VALUE) {
            PDuration = PFormatContext->duration / 1000; // Convertir de microsecondes à millisecondes
        } else {
            PDuration = 0;
        }
        
        // Créer le buffer DirectSound
        HRESULT hr = CreateSoundBuffer();
        if (FAILED(hr)) {
            throw Exception("Failed to create DirectSound buffer");
        }
        
        // Configurer les notifications
        hr = SetupNotifications();
        if (FAILED(hr)) {
            throw Exception("Failed to setup notifications");
        }
        
        // Remplir le buffer avec les premières données
        ProcessAudio();
        
        return true;
    }
    catch (Exception &e) {
        // Afficher l'erreur
        OutputDebugString(e.Message.c_str());
        
        // Nettoyer FFmpeg
        CleanupFFmpeg();
        
        return false;
    }
}

unsigned __int64 __fastcall XFFmpeg::GetMsLength(void) {
    return PDuration;
}

unsigned __int64 __fastcall XFFmpeg::GetPosition(void) {
    if (!PSecondaryBuffer) {
        return 0;
    }
    
    DWORD playPosition = 0;
    DWORD writePosition = 0;
    
    HRESULT hr = PSecondaryBuffer->GetCurrentPosition(&playPosition, &writePosition);
    if (FAILED(hr)) {
        return 0;
    }
    
    // Calculer la position en millisecondes
    DWORD position = (playPosition * 1000) / PWaveFormat.nAvgBytesPerSec;
    
    return position;
}

void __fastcall XFFmpeg::SetPosition(unsigned __int64 mspos) {
    if (!PSecondaryBuffer || !PFormatContext) {
        return;
    }
    
    // Limiter la position à la durée du fichier
    if (mspos > PDuration) {
        mspos = PDuration;
    }
    
    // Calculer la position en octets
    DWORD bytePosition = (mspos * PWaveFormat.nAvgBytesPerSec) / 1000;
    
    // Définir la position dans le buffer
    HRESULT hr = PSecondaryBuffer->SetCurrentPosition(bytePosition);
    if (FAILED(hr)) {
        return;
    }
    
    // Définir la position dans le fichier
    int64_t timestamp = mspos * 1000; // Convertir en microsecondes
    int streamIndex = PAudioStreamIndex;
    
    // Convertir la position en timestamp
    int64_t targetTs = av_rescale_q(timestamp, 
                                    AV_TIME_BASE_Q, 
                                    PFormatContext->streams[streamIndex]->time_base);
    
    // Chercher la position
    int result = av_seek_frame(PFormatContext, streamIndex, targetTs, AVSEEK_FLAG_BACKWARD);
    if (result < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(result, errbuf, AV_ERROR_MAX_STRING_SIZE);
        OutputDebugString(("Failed to seek: " + String(errbuf)).c_str());
        return;
    }
    
    // Vider les buffers
    avcodec_flush_buffers(PCodecContext);
    PAudioBufferPos = 0;
}

bool __fastcall XFFmpeg::GetIsPlaying(void) {
    if (!PSecondaryBuffer) {
        return false;
    }
    
    DWORD status = 0;
    HRESULT hr = PSecondaryBuffer->GetStatus(&status);
    if (FAILED(hr)) {
        return false;
    }
    
    return (status & DSBSTATUS_PLAYING) != 0;
}

bool __fastcall XFFmpeg::GetHaveChannel(void) {
    return (PSecondaryBuffer != NULL && PFormatContext != NULL);
}

float* __fastcall XFFmpeg::GetFFT(void) {
    if (!PSecondaryBuffer || !PIsPlaying) {
        // Si pas de lecture en cours, retourner le tableau FFT vide
        return PFFT;
    }
    
    try {
        // Obtenir la position actuelle
        DWORD playPosition = 0;
        DWORD writePosition = 0;
        
        HRESULT hr = PSecondaryBuffer->GetCurrentPosition(&playPosition, &writePosition);
        if (FAILED(hr)) {
            OutputDebugString(L"Failed to get current position in GetFFT");
            return PFFT;
        }
        
        // Verrouiller le buffer pour accéder aux données
        LPVOID buffer1 = NULL;
        DWORD buffer1Size = 0;
        LPVOID buffer2 = NULL;
        DWORD buffer2Size = 0;
        
        // Limiter la taille de lecture à un multiple de la taille d'un bloc
        DWORD readSize = READ_BUFFER_SIZE;
        readSize = (readSize / PWaveFormat.nBlockAlign) * PWaveFormat.nBlockAlign;
        
        hr = PSecondaryBuffer->Lock(playPosition, readSize, &buffer1, &buffer1Size, &buffer2, &buffer2Size, 0);
        if (FAILED(hr)) {
            OutputDebugString(L"Failed to lock buffer in GetFFT");
            return PFFT;
        }
        
        // Calculer les valeurs FFT à partir des données audio
        CalculateFFT((BYTE*)buffer1, buffer1Size);
        
        // Déverrouiller le buffer
        PSecondaryBuffer->Unlock(buffer1, buffer1Size, buffer2, buffer2Size);
    }
    catch (...) {
        OutputDebugString(L"Exception in GetFFT");
    }
    
    return PFFT;
}

float __fastcall XFFmpeg::GetVolume(void) {
    return PVolume;
}

void __fastcall XFFmpeg::SetVolume(float volume) {
    // Limiter le volume entre 0.0 et 1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    PVolume = volume;
    
    if (!PSecondaryBuffer) {
        return;
    }
    
    // Convertir le volume en décibels (DirectSound utilise des décibels)
    // 0.0 = -10000 dB (silence), 1.0 = 0 dB (volume maximal)
    LONG dsVolume = (PVolume == 0.0f) ? DSBVOLUME_MIN : (LONG)(2000.0f * log10(PVolume) * 100.0f);
    if (dsVolume < DSBVOLUME_MIN) dsVolume = DSBVOLUME_MIN;
    if (dsVolume > DSBVOLUME_MAX) dsVolume = DSBVOLUME_MAX;
    
    // Définir le volume
    PSecondaryBuffer->SetVolume(dsVolume);
}

unsigned __int64 __fastcall XFFmpeg::GetMsBeforeEnd(void) {
    if (!PSecondaryBuffer) {
        return 0;
    }
    
    unsigned __int64 position = GetPosition();
    if (position >= PDuration) {
        return 0;
    }
    
    return PDuration - position;
}

// ---------------------------------------------------------------------------
//   private
// ---------------------------------------------------------------------------

HRESULT __fastcall XFFmpeg::InitializeDirectSound() {
    OutputDebugString(L"Initializing DirectSound");
    
    // Créer l'objet DirectSound
    HRESULT hr = DirectSoundCreate8(NULL, &PDirectSound, NULL);
    if (FAILED(hr)) {
        OutputDebugString(L"Failed to create DirectSound object");
        return hr;
    }
    
    // Définir le niveau de coopération
    hr = PDirectSound->SetCooperativeLevel(PHwnd, DSSCL_PRIORITY);
    if (FAILED(hr)) {
        OutputDebugString(L"Failed to set cooperative level");
        SAFE_RELEASE(PDirectSound);
        return hr;
    }
    
    // Créer le buffer primaire
    DSBUFFERDESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.dwSize = sizeof(DSBUFFERDESC);
    bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
    
    hr = PDirectSound->CreateSoundBuffer(&bufferDesc, &PPrimaryBuffer, NULL);
    if (FAILED(hr)) {
        OutputDebugString(L"Failed to create primary buffer");
        SAFE_RELEASE(PDirectSound);
        return hr;
    }
    
    // Configurer le format du buffer primaire
    WAVEFORMATEX waveFormat;
    ZeroMemory(&waveFormat, sizeof(waveFormat));
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 2;
    waveFormat.nSamplesPerSec = 44100;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    
    hr = PPrimaryBuffer->SetFormat(&waveFormat);
    if (FAILED(hr)) {
        OutputDebugString(L"Failed to set primary buffer format");
        SAFE_RELEASE(PPrimaryBuffer);
        SAFE_RELEASE(PDirectSound);
        return hr;
    }
    
    OutputDebugString(L"DirectSound initialized successfully");
    return S_OK;
}

HRESULT __fastcall XFFmpeg::CreateSoundBuffer() {
    OutputDebugString(L"Creating sound buffer");
    
    // Libérer le buffer secondaire précédent
    SAFE_RELEASE(PSecondaryBuffer);
    
    // Calculer la taille du buffer (en octets)
    PBufferSize = (DS_BUFFER_SIZE * PWaveFormat.nAvgBytesPerSec) / 1000;
    
    // S'assurer que la taille du buffer est un multiple de la taille d'un bloc
    PBufferSize = (PBufferSize / PWaveFormat.nBlockAlign) * PWaveFormat.nBlockAlign;
    
    // Créer le buffer secondaire
    DSBUFFERDESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.dwSize = sizeof(DSBUFFERDESC);
    bufferDesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
    bufferDesc.dwBufferBytes = PBufferSize;
    bufferDesc.lpwfxFormat = &PWaveFormat;
    
    // Afficher les informations du format audio
    UnicodeString debugInfo = L"Audio format: ";
    debugInfo += L"Channels=" + IntToStr(static_cast<int>(PWaveFormat.nChannels)) + L", ";
    debugInfo += L"SampleRate=" + IntToStr(static_cast<int>(PWaveFormat.nSamplesPerSec)) + L", ";
    debugInfo += L"BitsPerSample=" + IntToStr(static_cast<int>(PWaveFormat.wBitsPerSample)) + L", ";
    debugInfo += L"BlockAlign=" + IntToStr(static_cast<int>(PWaveFormat.nBlockAlign)) + L", ";
    debugInfo += L"AvgBytesPerSec=" + IntToStr(static_cast<int>(PWaveFormat.nAvgBytesPerSec)) + L", ";
    debugInfo += L"BufferSize=" + IntToStr(static_cast<int>(PBufferSize));
    OutputDebugString(debugInfo.c_str());
    
    LPDIRECTSOUNDBUFFER tempBuffer = NULL;
    HRESULT hr = PDirectSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
    if (FAILED(hr)) {
        OutputDebugString(L"Failed to create sound buffer");
        return hr;
    }
    
    // Obtenir l'interface DirectSoundBuffer8
    hr = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&PSecondaryBuffer);
    tempBuffer->Release();
    
    if (FAILED(hr)) {
        OutputDebugString(L"Failed to get DirectSoundBuffer8 interface");
        return hr;
    }
    
    OutputDebugString(L"Sound buffer created successfully");
    return S_OK;
}

HRESULT __fastcall XFFmpeg::SetupNotifications() {
    OutputDebugString(L"Setting up notifications");
    
    // Libérer les notifications précédentes
    SAFE_RELEASE(PNotify);
    
    // Fermer les événements précédents
    if (PNotifyEvents[0]) {
        CloseHandle(PNotifyEvents[0]);
        PNotifyEvents[0] = NULL;
    }
    
    if (PNotifyEvents[1]) {
        CloseHandle(PNotifyEvents[1]);
        PNotifyEvents[1] = NULL;
    }
    
    // Créer les événements de notification
    PNotifyEvents[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
    PNotifyEvents[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
    
    if (!PNotifyEvents[0] || !PNotifyEvents[1]) {
        OutputDebugString(L"Failed to create notification events");
        return E_FAIL;
    }
    
    // Obtenir l'interface DirectSoundNotify
    HRESULT hr = PSecondaryBuffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&PNotify);
    if (FAILED(hr)) {
        OutputDebugString(L"Failed to get DirectSoundNotify8 interface");
        return hr;
    }
    
    // Configurer les positions de notification
    DSBPOSITIONNOTIFY notifyPositions[2];
    notifyPositions[0].dwOffset = PBufferSize / 2 - 1;
    notifyPositions[0].hEventNotify = PNotifyEvents[0];
    notifyPositions[1].dwOffset = PBufferSize - 1;
    notifyPositions[1].hEventNotify = PNotifyEvents[1];
    
    // Afficher les informations de notification
    UnicodeString debugInfo = L"Notification positions: ";
    debugInfo += L"Position1=" + IntToStr(static_cast<int>(notifyPositions[0].dwOffset)) + L", ";
    debugInfo += L"Position2=" + IntToStr(static_cast<int>(notifyPositions[1].dwOffset));
    OutputDebugString(debugInfo.c_str());
    
    hr = PNotify->SetNotificationPositions(2, notifyPositions);
    if (FAILED(hr)) {
        OutputDebugString(L"Failed to set notification positions");
        SAFE_RELEASE(PNotify);
        return hr;
    }
    
    OutputDebugString(L"Notifications set up successfully");
    return S_OK;
}

void __fastcall XFFmpeg::CleanupDirectSound() {
    // Arrêter la lecture
    Stop();
    
    // Libérer les ressources DirectSound
    SAFE_RELEASE(PNotify);
    SAFE_RELEASE(PSecondaryBuffer);
    SAFE_RELEASE(PPrimaryBuffer);
    SAFE_RELEASE(PDirectSound);
    
    // Fermer les événements de notification
    if (PNotifyEvents[0]) {
        CloseHandle(PNotifyEvents[0]);
        PNotifyEvents[0] = NULL;
    }
    
    if (PNotifyEvents[1]) {
        CloseHandle(PNotifyEvents[1]);
        PNotifyEvents[1] = NULL;
    }
}

void __fastcall XFFmpeg::CleanupFFmpeg() {
    // Libérer le buffer audio
    if (PAudioBuffer) {
        av_free(PAudioBuffer);
        PAudioBuffer = NULL;
    }
    
    // Libérer la trame
    if (PFrame) {
        av_frame_free(&PFrame);
        PFrame = NULL;
    }
    
    // Libérer le paquet
    if (PPacket) {
        av_packet_free(&PPacket);
        PPacket = NULL;
    }
    
    // Fermer le resampler
    if (PSwrContext) {
        swr_free(&PSwrContext);
        PSwrContext = NULL;
    }
    
    // Fermer le codec
    if (PCodecContext) {
        avcodec_free_context(&PCodecContext);
        PCodecContext = NULL;
    }
    
    // Fermer le fichier
    if (PFormatContext) {
        avformat_close_input(&PFormatContext);
        PFormatContext = NULL;
    }
    
    PAudioStreamIndex = -1;
    PAudioBufferSize = 0;
    PAudioBufferPos = 0;
}

DWORD WINAPI XFFmpeg::PlaybackThread(LPVOID lpParameter) {
    XFFmpeg* pThis = static_cast<XFFmpeg*>(lpParameter);
    if (!pThis) {
        return 0;
    }
    
    // Tableau des événements à attendre
    HANDLE events[2];
    events[0] = pThis->PNotifyEvents[0];
    events[1] = pThis->PNotifyEvents[1];
    
    // Vérifier que les événements sont valides
    if (!events[0] || !events[1]) {
        OutputDebugString(L"Invalid notification events in PlaybackThread");
        return 0;
    }
    
    OutputDebugString(L"PlaybackThread started");
    
    // Boucle de lecture
    while (pThis->PThreadActive) {
        // Attendre une notification
        DWORD result = WaitForMultipleObjects(2, events, FALSE, 100);
        
        // Vérifier si le thread doit s'arrêter
        if (!pThis->PThreadActive) {
            OutputDebugString(L"PlaybackThread stopped");
            break;
        }
        
        // Traiter la notification
        if (result == WAIT_OBJECT_0 || result == WAIT_OBJECT_0 + 1) {
            OutputDebugString(L"Processing audio in PlaybackThread");
            pThis->ProcessAudio();
        }
        else if (result == WAIT_TIMEOUT) {
            // Timeout, continuer la boucle
            continue;
        }
        else {
            // Erreur
            OutputDebugString(L"Error in WaitForMultipleObjects in PlaybackThread");
            break;
        }
    }
    
    OutputDebugString(L"PlaybackThread exiting");
    return 0;
}

void __fastcall XFFmpeg::ProcessAudio() {
    if (!PSecondaryBuffer || !PFormatContext) {
        return;
    }
    
    // Obtenir la position actuelle
    DWORD playPosition = 0;
    DWORD writePosition = 0;
    
    HRESULT hr = PSecondaryBuffer->GetCurrentPosition(&playPosition, &writePosition);
    if (FAILED(hr)) {
        return;
    }
    
    // Si c'est le premier appel, remplir tout le buffer
    if (!PIsPlaying) {
        // Verrouiller le buffer entier
        LPVOID buffer1 = NULL;
        DWORD buffer1Size = 0;
        LPVOID buffer2 = NULL;
        DWORD buffer2Size = 0;
        
        hr = PSecondaryBuffer->Lock(0, PBufferSize, &buffer1, &buffer1Size, &buffer2, &buffer2Size, 0);
        if (FAILED(hr)) {
            return;
        }
        
        // Remplir le buffer avec des données audio
        BYTE* pBuffer1 = static_cast<BYTE*>(buffer1);
        DWORD bytesToWrite1 = buffer1Size;
        DWORD bytesWritten1 = 0;
        
        while (bytesWritten1 < bytesToWrite1) {
            // Si le buffer audio est vide, décoder plus de données
            if (PAudioBufferPos >= PAudioBufferSize) {
                if (!DecodeAudioPacket()) {
                    // Fin du fichier ou erreur, remplir le reste avec des zéros
                    ZeroMemory(pBuffer1 + bytesWritten1, bytesToWrite1 - bytesWritten1);
                    break;
                }
            }
            
            // Copier les données du buffer audio au buffer DirectSound
            DWORD bytesToCopy = min(bytesToWrite1 - bytesWritten1, PAudioBufferSize - PAudioBufferPos);
            memcpy(pBuffer1 + bytesWritten1, PAudioBuffer + PAudioBufferPos, bytesToCopy);
            
            bytesWritten1 += bytesToCopy;
            PAudioBufferPos += bytesToCopy;
        }
        
        // Remplir la deuxième partie du buffer (si nécessaire)
        if (buffer2 && buffer2Size > 0) {
            BYTE* pBuffer2 = static_cast<BYTE*>(buffer2);
            DWORD bytesToWrite2 = buffer2Size;
            DWORD bytesWritten2 = 0;
            
            while (bytesWritten2 < bytesToWrite2) {
                // Si le buffer audio est vide, décoder plus de données
                if (PAudioBufferPos >= PAudioBufferSize) {
                    if (!DecodeAudioPacket()) {
                        // Fin du fichier ou erreur, remplir le reste avec des zéros
                        ZeroMemory(pBuffer2 + bytesWritten2, bytesToWrite2 - bytesWritten2);
                        break;
                    }
                }
                
                // Copier les données du buffer audio au buffer DirectSound
                DWORD bytesToCopy = min(bytesToWrite2 - bytesWritten2, PAudioBufferSize - PAudioBufferPos);
                memcpy(pBuffer2 + bytesWritten2, PAudioBuffer + PAudioBufferPos, bytesToCopy);
                
                bytesWritten2 += bytesToCopy;
                PAudioBufferPos += bytesToCopy;
            }
        }
        
        // Déverrouiller le buffer
        PSecondaryBuffer->Unlock(buffer1, buffer1Size, buffer2, buffer2Size);
    }
    else {
        // Déterminer quelle moitié du buffer doit être remplie
        DWORD lockOffset = 0;
        DWORD lockSize = PBufferSize / 2;
        
        if (playPosition < PBufferSize / 2) {
            lockOffset = PBufferSize / 2;
        }
        
        // Verrouiller le buffer pour écrire les données
        LPVOID buffer1 = NULL;
        DWORD buffer1Size = 0;
        LPVOID buffer2 = NULL;
        DWORD buffer2Size = 0;
        
        hr = PSecondaryBuffer->Lock(lockOffset, lockSize, &buffer1, &buffer1Size, &buffer2, &buffer2Size, 0);
        if (FAILED(hr)) {
            return;
        }
        
        // Remplir le buffer avec des données audio
        BYTE* pBuffer1 = static_cast<BYTE*>(buffer1);
        DWORD bytesToWrite1 = buffer1Size;
        DWORD bytesWritten1 = 0;
        
        while (bytesWritten1 < bytesToWrite1) {
            // Si le buffer audio est vide, décoder plus de données
            if (PAudioBufferPos >= PAudioBufferSize) {
                if (!DecodeAudioPacket()) {
                    // Fin du fichier ou erreur, remplir le reste avec des zéros
                    ZeroMemory(pBuffer1 + bytesWritten1, bytesToWrite1 - bytesWritten1);
                    break;
                }
            }
            
            // Copier les données du buffer audio au buffer DirectSound
            DWORD bytesToCopy = min(bytesToWrite1 - bytesWritten1, PAudioBufferSize - PAudioBufferPos);
            memcpy(pBuffer1 + bytesWritten1, PAudioBuffer + PAudioBufferPos, bytesToCopy);
            
            bytesWritten1 += bytesToCopy;
            PAudioBufferPos += bytesToCopy;
        }
        
        // Remplir la deuxième partie du buffer (si nécessaire)
        if (buffer2 && buffer2Size > 0) {
            BYTE* pBuffer2 = static_cast<BYTE*>(buffer2);
            DWORD bytesToWrite2 = buffer2Size;
            DWORD bytesWritten2 = 0;
            
            while (bytesWritten2 < bytesToWrite2) {
                // Si le buffer audio est vide, décoder plus de données
                if (PAudioBufferPos >= PAudioBufferSize) {
                    if (!DecodeAudioPacket()) {
                        // Fin du fichier ou erreur, remplir le reste avec des zéros
                        ZeroMemory(pBuffer2 + bytesWritten2, bytesToWrite2 - bytesWritten2);
                        break;
                    }
                }
                
                // Copier les données du buffer audio au buffer DirectSound
                DWORD bytesToCopy = min(bytesToWrite2 - bytesWritten2, PAudioBufferSize - PAudioBufferPos);
                memcpy(pBuffer2 + bytesWritten2, PAudioBuffer + PAudioBufferPos, bytesToCopy);
                
                bytesWritten2 += bytesToCopy;
                PAudioBufferPos += bytesToCopy;
            }
        }
        
        // Déverrouiller le buffer
        PSecondaryBuffer->Unlock(buffer1, buffer1Size, buffer2, buffer2Size);
    }
}

bool __fastcall XFFmpeg::DecodeAudioPacket() {
    if (!PFormatContext || !PCodecContext || !PSwrContext || !PPacket || !PFrame) {
        return false;
    }
    
    // Réinitialiser la position du buffer audio
    PAudioBufferPos = 0;
    
    // Lire les paquets jusqu'à ce qu'on obtienne des données audio
    while (true) {
        // Lire un paquet
        int result = av_read_frame(PFormatContext, PPacket);
        if (result < 0) {
            // Fin du fichier ou erreur
            return false;
        }
        
        // Vérifier si c'est un paquet audio
        if (PPacket->stream_index != PAudioStreamIndex) {
            // Libérer le paquet et continuer
            av_packet_unref(PPacket);
            continue;
        }
        
        // Envoyer le paquet au décodeur
        result = avcodec_send_packet(PCodecContext, PPacket);
        if (result < 0) {
            // Erreur d'envoi du paquet
            av_packet_unref(PPacket);
            return false;
        }
        
        // Recevoir les trames décodées
        result = avcodec_receive_frame(PCodecContext, PFrame);
        if (result < 0) {
            // Erreur de réception de la trame ou besoin de plus de données
            av_packet_unref(PPacket);
            if (result == AVERROR(EAGAIN)) {
                // Besoin de plus de données, continuer
                continue;
            }
            return false;
        }
        
        // Libérer le paquet
        av_packet_unref(PPacket);
        
        // Convertir la trame en PCM
        int outSamples = swr_convert(PSwrContext,
                                    &PAudioBuffer,
                                    PAudioBufferSize / PWaveFormat.nBlockAlign,
                                    (const uint8_t**)PFrame->data,
                                    PFrame->nb_samples);
        
        if (outSamples <= 0) {
            // Erreur de conversion
            return false;
        }
        
        // Calculer la taille des données converties
        PAudioBufferSize = outSamples * PWaveFormat.nBlockAlign;
        
        // Libérer la trame
        av_frame_unref(PFrame);
        
        return true;
    }
}

void __fastcall XFFmpeg::CalculateFFT(const BYTE* pData, DWORD dataSize) {
    // Cette implémentation est simplifiée et ne calcule pas réellement la FFT
    // Pour une implémentation complète, vous auriez besoin d'une bibliothèque FFT
    
    // Réinitialiser les tableaux
    ZeroMemory(PFFT, sizeof(PFFT));
    ZeroMemory(PRawData, sizeof(PRawData));
    
    // Si pas de données, retourner
    if (!pData || dataSize == 0) {
        return;
    }
    
    // Calculer le nombre d'échantillons
    DWORD numSamples = dataSize / (PWaveFormat.wBitsPerSample / 8);
    DWORD numChannels = PWaveFormat.nChannels;
    
    // Limiter le nombre d'échantillons à traiter
    if (numSamples > 4096) numSamples = 4096;
    
    // Tableau temporaire pour stocker les échantillons audio
    float samples[4096] = {0};
    
    // Convertir les données audio en valeurs flottantes entre -1 et 1
    for (DWORD i = 0; i < numSamples && i < 4096; i++) {
        float value = 0.0f;
        
        if (PWaveFormat.wBitsPerSample == 8) {
            // Échantillons 8 bits (0-255)
            value = (static_cast<float>(pData[i]) - 128.0f) / 128.0f;
        } else if (PWaveFormat.wBitsPerSample == 16) {
            // Échantillons 16 bits (-32768 à 32767)
            short sample = ((short*)pData)[i];
            value = static_cast<float>(sample) / 32768.0f;
        } else if (PWaveFormat.wBitsPerSample == 24) {
            // Échantillons 24 bits (traitement simplifié)
            int sample = (static_cast<int>(pData[i*3]) << 8) |
                         (static_cast<int>(pData[i*3+1]) << 16) |
                         (static_cast<int>(pData[i*3+2]) << 24);
            sample >>= 8; // Ajuster à 24 bits
            value = static_cast<float>(sample) / 8388608.0f;
        } else if (PWaveFormat.wBitsPerSample == 32) {
            // Échantillons 32 bits (float)
            if (PWaveFormat.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
                value = ((float*)pData)[i];
            } else {
                // Échantillons 32 bits (int)
                int sample = ((int*)pData)[i];
                value = static_cast<float>(sample) / 2147483648.0f;
            }
        }
        
        // Stocker la valeur dans le tableau d'échantillons
        samples[i] = value;
    }
    
    // Mixer les canaux si nécessaire
    if (numChannels > 1) {
        for (DWORD i = 0; i < numSamples / numChannels && i < 4096; i++) {
            float mixedValue = 0.0f;
            for (DWORD ch = 0; ch < numChannels; ch++) {
                mixedValue += samples[i * numChannels + ch];
            }
            mixedValue /= numChannels;
            PRawData[i] = mixedValue;
        }
    } else {
        // Copier les échantillons dans le tableau de données brutes
        for (DWORD i = 0; i < numSamples && i < 4096; i++) {
            PRawData[i] = samples[i];
        }
    }
    
    // Simuler une FFT en créant un spectre à partir des données brutes
    // Cette méthode ne calcule pas réellement la FFT, mais crée un spectre visuellement plausible
    for (int i = 0; i < 512; i++) {
        // Calculer une valeur basée sur la position dans le spectre
        // Les fréquences plus basses (indices plus petits) ont généralement plus d'énergie
        float freqFactor = 1.0f - (i / 512.0f);
        
        // Calculer une moyenne pondérée des échantillons pour cette "bande de fréquence"
        float sum = 0.0f;
        int count = 0;
        
        // Utiliser différentes parties des données brutes pour différentes bandes de fréquence
        int startIdx = (i * numSamples / 512) % 4096;
        int endIdx = ((i + 1) * numSamples / 512) % 4096;
        
        for (int j = startIdx; j < endIdx && j < 4096; j++) {
            sum += fabsf(PRawData[j]);
            count++;
        }
        
        // Calculer la moyenne et appliquer des facteurs pour rendre le spectre plus intéressant
        float value = (count > 0) ? (sum / count) : 0.0f;
        value = value * freqFactor * 2.0f; // Amplifier les basses fréquences
        
        // Appliquer le volume
        value *= PVolume;
        
        // Limiter la valeur entre 0 et 1
        if (value < 0.0f) value = 0.0f;
        if (value > 1.0f) value = 1.0f;
        
        // Stocker la valeur dans le tableau FFT
        PFFT[i] = value;
    }
}