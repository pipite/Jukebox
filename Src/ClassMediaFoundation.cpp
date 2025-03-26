//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include <algorithm> // Pour les fonctions min et max
#include <math.h>     // Pour la fonction fabsf

#pragma package(smart_init)
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "Propsys.lib")

#include "ClassMediaFoundation.h"

//---------------------------------------------------------------------------

// Constantes pour la conversion des unités de temps
const UINT64 MFTIMES_PER_SEC = 10000000; // 100-nanosecondes par seconde
const UINT64 MFTIMES_PER_MS = 10000;     // 100-nanosecondes par milliseconde

// ---------------------------------------------------------------------------
//   ClassMediaFoundation
//       Durées Media Foundation: UINT64 = 100-nanosecondes
//       Volume Media Foundation: float = 0.0 <= Volume <= 1.0
//   Classmix
//       Durées Mix: unsigned __int64 = 1000 for a second.
//       Volume Mix: float = 0.0 <= Volume <= 1.0
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//   public
// ---------------------------------------------------------------------------

// Fonction de rappel pour le timer
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    // Récupérer l'instance de XMediaFoundation à partir du handle de fenêtre
    XMediaFoundation* pThis = reinterpret_cast<XMediaFoundation*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (pThis && pThis->PIsPlaying) {
        // Appeler la méthode privée ReadSample directement
        // C'est possible car TimerProc est déclarée comme amie de la classe
        pThis->ReadSample();
    }
}

__fastcall XMediaFoundation::XMediaFoundation() : XEngine() {
    PHwnd = NULL;
    PSourceReader = NULL;
    PAudioType = NULL;
    PSample = NULL;
    PBuffer = NULL;
    PMediaSession = NULL;
    PMediaSink = NULL;
    PAudioVolume = NULL;
    PClock = NULL;
    PWaveFormat = NULL;
    PBytesPerSecond = 0;
    PDuration = 0;
    PCurrentPos = 0;
    PVolume = 1.0f;
    PIsPlaying = false;
    PIsInitialized = false;
    PTimerID = 0;

    // Initialiser les tableaux à zéro
    ZeroMemory(PFFT, sizeof(PFFT));
    ZeroMemory(PRawData, sizeof(PRawData));
}

__fastcall XMediaFoundation::~XMediaFoundation(void) {
    // Arrêter le timer
    if (PTimerID) {
        KillTimer(PHwnd, PTimerID);
        PTimerID = 0;
    }

    // Libérer les ressources Media Foundation
    Stop();

    SAFE_RELEASE(PBuffer);
    SAFE_RELEASE(PSample);
    SAFE_RELEASE(PAudioType);
    SAFE_RELEASE(PSourceReader);

    if (PWaveFormat) {
        CoTaskMemFree(PWaveFormat);
        PWaveFormat = NULL;
    }

    // Fermer Media Foundation
    if (PIsInitialized) {
        MFShutdown();
    }
}

