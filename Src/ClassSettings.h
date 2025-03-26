//---------------------------------------------------------------------------
#ifndef ClassSettingsH
#define ClassSettingsH

class XSettings;

#include "ClassCore.h"
#include "ClassIntf.h"

//---------------------------------------------------------------------------
class XSettings {
private:
    // Membres privés
    bool          PCreated;
    XCore        *Core;
	XIntf        *Intf;
	UnicodeString PSettingPath;
	UnicodeString PBackupPath;
    UnicodeString PSettingFile;
    UnicodeString PAppPath;

    // Méthode pour initialiser les chemins
    void __fastcall InitializePaths();

    friend XIntf;

public:
    // Constructeur et destructeur
	XSettings(XCore *core);
    ~XSettings();

    // Méthodes de gestion des paramètres
    bool __fastcall Load();
    bool __fastcall Save();

    // Paramètres de l'interface
    int           FontLimitMin;    // Limite minimale pour la taille de police
    int           FontLimitMax;    // Limite maximale pour la taille de police
    int           AutoSaveToKeep;  // Nombre de sauvegardes automatiques à conserver
    int           AutoSelect;      // Nombre de chansons à sélectionner automatiquement

    // Paramètres d'arborescence
    int           ArboViewMode;    // Mode d'affichage de l'arborescence
    int           ArboAzMode;      // Mode d'affichage alphabétique

    // Paramètres de mixage
    int           AdbPercent;      // Pourcentage ADB
    unsigned __int64 FadeInDuration;  // Durée du fondu d'entrée (ms)
    unsigned __int64 FadeOutDuration; // Durée du fondu de sortie (ms)

    // Paramètres généraux
    int           Volume;          // Volume
    UnicodeString MusicPath;       // Chemin vers les fichiers musicaux
    UnicodeString Engine;          // Moteur audio utilisé

    // Paramètres de police
    int           FontSizeMin;     // Taille minimale de police
    int           FontSizeMax;     // Taille maximale de police
    bool          FontResize;      // Redimensionnement automatique de la police
    bool          PanelResize;     // Redimensionnement automatique des panneaux

    // Couleurs
    TColor        ColorList;       // Couleur de la liste
    TColor        ColorNext;       // Couleur de la liste suivante
    TColor        ColorPast;       // Couleur de la liste précédente
    TColor        ColorPlaylist;   // Couleur de la playlist

    // Propriétés
    __property bool          Created    = {read = PCreated};
    __property UnicodeString SettingPath = {read = PSettingPath};
    __property UnicodeString BackupPath  = {read = PBackupPath};
};
#endif
