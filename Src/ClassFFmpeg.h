//---------------------------------------------------------------------------
#ifndef ClassFFmpegH
#define ClassFFmpegH

class XFFmpeg;

// Inclusions FFmpeg
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libswresample/swresample.h>
}

// Inclusions DirectSound
#include <dsound.h>

// Inclusions pour la FFT
#include <complex>
#include <vector>

#include "VirtualEngine.h"
#include "ClassNode.h"

// Macro pour libérer les interfaces COM
#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }

// Taille du buffer DirectSound (en millisecondes)
#define DS_BUFFER_SIZE 1000

// Taille du buffer de lecture (en octets)
#define READ_BUFFER_SIZE 4096

// Taille de la FFT
#define FFT_SIZE 1024

//---------------------------------------------------------------------------
class XFFmpeg : public XEngine {
private:
    // DirectSound
    HWND                    PHwnd;
    LPDIRECTSOUND8          PDirectSound;
    LPDIRECTSOUNDBUFFER     PPrimaryBuffer;
    LPDIRECTSOUNDBUFFER8    PSecondaryBuffer;
    LPDIRECTSOUNDNOTIFY8    PNotify;
    
    HANDLE                  PNotifyEvents[2];
    HANDLE                  PThread;
    DWORD                   PThreadID;
    bool                    PThreadActive;
    
    WAVEFORMATEX            PWaveFormat;
    DWORD                   PBufferSize;
    DWORD                   PReadPosition;
    DWORD                   PWritePosition;
    
    // FFmpeg
    AVFormatContext*        PFormatContext;
    AVCodecContext*         PCodecContext;
    SwrContext*             PSwrContext;
    int                     PAudioStreamIndex;
    AVPacket*               PPacket;
    AVFrame*                PFrame;
    uint8_t*                PAudioBuffer;
    size_t                  PAudioBufferSize;
    size_t                  PAudioBufferPos;
    
    // FFT
    float                   PFFT[512];      // Données FFT
    float                   PRawData[4096]; // Données audio brutes pour l'affichage de la forme d'onde
    
    // Informations sur le fichier audio
    unsigned __int64        PDuration;      // Durée en millisecondes
    float                   PVolume;
    bool                    PIsPlaying;
    bool                    PIsInitialized;
    
    // Méthodes privées pour la gestion des données audio
    HRESULT                 __fastcall InitializeDirectSound();
    HRESULT                 __fastcall CreateSoundBuffer();
    HRESULT                 __fastcall SetupNotifications();
    void                    __fastcall CleanupDirectSound();
    void                    __fastcall CleanupFFmpeg();
    void                    __fastcall CalculateFFT(const BYTE* pData, DWORD dataSize);
    bool                    __fastcall DecodeAudioPacket();
    
    // Thread de lecture
    static DWORD WINAPI     PlaybackThread(LPVOID lpParameter);
    void                    __fastcall ProcessAudio();
    
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
    XFFmpeg();
    ~XFFmpeg(void);

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