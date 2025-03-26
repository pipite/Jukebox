//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassCore.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

#define LISTPATH L""

// ---------------------------------------------------------------------------
//   XCore
// ---------------------------------------------------------------------------
__fastcall XCore::XCore(XIntf *intf) {
    // Initialisation des pointeurs à NULL
    PCreated            = false;
    Intf                = intf;
    PWorkingLevel       = 0;
    Settings            = NULL;
    Loader              = NULL;
    PMix                = NULL;
    PSavFile            = NULL;
    TvList              = NULL;
    TvNext              = NULL;
    TvPast              = NULL;
    TvPlayList          = NULL;
	//Mtx                 = NULL;
    PFontSize           = 0;
    PVolume             = 0;

    try {
        // Création du mutex pour la synchronisation
       // Mtx                  = new TMutex();

        // Création des objets de base
        Settings             = new XSettings(this);
        PFontSize            = Settings->FontSizeMin;
        Loader               = new XLoader(this);

        // Création du mixeur audio
        PMix                 = new XMix(Intf->WinHandle, this);
        PMix->Spectrum->Image = Intf->ImgSpectrum;
        PMix->Wave->Image     = Intf->ImgWave;

        // Création du gestionnaire de fichiers
        PSavFile             = new XFile(this);

        // Création des vues d'arborescence
        TvList               = new XTvList(Intf->PanelList, this);
        TvNext               = new XTvNext(Intf->PanelNext, this);
        TvPast               = new XTvPast(Intf->PanelPast, this);
        TvPlayList           = new XTvPlayList(Intf->PanelPlayList, this);

        // Initialisation du générateur de nombres aléatoires
        srand(time(NULL));
        Randomize();

        // Marquer la création comme réussie
        PCreated             = true;

        // Chargement des Node Song
        if (Loader) {
            Loader->Load();
        }
    }
    catch (...) {
        // Nettoyage en cas d'exception
        if (TvPlayList) { delete TvPlayList; TvPlayList = NULL; }
        if (TvPast)     { delete TvPast;     TvPast     = NULL; }
        if (TvNext)     { delete TvNext;     TvNext     = NULL; }
        if (TvList)     { delete TvList;     TvList     = NULL; }
        if (PSavFile)   { delete PSavFile;   PSavFile   = NULL; }
        if (PMix)       { delete PMix;       PMix       = NULL; }
        if (Loader)     { delete Loader;     Loader     = NULL; }
        if (Settings)   { delete Settings;   Settings   = NULL; }
		//if (Mtx)        { delete Mtx;        Mtx        = NULL; }

        // Relancer l'exception pour informer l'appelant
        throw;
    }
}

__fastcall XCore::~XCore(void) {
    try {
        // Arrêter le mixeur audio avant de libérer les ressources
        if (PMix) {
            PMix->Stop();
        }

        // Libération des ressources dans l'ordre inverse de leur création
        if (TvPlayList) { delete TvPlayList; TvPlayList = NULL; }
        if (TvPast)     { delete TvPast;     TvPast     = NULL; }
        if (TvNext)     { delete TvNext;     TvNext     = NULL; }
        if (TvList)     { delete TvList;     TvList     = NULL; }
        if (PSavFile)   { delete PSavFile;   PSavFile   = NULL; }
        if (PMix)       { delete PMix;       PMix       = NULL; }
        if (Loader)     { delete Loader;     Loader     = NULL; }
        if (Settings)   { delete Settings;   Settings   = NULL; }
        //if (Mtx)        { delete Mtx;        Mtx        = NULL; }

        // Réinitialisation des autres pointeurs
        Intf = NULL;
    }
    catch (...) {
        // Ignorer les exceptions pendant la libération des ressources
        // pour éviter les problèmes lors de la fermeture de l'application
    }
}

void __fastcall XCore::LOG(UnicodeString module, const UnicodeString function, const UnicodeString e) {
//	std::ofstream logFile("Jukebox-error.log", std::ios::app);
//	if (logFile) {
//		logFile << "[" << module << "] " << function << " - Exception: " << e << std::endl;
//	}
}



void __fastcall XCore::SetPanelListMode(UnicodeString visual) {
    if (!PCreated) return;
    if (!TvList || !Intf) return;

    try {
        if (visual == "DropFolder") {
            // Mode dossier de dépôt
            TvList->Visible = false;
            TvList->Align = alNone;
            Intf->LoadingAnimation(false);
            Intf->DropFolder(true);
        } else if (visual == "List") {
            // Mode liste
            Intf->DropFolder(false);
            Intf->LoadingAnimation(false);
            TvList->Visible = true;
            TvList->Align = alClient;
        } else { // Loading
            // Mode chargement
            TvList->Visible = false;
            TvList->Align = alNone;
            Intf->DropFolder(false);
            Intf->LoadingAnimation(true);
        }

        // Sauvegarder le mode
        PPanelListMode = visual;
    }
    catch (...) {
        // Ignorer les exceptions
    }
}

