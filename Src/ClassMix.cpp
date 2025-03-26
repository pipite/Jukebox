//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassMix.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
 * @brief Constructeur de la classe XMix
 * @param handle Handle de la fenêtre principale
 * @param core Pointeur vers le noyau de l'application
 */
__fastcall XMix::XMix(HWND handle, XCore *core)
{
    // Initialisation des membres
    Handle = handle;
    Core = core;

    // Initialisation des paramètres
    PAdbPercent = core && core->Settings ? core->Settings->AdbPercent : 10;
    PMsMixLength = 5000; // 5 secondes par défaut
    PIsMixing = false;

    // Initialisation des pointeurs à NULL
    PLast = nullptr;
    PCurrent = nullptr;
    PNext = nullptr;
    PTest = nullptr;
    PSpectrum = nullptr;
    PWave = nullptr;
    PTmp = nullptr;

    try {
        // Création des lecteurs
        PLast = new XPlayers(Core, Handle);
        PCurrent = new XPlayers(Core, Handle);
        PNext = new XPlayers(Core, Handle);
        PTest = new XPlayers(Core, Handle);

        // Création des visualisations
        PSpectrum = new XSpectrum(this);
        PWave = new XWave(Core);
    }
    catch (...) {
        // Libération des ressources en cas d'exception
        if (PLast)     { delete PLast;     PLast     = nullptr; }
        if (PCurrent)  { delete PCurrent;  PCurrent  = nullptr; }
        if (PNext)     { delete PNext;     PNext     = nullptr; }
        if (PTest)     { delete PTest;     PTest     = nullptr; }
        if (PSpectrum) { delete PSpectrum; PSpectrum = nullptr; }
        if (PWave)     { delete PWave;     PWave     = nullptr; }
        throw; // Relancer l'exception
    }
}

/**
 * @brief Destructeur de la classe XMix
 */
__fastcall XMix::~XMix()
{
    // Libération des ressources dans l'ordre inverse de leur création
	if (PTest)     { delete PTest;     PTest     = nullptr; }
	if (PNext)     { delete PNext;     PNext     = nullptr; }
	if (PCurrent)  { delete PCurrent;  PCurrent  = nullptr; }
	if (PLast)     { delete PLast;     PLast     = nullptr; }
	if (PSpectrum) { delete PSpectrum; PSpectrum = nullptr; }
	if (PWave)     { delete PWave;     PWave     = nullptr; }
}

/**
 * @brief Initialise le mixeur avec les morceaux disponibles
 * @return true si l'initialisation a réussi, false sinon
 */
bool __fastcall XMix::Init() {
    // Vérification des prérequis
    if (!IsValid() || !Core->TvList || !Core->TvList->NRSong) return false;
    if (!PCurrent || !PNext) return false;

    // Vérification si des morceaux sont disponibles
    if (!Core->TvList->NRSong->HaveSong) return false;

    try {
        // Initialisation des listes
        Core->TvList->SetNodeRoot();
        Core->TvPast->Reset();
        Core->TvNext->Fill();

        // Récupération du premier morceau
        XNode *cn = Core->TvNext->Pop();
        if (!cn) return false;

        // Configuration des lecteurs
        PCurrent->Node = cn;

        // Vérification que Core->TvNext->Next existe avant de l'assigner
        if (Core->TvNext && Core->TvNext->Next) {
            PNext->Node = Core->TvNext->Next;
        }

        // Démarrage de la premiere lecture
        PCurrent->Play();
        PIsMixing = true;

        return true;
    }
    catch (...) {
        // Capture des exceptions inconnues
        PIsMixing = false;
        return false;
    }
}

/**
 * @brief Fait tourner les lecteurs (précédent <- actuel <- suivant <- temporaire)
 */
void __fastcall XMix::Roll() {
    if (!IsValid() || !PLast || !PCurrent || !PNext) return;

    // Rotation des lecteurs
    PTmp = PLast;
    PLast = PCurrent;
    PCurrent = PNext;
    PNext = PTmp;
    PTmp = nullptr; // Éviter les références pendantes

    // Configuration du lecteur suivant
    if (Core->TvNext && Core->TvNext->Next && PNext) {
        PNext->Node = Core->TvNext->Next;
    }

    // Indication que la forme d'onde doit être rafraîchie
    if (PWave) {
        PWave->NeedRefresh = true;
    }
}

/**
 * @brief Gestion automatique du mixage et des transitions
 */
