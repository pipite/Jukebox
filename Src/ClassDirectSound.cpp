//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

//#include <algorithm> // Pour les fonctions min et max
//#include <math.h>     // Pour la fonction fabsf
#include "ClassDirectSound.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "winmm.lib")

// ---------------------------------------------------------------------------
//   ClassDirectSound
//       Durées DirectSound: DWORD = millisecondes
//       Volume DirectSound: float = 0.0 <= Volume <= 1.0
//   Classmix
//       Durées Mix: unsigned __int64 = 1000 for a second.
//       Volume Mix: float = 0.0 <= Volume <= 1.0
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//   public
// ---------------------------------------------------------------------------

__fastcall XDirectSound::XDirectSound() : XEngine() {
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
    
    PAudioFile = NULL;
    PFileSize = 0;
    PFilePosition = 0;
    PDuration = 0;
    
    PVolume = 1.0f;
    PIsPlaying = false;
    PIsInitialized = false;
    
    // Initialiser les tableaux à zéro
    ZeroMemory(PFFT, sizeof(PFFT));
    ZeroMemory(PRawData, sizeof(PRawData));
}

__fastcall XDirectSound::~XDirectSound(void) {
    // Arrêter la lecture
    Stop();
    
    // Nettoyer DirectSound
    CleanupDirectSound();
}