void __fastcall XCore::SetWorking(bool work) {
    if (!PCreated) return;
    if (!Intf) return;

    try {
        // Incrémenter ou décrémenter le niveau de travail
        if (work) {
            PWorkingLevel++;
        } else {
            PWorkingLevel--;
            // Éviter les valeurs négatives
            if (PWorkingLevel < 0) {
                PWorkingLevel = 0;
            }
        }

        // Déterminer si l'animation de travail doit être affichée
        bool b = (PWorkingLevel > 0);

        // Mettre à jour l'animation
        Intf->WorkingAnimation(b);
    }
    catch (...) {
        // Ignorer les exceptions
    }
}

void __fastcall XCore::SetFontSize(int size) {
    if (!PCreated) return;
    if (!Settings) return;

    try {
        // Vérifier si le redimensionnement de la police est activé
        if (!Settings->FontResize) {
            size = Settings->FontSizeMin;
        }

        // Vérifier si la taille a changé
        if (PFontSize != size) {
            // Sauvegarder la nouvelle taille
            PFontSize = size;

            // Mettre à jour la taille de la police dans les vues d'arborescence
            if (TvList) {
                TvList->Font->Size = size;
            }
            if (TvNext) {
                TvNext->Font->Size = size;
            }
            if (TvPast) {
                TvPast->Font->Size = size;
            }
            if (TvPlayList) {
                TvPlayList->Font->Size = size;
            }
        }
    }
    catch (...) {
        // Ignorer les exceptions
    }
}

void __fastcall XCore::RefreshTv(void) {
    if (!PCreated) return;

    try {
        // Rafraîchir les vues d'arborescence
        if (TvList) {
            TvList->Refresh();
        }
        if (TvPast) {
            TvPast->Refresh();
        }
        if (TvPlayList) {
            TvPlayList->Refresh();
        }
    }
    catch (...) {
        // Ignorer les exceptions
    }
}

bool __fastcall XCore::GetIsValidFolderPath(void) {
	return DirectoryExists( Settings->MusicPath );
}

void __fastcall XCore::ModifyFolderPath(UnicodeString folderpath) {
    // Vérifications préliminaires
    if (!PCreated) return;
    if (!DirectoryExists(folderpath)) return;

    try {
        // Désactiver les timers et arrêter la lecture
        if (Intf) {
            Intf->Timers = false;
        }
        if (PMix) {
            PMix->Stop();
        }

        // Réinitialiser les vues d'arborescence
        if (TvNext) {
            TvNext->Reset();
        }
        if (TvPast) {
            TvPast->Reset();
        }
        if (TvPlayList) {
            TvPlayList->Reset();
        }
        if (TvList) {
            TvList->Reset();
        }

        // Mettre à jour le chemin de la musique
        if (Settings) {
            Settings->MusicPath = folderpath;
        }

        // Lancer le thread de scan du disque
        try {
            ThreadScanDisk *thread = new ThreadScanDisk(this, false);
            // Le thread se libérera automatiquement grâce à FreeOnTerminate = true
        }
        catch (...) {
            // Ignorer les exceptions lors de la création du thread
            // et réactiver les timers
            if (Intf) {
                Intf->Timers = true;
            }
        }
    }
    catch (...) {
        // Ignorer les exceptions et réactiver les timers
        if (Intf) {
            Intf->Timers = true;
        }
    }
}

void __fastcall XCore::LoadPlayList(UnicodeString path) {
    // Vérifications préliminaires
    if (!PCreated) return;
    if (!FileExists(path)) return;
    if (!PSavFile || !Settings) return;

    try {
        // Archiver la playlist actuelle
        PSavFile->ArchivePlayList();

        // Supprimer l'ancienne playlist
        UnicodeString playlistPath = Settings->SettingPath + "\\PlayList.sav";
        if (FileExists(playlistPath)) {
            DeleteFile(playlistPath);
        }

        // Copier la nouvelle playlist
        TFile::Copy(path, playlistPath);

        // Lancer le thread de scan de la playlist
        try {
            ThreadScanPlaylist *thread = new ThreadScanPlaylist(this, false);
            // Le thread se libérera automatiquement grâce à FreeOnTerminate = true
        }
        catch (...) {
            // Ignorer les exceptions lors de la création du thread
        }
    }
    catch (...) {
        // Ignorer les exceptions
    }
}

void __fastcall XCore::SetVolume(int value) {
    try {
        // Limiter la valeur entre 0 et 65535
        if (value < 0) value = 0;
        if (value > 65535) value = 65535;

        // Calculer le volume inversé (0 = volume maximum, 65535 = volume minimum)
        unsigned v = 65535 - (unsigned)value;

        // Calculer le volume pour les deux canaux (gauche et droit)
        unsigned Vol = ((v & 0x0000ffff) | (v << 16));

        // Définir le volume
        waveOutSetVolume(NULL, Vol);

        // Sauvegarder la valeur
        PVolume = value;
    }
    catch (...) {
        // Ignorer les exceptions
    }
}

