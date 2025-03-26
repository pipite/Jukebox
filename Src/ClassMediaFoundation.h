//---------------------------------------------------------------------------
#ifndef ClassMediaFoundationH
#define ClassMediaFoundationH

class XMediaFoundation;

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>
#include <evr.h>
#include <mmreg.h> // Pour WAVE_FORMAT_IEEE_FLOAT

// Déclaration anticipée de la fonction de rappel du timer
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

#include "VirtualEngine.h"
#include "ClassNode.h"

// Macro pour libérer les interfaces COM
#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }

//---------------------------------------------------------------------------
class XMediaFoundation : public XEngine {
    // Déclarer la fonction TimerProc comme amie de la classe
    friend VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
private:
    HWND                    PHwnd;
    IMFSourceReader*        PSourceReader;
    IMFMediaType*           PAudioType;
    IMFSample*              PSample;
    IMFMediaBuffer*         PBuffer;
    IMFMediaSession*        PMediaSession;
    IMFMediaSink*           PMediaSink;
    IMFAudioStreamVolume*   PAudioVolume;
    IMFPresentationClock*   PClock;

    WAVEFORMATEX*           PWaveFormat;
    UINT32                  PBytesPerSecond;
    UINT64                  PDuration;      // Durée en 100-nanosecondes
    UINT64                  PCurrentPos;    // Position actuelle en 100-nanosecondes

    float                   PFFT[512];      // Données FFT
    float                   PRawData[4096]; // Données audio brutes pour l'affichage de la forme d'onde
    float                   PVolume;
    bool                    PIsPlaying;
    bool                    PIsInitialized;

    // Timer pour la lecture continue
    UINT                    PTimerID;

    // Méthode pour créer la topologie de lecture
    HRESULT                 __fastcall CreatePlaybackTopology(IMFMediaSource* pSource, IMFTopology** ppTopology);
    
    // Méthodes privées pour la gestion des données audio
    HRESULT                 __fastcall ProcessSample();
    HRESULT                 __fastcall ReadSample();
    void                    __fastcall CalculateFFT(const BYTE* pData, DWORD dataSize);
    
    // Méthodes d'accès aux propriétés
    bool                    __fastcall GetHaveChannel(void);
    float*                  __fastcall GetFFT(void);
    float                   __fastcall GetVolume(void);
    unsigned __int64        __fastcall GetMsLength(void);
    unsigned __int64        __fastcall GetPosition(void);
    unsigned __int64        __fastcall GetMsBeforeEnd(void);
    bool                    __fastcall GetIsPlaying(void);
    bool                    __fastcall SetNode(XNode *node);
    void                    __fastcall SetVolume(float volume);
    void                    __fastcall SetPosition(unsigned __int64 mspos);

public:
    XMediaFoundation();
    ~XMediaFoundation(void);

    bool __fastcall Initialize(HWND hwnd);
    bool __fastcall Play(void);
    void __fastcall Stop(void);
    bool __fastcall IsAudioFile(XNode *node);

    __property XNode*           Node        = {                      write = SetNode};
    __property unsigned __int64 MsLength    = {read = GetMsLength};
    __property unsigned __int64 Position    = {read = GetPosition,   write = SetPosition};
    __property bool             IsPlaying   = {read = GetIsPlaying};
    __property bool             HaveChannel = {read = GetHaveChannel};
    __property float*           FFT         = {read = GetFFT};
    __property float            Volume      = {read = GetVolume,     write = SetVolume};
    __property unsigned __int64 MsBeforeEnd = {read = GetMsBeforeEnd};
};
#endif