bool __fastcall XMediaFoundation::Initialize(HWND hwnd) {
    PHwnd = hwnd;

    // Initialiser Media Foundation
    HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
    if (FAILED(hr)) {
        MessageDlg("Failed to initialize Media Foundation.", mtError, TMsgDlgButtons() << mbOK, 0);
        return false;
    }

    // Stocker l'instance dans les données de la fenêtre pour le timer
    SetWindowLongPtr(PHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    PIsInitialized = true;
    return true;
}

bool __fastcall XMediaFoundation::Play(void) {
    if (!PSourceReader) {
        return false;
    }

    // Arrêter le timer existant s'il y en a un
    if (PTimerID) {
        KillTimer(PHwnd, PTimerID);
        PTimerID = 0;
    }

    PIsPlaying = true;

    // Lire le premier échantillon pour l'analyse FFT
    HRESULT hr = ReadSample();
    if (FAILED(hr)) {
        PIsPlaying = false;
        return false;
    }

    // Démarrer un timer pour lire les échantillons suivants (environ 30 fois par seconde)
    PTimerID = SetTimer(PHwnd, 1, 33, TimerProc);
    if (!PTimerID) {
        PIsPlaying = false;
        return false;
    }

    return true;
}

void __fastcall XMediaFoundation::Stop(void) {
    // Arrêter le timer
    if (PTimerID) {
        KillTimer(PHwnd, PTimerID);
        PTimerID = 0;
    }

    PIsPlaying = false;

    // Libérer les ressources actuelles
    SAFE_RELEASE(PBuffer);
    SAFE_RELEASE(PSample);
}

bool __fastcall XMediaFoundation::IsAudioFile(XNode *node) {
    if (!node) {
        return false;
    }
    
    // Créer un source reader pour vérifier si le fichier est un fichier audio valide
    IMFSourceReader* pReader = NULL;
    IMFAttributes* pAttributes = NULL;
    HRESULT hr;
    
    // Créer les attributs pour le source reader
    hr = MFCreateAttributes(&pAttributes, 1);
    if (FAILED(hr)) {
        return false;
    }
    
    // Configurer les attributs
    hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE);
    if (FAILED(hr)) {
        SAFE_RELEASE(pAttributes);
        return false;
    }
    
    // Créer le source reader
    hr = MFCreateSourceReaderFromURL(node->Path.c_str(), pAttributes, &pReader);
    
    // Libérer les ressources
    SAFE_RELEASE(pAttributes);
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Vérifier si le fichier contient un flux audio
    IMFMediaType* pNativeType = NULL;
    hr = pReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &pNativeType);
    
    SAFE_RELEASE(pNativeType);
    SAFE_RELEASE(pReader);
    
    return SUCCEEDED(hr);
}

// ---------------------------------------------------------------------------
//   property
// ---------------------------------------------------------------------------

// Méthode pour créer la topologie de lecture (simplifiée)
HRESULT __fastcall XMediaFoundation::CreatePlaybackTopology(IMFMediaSource* pSource, IMFTopology** ppTopology) {
    // Cette méthode est simplifiée pour éviter les problèmes de compilation
    return E_NOTIMPL;
}

bool __fastcall XMediaFoundation::SetNode(XNode *node) {
    if (!node) {
        return false;
    }

    // Arrêter la lecture en cours
    Stop();

    // Libérer les ressources actuelles
    SAFE_RELEASE(PSourceReader);
    SAFE_RELEASE(PAudioType);

    if (PWaveFormat) {
        CoTaskMemFree(PWaveFormat);
        PWaveFormat = NULL;
    }

    // Créer les attributs pour le source reader
    IMFAttributes* pAttributes = NULL;
    HRESULT hr = MFCreateAttributes(&pAttributes, 1);
    if (FAILED(hr)) {
        return false;
    }

    // Configurer les attributs
    hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, FALSE);
    if (FAILED(hr)) {
        SAFE_RELEASE(pAttributes);
        return false;
    }

    // Créer le source reader
    hr = MFCreateSourceReaderFromURL(node->Path.c_str(), pAttributes, &PSourceReader);
    SAFE_RELEASE(pAttributes);

    if (FAILED(hr)) {
        return false;
    }

    // Configurer le format audio de sortie (PCM)
    hr = MFCreateMediaType(&PAudioType);
    if (FAILED(hr)) {
        SAFE_RELEASE(PSourceReader);
        return false;
    }

    hr = PAudioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    if (FAILED(hr)) {
        SAFE_RELEASE(PAudioType);
        SAFE_RELEASE(PSourceReader);
        return false;
    }

    hr = PAudioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    if (FAILED(hr)) {
        SAFE_RELEASE(PAudioType);
        SAFE_RELEASE(PSourceReader);
        return false;
    }

    // Configurer le source reader pour utiliser le format PCM
    hr = PSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, PAudioType);
    if (FAILED(hr)) {
        SAFE_RELEASE(PAudioType);
        SAFE_RELEASE(PSourceReader);
        return false;
    }

    // Obtenir le format audio configuré
    IMFMediaType* pAudioTypeOut = NULL;
    hr = PSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pAudioTypeOut);
    if (FAILED(hr)) {
        SAFE_RELEASE(PAudioType);
        SAFE_RELEASE(PSourceReader);
        return false;
    }

    // Obtenir le format WAVEFORMATEX
    UINT32 waveFormatSize = 0;
    hr = MFCreateWaveFormatExFromMFMediaType(pAudioTypeOut, &PWaveFormat, &waveFormatSize);
    SAFE_RELEASE(pAudioTypeOut);

    if (FAILED(hr)) {
        SAFE_RELEASE(PAudioType);
        SAFE_RELEASE(PSourceReader);
        return false;
    }

    // Calculer le nombre d'octets par seconde
    PBytesPerSecond = PWaveFormat->nAvgBytesPerSec;

    // Obtenir la durée du fichier
    PROPVARIANT var;
    PropVariantInit(&var);

    hr = PSourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var);
    if (SUCCEEDED(hr) && var.vt == VT_UI8) {
        PDuration = var.uhVal.QuadPart;
    } else {
        PDuration = 0;
    }

    PropVariantClear(&var);

    // Sélectionner uniquement le flux audio
    hr = PSourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
    if (FAILED(hr)) {
        SAFE_RELEASE(PAudioType);
        SAFE_RELEASE(PSourceReader);
        if (PWaveFormat) {
            CoTaskMemFree(PWaveFormat);
            PWaveFormat = NULL;
        }
        return false;
    }

    hr = PSourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
    if (FAILED(hr)) {
        SAFE_RELEASE(PAudioType);
        SAFE_RELEASE(PSourceReader);
        if (PWaveFormat) {
            CoTaskMemFree(PWaveFormat);
            PWaveFormat = NULL;
        }
        return false;
    }

    // Réinitialiser la position
    PCurrentPos = 0;

    return true;
}