void __fastcall XMix::Auto() {
    if (!IsValid() || !Core->TvList || !Core->TvList->NRSong) return;

    // Vérification si des morceaux sont disponibles
    if (Core->TvList->NRSong->HaveSong) {
        // Gestion des transitions automatiques
        if (!PIsMixing && PCurrent && PNext && Core->Settings) {
            // Configuration des fondus
            PCurrent->FadeOutAuto(Core->Settings->FadeOutDuration);
            PNext->FadeInAuto(Core->Settings->FadeInDuration);

            // Vérification si c'est le moment de démarrer le mixage
            if (GetIsTimeToStartMix()) {
                // Démarrage de la transition
                PCurrent->Play();
                if (PLast) PLast->Stop();

                // Mise à jour des listes
                if (Core->TvNext) {
                    Core->TvNext->Pop();
                    Roll();

                    // Vérification que PNext et Core->TvNext->Next existent
                    if (PNext && Core->TvNext->Next) {
                        PNext->Node = Core->TvNext->Next;
                    }

                    Core->TvNext->Fill();
                }

                PIsMixing = true;
            }
        }

        // Mise à jour des visualisations
        if (PSpectrum) PSpectrum->Draw();
        if (PWave && PCurrent && PCurrent->Node) PWave->Refresh(PCurrent->Node);
    }

    // Gestion du fondu enchaîné si un mixage est en cours
    if (PIsMixing) CrossFade();
}

/**
 * @brief Passe immédiatement au morceau suivant
 */
void __fastcall XMix::NextNow()
{
    if (!IsValid() || !Core->TvNext) return;

    // Récupération du morceau suivant et lecture immédiate
    XNode *nextNode = Core->TvNext->Pop();
    if (nextNode) {
        NodeNow(nextNode);
    }
}

/**
 * @brief Joue immédiatement un nœud spécifique
 * @param node Nœud à jouer
 */
void __fastcall XMix::NodeNow(XNode *node)
{
    // Vérification des prérequis
    if (!IsValid() || !PCurrent || !PNext || !node) return;
    if (!PCurrent->IsPlaying) return;
    if (!Core->TvList || !Core->TvList->NRSong || !Core->TvList->NRSong->HaveSong) return;

    try {
        // Arrêt du morceau précédent
        if (PLast) PLast->Stop();

        // Configuration des fondus
        if (Core->Settings) {
            PCurrent->FadeOutNow(Core->Settings->FadeOutDuration);
            PNext->FadeInNow(Core->Settings->FadeInDuration);
        }

        // Configuration du lecteur suivant
        PNext->Node = node;

        // Rotation des lecteurs
        Roll();

        // Mise à jour de la liste suivante
        if (Core->TvNext && PNext) {
            if (Core->TvNext->Next) {
                PNext->Node = Core->TvNext->Next;
            }
            Core->TvNext->Fill();
        }

        // Démarrage de la lecture
        if (PCurrent) {
            PCurrent->Play();
            PIsMixing = true;
        }
    }
    catch (...) {
        // Capture des exceptions inconnues
        PIsMixing = false;
    }
}

/**
 * @brief Gère le fondu enchaîné entre les morceaux
 */
void __fastcall XMix::CrossFade() {
    if (!IsValid() || !PLast || !PCurrent) return;

    // Exécution des fondus
    bool fadeOutComplete = PLast->FadeOut();
    bool fadeInComplete = PCurrent->FadeIn();

    // Mise à jour de l'état du mixage
    PIsMixing = (fadeOutComplete || fadeInComplete);
}

/**
 * @brief Vérifie s'il est temps de démarrer le mixage
 * @return true s'il est temps de démarrer le mixage, false sinon
 */
bool __fastcall XMix::GetIsTimeToStartMix() {
    // Vérification des prérequis
    if (!IsValid() || !PCurrent || !PCurrent->IsValid) return false;
    if (!PNext || !PCurrent->Node || !PNext->Node) return false;

    // Vérification de la position par rapport aux points de fondu
    if (PCurrent->Position > (PCurrent->Node->MsFadeOut - PNext->Node->MsFadeIn)) {
        return true;
    }

    return false;
}

/**
 * @brief Démarre ou reprend la lecture
 */
void __fastcall XMix::Play() {
    if (!IsValid() || !PCurrent) return;

    // Démarrage ou reprise de la lecture
    if (!PCurrent->IsPlaying) {
        PCurrent->Play();
    } else if (PLast) {
        // Si déjà en lecture, arrêt du morceau précédent
        PLast->Stop();
    }
}

/**
 * @brief Arrête la lecture
 */
void __fastcall XMix::Stop() {
    // Arrêt de tous les lecteurs
    if (PLast) PLast->Stop();
    if (PCurrent) PCurrent->Stop();

    // Réinitialisation de l'état du mixage
    PIsMixing = false;
}

/**
 * @brief Obtient la position actuelle de lecture
 * @return Position en millisecondes
 */
unsigned __int64 __fastcall XMix::GetPosition() {
    // Vérification des prérequis
    if (!PCurrent) return 0;

    // Récupération de la durée totale
    unsigned __int64 length = PCurrent->MsLength;
    if (length == 0) return 0;

    // Récupération de la position actuelle
    return PCurrent->Position;
}