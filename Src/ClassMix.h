//---------------------------------------------------------------------------
#ifndef ClassMixH
#define ClassMixH

class XMix;

// Inclusions nécessaires pour les définitions complètes
#include "ClassCore.h"
#include "ClassPlayers.h"
#include "ClassSpectrum.h"
#include "ClassWave.h"

class XMix {
private:
    // Membres privés
    HWND             Handle;        // Handle de la fenêtre principale
	XCore           *Core;          // Référence au noyau de l'application
	XPlayers        *PLast;         // Lecteur pour le morceau précédent
    XPlayers        *PCurrent;      // Lecteur pour le morceau actuel
    XPlayers        *PNext;         // Lecteur pour le morceau suivant
    XPlayers        *PTmp;          // Lecteur temporaire pour les rotations
    XPlayers        *PTest;         // Lecteur pour tester les fichiers audio
	XSpectrum       *PSpectrum;     // Visualisation du spectre audio
    XWave           *PWave;         // Visualisation de la forme d'onde
    bool             PIsMixing;     // Indique si un mixage est en cours
    unsigned __int64 PMsMixLength;  // Durée du mixage en millisecondes
    int              PAdbPercent;   // Pourcentage ADB pour la détection automatique

    // Méthodes privées
    bool             __fastcall GetIsTimeToStartMix();
    unsigned __int64 __fastcall GetPosition();
    void             __fastcall Roll();
    void             __fastcall CrossFade();

    // Vérifie si le Core est valide
    bool             __fastcall IsValid() const { return Core != nullptr; }

    // Propriétés privées
    __property bool             IsTimeToStartMix = { read = GetIsTimeToStartMix };
    __property unsigned __int64 MsMixLength      = { read = PMsMixLength, write = PMsMixLength };

public:
    // Constructeur et destructeur
	XMix(HWND handle, XCore *core);
    ~XMix();

    // Méthodes publiques
    bool __fastcall Init();                     // Initialise le mixeur
    void __fastcall Stop();                     // Arrête la lecture
    void __fastcall Auto();                     // Gestion automatique du mixage
    void __fastcall Play();                     // Démarre ou reprend la lecture
    void __fastcall NextNow();                  // Passe immédiatement au morceau suivant
    void __fastcall NodeNow(XNode *node);       // Joue immédiatement un nœud spécifique

    // Propriétés publiques
    __property bool             IsMixing  = { read = PIsMixing };
    __property unsigned __int64 Position  = { read = GetPosition };
    __property XPlayers        *Last      = { read = PLast };
    __property XPlayers        *Current   = { read = PCurrent };
    __property XPlayers        *Next      = { read = PNext };
    __property XPlayers        *Test      = { read = PTest };
    __property XSpectrum       *Spectrum  = { read = PSpectrum };
    __property XWave           *Wave      = { read = PWave };
};
#endif
