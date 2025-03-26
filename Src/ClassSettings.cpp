//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassSettings.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   XSettings
// ---------------------------------------------------------------------------
__fastcall XSettings::XSettings(XCore *core) {
    // Initialisation des références
    Core = core;
    PCreated = false;

    // Vérification de la présence du fichier bass.dll
    if (!FileExists("bass.dll")) {
        MessageDlg("File bass.dll not found.", mtInformation, TMsgDlgButtons() << mbOK, 1);
        return;
    }

    // Initialisation des chemins
    InitializePaths();

    // Initialisation des paramètres par défaut
    // Limites et sauvegardes
    AutoSaveToKeep = 7;
    FontLimitMin = 6;
    FontLimitMax = 40;

    // Paramètres d'interface
    AutoSelect = 3;

    // Paramètres d'arborescence
    ArboViewMode = 0;
    ArboAzMode = 0;

    // Paramètres de mixage
    FadeInDuration = 1000;
    FadeOutDuration = 2000;
    AdbPercent = 10;

    // Paramètres généraux
    MusicPath = "";
    Volume = 6489;
    FontSizeMin = 8;
    FontSizeMax = 40;
    FontResize = true;
    PanelResize = true;
    Engine = "Bass";

    // Couleurs
    ColorList = clSkyBlue;
    ColorNext = clLime;
    ColorPast = clGray;
    ColorPlaylist = clGray;

    // Chargement des paramètres depuis le fichier
    if (Core && Core->Intf) {
        Intf = Core->Intf;
        Load();
    }
}

__fastcall XSettings::~XSettings() {
    // Rien à libérer
}

void __fastcall XSettings::InitializePaths() {
    // Initialisation des chemins de configuration
    PSettingPath = ExtractFilePath(Application->ExeName) + L"\\Config";
    PBackupPath = PSettingPath + L"\\Backup";
    PSettingFile = L"Config.ini";

    // Création des répertoires si nécessaire
    if (!DirectoryExists(PSettingPath)) CreateDir(PSettingPath);
    if (!DirectoryExists(PBackupPath)) CreateDir(PBackupPath);

    // Initialisation du chemin de l'application
    PAppPath = ExtractFilePath(Application->ExeName);

    // Ajustement du chemin de l'application selon l'environnement
    int pos;
    if ((pos = PAppPath.Pos("\\Source\\.\\Win32\\Debug\\")) > 0)
        PAppPath = PAppPath.SubString(1, pos-1);
    else if ((pos = PAppPath.Pos("\\Source\\Win32\\Debug\\")) > 0)
        PAppPath = PAppPath.SubString(1, pos);
    else if ((pos = PAppPath.Pos("\\Bin\\")) > 0)
        PAppPath = PAppPath.SubString(1, pos);
}

bool __fastcall XSettings::Save() {
    // Vérification des prérequis
    if (!Core || !Intf) return false;

    // Création du répertoire de configuration si nécessaire
    if (!DirectoryExists(PSettingPath)) CreateDir(PSettingPath);

    // Ouverture du fichier INI
    TIniFile* iniFile = nullptr;
    bool result = true;

    try {
        iniFile = new TIniFile(PSettingPath + L"\\" + PSettingFile);

        // Sauvegarde des paramètres de fenêtre
        iniFile->WriteInteger("Win", "Top", Intf->WinTop);
        iniFile->WriteInteger("Win", "Left", Intf->WinLeft);
        iniFile->WriteInteger("Win", "Width", Intf->WinWidth);
        iniFile->WriteInteger("Win", "Height", Intf->WinHeight);

        // Sauvegarde des paramètres de fenêtre minimisée
        iniFile->WriteInteger("Min", "Top", Intf->MinTop);
        iniFile->WriteInteger("Min", "Left", Intf->MinLeft);

        // Sauvegarde des paramètres de liste
        iniFile->WriteInteger("List", "Width", Intf->ListWidth);

        // Sauvegarde des paramètres de liste suivante
        iniFile->WriteInteger("Next", "Height", Intf->NextHeight);
        iniFile->WriteInteger("Next", "AutoSelect", AutoSelect);

        // Sauvegarde des paramètres de liste précédente
        iniFile->WriteInteger("Past", "Height", Intf->PastHeight);

        // Sauvegarde des paramètres d'arborescence
        iniFile->WriteInteger("Arbo", "ViewMode", Core->TvList->ViewFlat);
        iniFile->WriteInteger("Arbo", "AzMode", Core->TvList->ViewAz);

        // Sauvegarde des paramètres de mixage
        iniFile->WriteInteger("Mix", "AdbPercent", AdbPercent);
        iniFile->WriteInteger("Mix", "FadeInDuration", (int)FadeInDuration);
        iniFile->WriteInteger("Mix", "FadeOutDuration", (int)FadeOutDuration);

        // Sauvegarde des paramètres généraux
        iniFile->WriteString("Settings", "Folder", MusicPath);
        iniFile->WriteInteger("Settings", "Volume", Intf->SbVolume);
        iniFile->WriteBool("Settings", "PanelResize", PanelResize);
        iniFile->WriteString("Settings", "Engine", Engine);

        // Sauvegarde des paramètres de police
        iniFile->WriteInteger("Fonts", "FontSizeMin", FontSizeMin);
        iniFile->WriteInteger("Fonts", "FontSizeMax", FontSizeMax);
        iniFile->WriteBool("Fonts", "FontResize", FontResize);
    }
    catch (...) {
        result = false;
    }

    // Libération des ressources
    if (iniFile) {
        delete iniFile;
    }

    return result;
}