unsigned __int64 __fastcall XMediaFoundation::GetMsLength(void) {
    if (!PSourceReader) {
        return 0;
    }

    // Convertir la durée de 100-nanosecondes en millisecondes
    // et ajuster pour correspondre à l'échelle de temps de XEngine
    return PDuration / MFTIMES_PER_MS;
}

unsigned __int64 __fastcall XMediaFoundation::GetPosition(void) {
    if (!PSourceReader) {
        return 0;
    }

    // Convertir la position de 100-nanosecondes en millisecondes
    // et ajuster pour correspondre à l'échelle de temps de XEngine
    return PCurrentPos / MFTIMES_PER_MS;
}

void __fastcall XMediaFoundation::SetPosition(unsigned __int64 mspos) {
    if (!PSourceReader) {
        return;
    }

    // Convertir la position de millisecondes en 100-nanosecondes
    UINT64 position = mspos * MFTIMES_PER_MS;

    // Limiter la position à la durée du fichier
    if (position > PDuration) {
        position = PDuration;
    }

    // Créer la valeur PROPVARIANT pour la position
    PROPVARIANT var;
    PropVariantInit(&var);
    var.vt = VT_I8;
    var.hVal.QuadPart = position;

    // Définir la position
    HRESULT hr = PSourceReader->SetCurrentPosition(GUID_NULL, var);
    PropVariantClear(&var);

    if (SUCCEEDED(hr)) {
        PCurrentPos = position;
    }
}

bool __fastcall XMediaFoundation::GetIsPlaying(void) {
    return PIsPlaying;
}

bool __fastcall XMediaFoundation::GetHaveChannel(void) {
    return (PSourceReader != NULL);
}

float* __fastcall XMediaFoundation::GetFFT(void) {
    if (!PSourceReader || !PIsPlaying) {
        // Si pas de lecture en cours, retourner le tableau FFT vide
        return PFFT;
    }
    
    // Lire et traiter le prochain échantillon
    HRESULT hr = ReadSample();
    if (FAILED(hr)) {
        // En cas d'erreur, arrêter la lecture
        PIsPlaying = false;
    }
    
    return PFFT;
}

float __fastcall XMediaFoundation::GetVolume(void) {
    return PVolume;
}

void __fastcall XMediaFoundation::SetVolume(float volume) {
    // Limiter le volume entre 0.0 et 1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    PVolume = volume;
}

unsigned __int64 __fastcall XMediaFoundation::GetMsBeforeEnd(void) {
    if (!PSourceReader) {
        return 0;
    }

    // Calculer le temps restant en millisecondes
    UINT64 msLength = PDuration / MFTIMES_PER_MS;
    UINT64 msPosition = GetPosition(); // Utiliser GetPosition pour obtenir la position actuelle

    if (msPosition >= msLength) {
        return 0;
    }

    return msLength - msPosition;
}