bool __fastcall XDirectSound::Initialize(HWND hwnd) {
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

bool __fastcall XDirectSound::Play(void) {
    if (!PSecondaryBuffer || !PAudioFile) {
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

void __fastcall XDirectSound::Stop(void) {
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

bool __fastcall XDirectSound::IsAudioFile(XNode *node) {
    if (!node) {
        return false;
    }
    
    // Ouvrir le fichier
    FILE* file = NULL;
    errno_t err = _wfopen_s(&file, node->Path.c_str(), L"rb");
    if (err != 0 || !file) {
        return false;
    }
    
    // Lire l'en-tête WAV
    char header[12];
    size_t bytesRead = fread(header, 1, 12, file);
    fclose(file);
    
    if (bytesRead < 12) {
        return false;
    }
    
    // Vérifier si c'est un fichier WAV
    if (memcmp(header, "RIFF", 4) == 0 && memcmp(header + 8, "WAVE", 4) == 0) {
        return true;
    }
    
    return false;
}

// ---------------------------------------------------------------------------
//   property
// ---------------------------------------------------------------------------

bool __fastcall XDirectSound::SetNode(XNode *node) {
    if (!node) {
        return false;
    }

    // Arrêter la lecture en cours
    Stop();

    // Fermer le fichier audio précédent
    if (PAudioFile) {
        fclose(PAudioFile);
        PAudioFile = NULL;
    }

    // Ouvrir le fichier audio
    errno_t err = _wfopen_s(&PAudioFile, node->Path.c_str(), L"rb");
    if (err != 0 || !PAudioFile) {
        return false;
    }

    try {
        // Lire l'en-tête WAV
        char header[44];
        size_t bytesRead = fread(header, 1, 44, PAudioFile);
        if (bytesRead < 44) {
            throw Exception("Failed to read WAV header");
        }

        // Vérifier si c'est un fichier WAV
        if (memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0) {
            throw Exception("Not a valid WAV file");
        }

        // Extraire les informations du format
        WAVEFORMATEX waveFormat;
        ZeroMemory(&waveFormat, sizeof(waveFormat));
        waveFormat.wFormatTag = *((WORD*)(header + 20));
        waveFormat.nChannels = *((WORD*)(header + 22));
        waveFormat.nSamplesPerSec = *((DWORD*)(header + 24));
        waveFormat.nAvgBytesPerSec = *((DWORD*)(header + 28));
        waveFormat.nBlockAlign = *((WORD*)(header + 32));
        waveFormat.wBitsPerSample = *((WORD*)(header + 34));

        // Trouver le chunk de données
        DWORD dataSize = 0;
        char chunkID[4];
        DWORD chunkSize = 0;

        while (true) {
            bytesRead = fread(chunkID, 1, 4, PAudioFile);
            if (bytesRead < 4) {
                throw Exception("Failed to read chunk ID");
            }

            bytesRead = fread(&chunkSize, 1, 4, PAudioFile);
            if (bytesRead < 4) {
                throw Exception("Failed to read chunk size");
            }

            if (memcmp(chunkID, "data", 4) == 0) {
                dataSize = chunkSize;
                break;
            }

            // Passer au chunk suivant
            fseek(PAudioFile, chunkSize, SEEK_CUR);
        }

        // Calculer la durée du fichier
        PDuration = (dataSize * 1000) / waveFormat.nAvgBytesPerSec;

        // Stocker la position du début des données
        PFilePosition = ftell(PAudioFile);

        // Stocker la taille du fichier
        PFileSize = dataSize;

        // Copier le format audio
        memcpy(&PWaveFormat, &waveFormat, sizeof(WAVEFORMATEX));

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

        // Fermer le fichier audio
        if (PAudioFile) {
            fclose(PAudioFile);
            PAudioFile = NULL;
        }

        return false;
    }
}

unsigned __int64 __fastcall XDirectSound::GetMsLength(void) {
    return PDuration;
}

unsigned __int64 __fastcall XDirectSound::GetPosition(void) {
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

void __fastcall XDirectSound::SetPosition(unsigned __int64 mspos) {
    if (!PSecondaryBuffer || !PAudioFile) {
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
    fseek(PAudioFile, PFilePosition + bytePosition, SEEK_SET);
}

bool __fastcall XDirectSound::GetIsPlaying(void) {
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

bool __fastcall XDirectSound::GetHaveChannel(void) {
    return (PSecondaryBuffer != NULL && PAudioFile != NULL);
}

float* __fastcall XDirectSound::GetFFT(void) {
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

float __fastcall XDirectSound::GetVolume(void) {
    return PVolume;
}

void __fastcall XDirectSound::SetVolume(float volume) {
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

unsigned __int64 __fastcall XDirectSound::GetMsBeforeEnd(void) {
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

HRESULT __fastcall XDirectSound::InitializeDirectSound() {
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

HRESULT __fastcall XDirectSound::CreateSoundBuffer() {
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

HRESULT __fastcall XDirectSound::SetupNotifications() {
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

void __fastcall XDirectSound::CleanupDirectSound() {
    // Arrêter la lecture
    Stop();
    
    // Fermer le fichier audio
    if (PAudioFile) {
        fclose(PAudioFile);
        PAudioFile = NULL;
    }
    
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

DWORD WINAPI XDirectSound::PlaybackThread(LPVOID lpParameter) {
    XDirectSound* pThis = static_cast<XDirectSound*>(lpParameter);
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

void __fastcall XDirectSound::ProcessAudio() {
    if (!PSecondaryBuffer || !PAudioFile) {
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

        // Lire les données du fichier pour la première partie du buffer
        BYTE* pBuffer1 = static_cast<BYTE*>(buffer1);
        DWORD bytesToRead1 = buffer1Size;
        DWORD bytesRead1 = 0;

        if (PAudioFile) {
            bytesRead1 = fread(pBuffer1, 1, bytesToRead1, PAudioFile);

            // Si nous avons atteint la fin du fichier, remplir le reste du buffer avec des zéros
            if (bytesRead1 < bytesToRead1) {
                ZeroMemory(pBuffer1 + bytesRead1, bytesToRead1 - bytesRead1);

                // Revenir au début du fichier pour la lecture en boucle
                fseek(PAudioFile, PFilePosition, SEEK_SET);
            }
        }

        // Lire les données du fichier pour la deuxième partie du buffer (si nécessaire)
        if (buffer2 && buffer2Size > 0) {
            BYTE* pBuffer2 = static_cast<BYTE*>(buffer2);
            DWORD bytesToRead2 = buffer2Size;
            DWORD bytesRead2 = 0;

            if (PAudioFile) {
                bytesRead2 = fread(pBuffer2, 1, bytesToRead2, PAudioFile);

                // Si nous avons atteint la fin du fichier, remplir le reste du buffer avec des zéros
                if (bytesRead2 < bytesToRead2) {
                    ZeroMemory(pBuffer2 + bytesRead2, bytesToRead2 - bytesRead2);

                    // Revenir au début du fichier pour la lecture en boucle
                    fseek(PAudioFile, PFilePosition, SEEK_SET);
                }
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

        // Lire les données du fichier
        BYTE* pBuffer = static_cast<BYTE*>(buffer1);
        DWORD bytesToRead = buffer1Size;
        DWORD bytesRead = 0;

        if (PAudioFile) {
            bytesRead = fread(pBuffer, 1, bytesToRead, PAudioFile);

            // Si nous avons atteint la fin du fichier, remplir le reste du buffer avec des zéros
            if (bytesRead < bytesToRead) {
                ZeroMemory(pBuffer + bytesRead, bytesToRead - bytesRead);

                // Revenir au début du fichier pour la lecture en boucle
                fseek(PAudioFile, PFilePosition, SEEK_SET);
            }
        }

        // Lire les données du fichier pour la deuxième partie du buffer (si nécessaire)
        if (buffer2 && buffer2Size > 0) {
            BYTE* pBuffer2 = static_cast<BYTE*>(buffer2);
            DWORD bytesToRead2 = buffer2Size;
            DWORD bytesRead2 = 0;

            if (PAudioFile) {
                bytesRead2 = fread(pBuffer2, 1, bytesToRead2, PAudioFile);

                // Si nous avons atteint la fin du fichier, remplir le reste du buffer avec des zéros
                if (bytesRead2 < bytesToRead2) {
                    ZeroMemory(pBuffer2 + bytesRead2, bytesToRead2 - bytesRead2);

                    // Revenir au début du fichier pour la lecture en boucle
                    fseek(PAudioFile, PFilePosition, SEEK_SET);
                }
            }
        }

        // Déverrouiller le buffer
        PSecondaryBuffer->Unlock(buffer1, buffer1Size, buffer2, buffer2Size);
    }
}

void __fastcall XDirectSound::CalculateFFT(const BYTE* pData, DWORD dataSize) {
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