bool __fastcall XSettings::Load() {
    // Vérification des prérequis
    if (!Intf) return false;

    // Création du répertoire de configuration si nécessaire
    if (!DirectoryExists(PSettingPath)) CreateDir(PSettingPath);

    // Ouverture du fichier INI
    TIniFile* iniFile = nullptr;

    try {
        iniFile = new TIniFile(PSettingPath + L"\\" + PSettingFile);

        // Chargement des paramètres de fenêtre
        Intf->WinTop = iniFile->ReadInteger("Win", "Top", 150);
        Intf->WinLeft = iniFile->ReadInteger("Win", "Left", 150);
        Intf->WinWidth = iniFile->ReadInteger("Win", "Width", 600);
        Intf->WinHeight = iniFile->ReadInteger("Win", "Height", 600);

        // Chargement des paramètres de fenêtre minimisée
        Intf->MinTop = iniFile->ReadInteger("Min", "Top", 100);
        Intf->MinLeft = iniFile->ReadInteger("Min", "Left", 100);

        // Chargement des paramètres de liste
        Intf->ListWidth = iniFile->ReadInteger("List", "Width", 420);

        // Chargement des paramètres de liste suivante
        Intf->NextHeight = iniFile->ReadInteger("Next", "Height", 70);
        AutoSelect = iniFile->ReadInteger("Next", "AutoSelect", AutoSelect);

        // Chargement des paramètres de liste précédente
        Intf->PastHeight = iniFile->ReadInteger("Past", "Height", 50);

        // Chargement des paramètres d'arborescence
        ArboViewMode = iniFile->ReadInteger("Arbo", "ViewMode", ArboViewMode);
        ArboAzMode = iniFile->ReadInteger("Arbo", "AzMode", ArboAzMode);

        // Chargement des paramètres de mixage
        AdbPercent = iniFile->ReadInteger("Mix", "AdbPercent", AdbPercent);
        FadeInDuration = iniFile->ReadInteger("Mix", "FadeInDuration", FadeInDuration);
        FadeOutDuration = iniFile->ReadInteger("Mix", "FadeOutDuration", FadeOutDuration);

        // Chargement des paramètres généraux
        MusicPath = iniFile->ReadString("Settings", "Folder", MusicPath);
        Volume = iniFile->ReadInteger("Settings", "Volume", Volume);
        PanelResize = iniFile->ReadBool("Settings", "PanelResize", PanelResize);
        Engine = iniFile->ReadString("Settings", "Engine", Engine);

        // Chargement des paramètres de police
        FontSizeMin = iniFile->ReadInteger("Fonts", "FontSizeMin", FontSizeMin);
        FontSizeMax = iniFile->ReadInteger("Fonts", "FontSizeMax", FontSizeMax);
        FontResize = iniFile->ReadBool("Fonts", "FontResize", FontResize);

        // Marquer comme créé
        PCreated = true;
    }
    catch (...) {
        // Ignorer les erreurs
    }

    // Libération des ressources
    if (iniFile) {
        delete iniFile;
    }

    return PCreated;
}