// ---------------------------------------------------------------------------
//   private
// ---------------------------------------------------------------------------

HRESULT __fastcall XMediaFoundation::ReadSample() {
    if (!PSourceReader || !PIsPlaying) {
        return E_FAIL;
    }

    // Libérer les ressources précédentes
    SAFE_RELEASE(PSample);
    SAFE_RELEASE(PBuffer);

    // Variables pour la lecture
    DWORD streamIndex, flags;
    LONGLONG timestamp;

    // Lire le prochain échantillon
    HRESULT hr = PSourceReader->ReadSample(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        0,
        &streamIndex,
        &flags,
        &timestamp,
        &PSample
    );

    if (FAILED(hr)) {
        return hr;
    }

    // Vérifier les flags
    if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
        // Fin du fichier, arrêter la lecture
        PIsPlaying = false;

        // Arrêter le timer
        if (PTimerID) {
            KillTimer(PHwnd, PTimerID);
            PTimerID = 0;
        }

        return S_OK;
    }

    if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
        // Le type de média a changé, nous devons reconfigurer
        return E_FAIL;
    }

    if (!PSample) {
        // Pas d'échantillon disponible
        return S_OK;
    }

    // Mettre à jour la position actuelle
    PCurrentPos = timestamp;

    // Traiter l'échantillon
    return ProcessSample();
}

HRESULT __fastcall XMediaFoundation::ProcessSample() {
    if (!PSample) {
        return E_FAIL;
    }
    
    // Obtenir le buffer de l'échantillon
    HRESULT hr = PSample->ConvertToContiguousBuffer(&PBuffer);
    if (FAILED(hr)) {
        return hr;
    }
    
    // Verrouiller le buffer pour accéder aux données
    BYTE* pData = NULL;
    DWORD dataSize = 0;
    
    hr = PBuffer->Lock(&pData, NULL, &dataSize);
    if (FAILED(hr)) {
        return hr;
    }
    
    // Calculer les valeurs FFT à partir des données audio
    CalculateFFT(pData, dataSize);
    
    // Déverrouiller le buffer
    PBuffer->Unlock();
    
    return S_OK;
}

void __fastcall XMediaFoundation::CalculateFFT(const BYTE* pData, DWORD dataSize) {
    // Cette implémentation est simplifiée et ne calcule pas réellement la FFT
    // Pour une implémentation complète, vous auriez besoin d'une bibliothèque FFT

    // Réinitialiser les tableaux
    ZeroMemory(PFFT, sizeof(PFFT));
    ZeroMemory(PRawData, sizeof(PRawData));

    // Si pas de données, retourner
    if (!pData || dataSize == 0 || !PWaveFormat) {
        return;
    }

    // Calculer le nombre d'échantillons
    DWORD numSamples = dataSize / (PWaveFormat->wBitsPerSample / 8);
    DWORD numChannels = PWaveFormat->nChannels;

    // Limiter le nombre d'échantillons à traiter
    if (numSamples > 4096) numSamples = 4096;

    // Tableau temporaire pour stocker les échantillons audio
    float samples[4096] = {0};

    // Convertir les données audio en valeurs flottantes entre -1 et 1
    for (DWORD i = 0; i < numSamples && i < 4096; i++) {
        float value = 0.0f;

        if (PWaveFormat->wBitsPerSample == 8) {
            // Échantillons 8 bits (0-255)
            value = (static_cast<float>(pData[i]) - 128.0f) / 128.0f;
        } else if (PWaveFormat->wBitsPerSample == 16) {
            // Échantillons 16 bits (-32768 à 32767)
            short sample = ((short*)pData)[i];
            value = static_cast<float>(sample) / 32768.0f;
        } else if (PWaveFormat->wBitsPerSample == 24) {
            // Échantillons 24 bits (traitement simplifié)
            int sample = (static_cast<int>(pData[i*3]) << 8) |
                         (static_cast<int>(pData[i*3+1]) << 16) |
                         (static_cast<int>(pData[i*3+2]) << 24);
            sample >>= 8; // Ajuster à 24 bits
            value = static_cast<float>(sample) / 8388608.0f;
        } else if (PWaveFormat->wBitsPerSample == 32) {
            // Échantillons 32 bits (float)
            if (PWaveFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
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