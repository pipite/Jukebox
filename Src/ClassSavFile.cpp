//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassSavFile.h"

//---------------------------------------------------------------------------

/**
 * @brief Constructeur de la classe XFile
 * @param core Pointeur vers le noyau de l'application
 */
__fastcall XFile::XFile(XCore *core) {
    Core = core;
    PFile = new TStringList();

    if (IsValid() && Core->Loader) {
        Core->Loader->Loaded = false;
    }
}

/**
 * @brief Destructeur de la classe XFile
 */
__fastcall XFile::~XFile() {
    if (PFile) {
        delete PFile;
        PFile = nullptr;
    }
}

/**
 * @brief Déplace un fichier vers le dossier d'archive avec une extension de date
 * @param src Chemin du fichier source
 * @return true si l'opération a réussi, false sinon
 */
bool __fastcall XFile::MoveToArchive(const UnicodeString& src) {
    if (!IsValid() || !Core->Settings) return false;

    try {
        // Création du chemin de destination
        UnicodeString dest = Core->Settings->BackupPath + "\\" + ExtractFileName(src) + ExtendDate;

        // Copie du fichier vers l'archive
        TFile::Copy(src, dest);

        // Suppression du fichier source
        TFile::Delete(src);

        return true;
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
        return false;
    }
    catch (...) {
        // Capture des exceptions inconnues
        return false;
    }
}

/**
 * @brief Convertit une playlist du format V2.2 au format V2.3
 * @return true si la conversion a réussi ou si la playlist est déjà au format V2.3, false sinon
 */
bool __fastcall XFile::ConvertPlaylist() {
    if (!IsValid() || !Core->Settings) return false;
    if (!HavePlaylist) return false;

    try {
        // Lecture de la première ligne pour déterminer le format
        UnicodeString firstLine;
        {
            // Utilisation d'un bloc pour limiter la portée du flux
            std::ifstream file(UnicodeString(PathPlaylist).c_str());
            if (!file.is_open()) return false;

            // Lecture de la première ligne
            char buffer[1000] = {0};
            file.getline(buffer, sizeof(buffer) - 1);
            firstLine = UnicodeString(buffer);
        }

        // Vérification du format
        if (firstLine.Pos(L"Playlist V2.3")) {
            // Déjà au format V2.3
            return true;
        }

        if (firstLine.Pos(L"0,0,0,")) {
            // Format V2.2, conversion nécessaire
            UnicodeString musicPath = Core->Settings->MusicPath;

            // Chargement du fichier
            PFile->LoadFromFile(PathPlaylist);

            // Conversion des chemins absolus en chemins relatifs
            RemoveRootPath(false);

            // Mise à jour de la première ligne
            PFile->Strings[0] = "0,0,0," + musicPath;

            // Archivage de l'ancien fichier
            MoveToArchive(PathPlaylist);

            // Ajout du marqueur V2.3
            PFile->Insert(0, "Playlist V2.3");

            // Sauvegarde du fichier converti
            PFile->SaveToFile(PathPlaylist);

            return true;
        }
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
        return false;
    }
    catch (...) {
        // Capture des exceptions inconnues
        return false;
    }

    return false;
}

/**
 * @brief Convertit les chemins absolus en chemins relatifs dans le fichier
 * @param bypass Si true, convertit tous les chemins, sinon uniquement ceux de type 2
 */
void __fastcall XFile::RemoveRootPath(bool bypass) {
    if (!IsValid() || !Core->Settings || !PFile) return;

    try {
        // Parcours de toutes les lignes du fichier
        for (int i = 0; i < PFile->Count; i++) {
            UnicodeString line = PFile->Strings[i];

            // Extraction des parties de la ligne
            // Format: niveau,compteur,type,chemin
            int pos1 = line.Pos(",");
            if (pos1 <= 0) continue;

            UnicodeString part1 = line.SubString(1, pos1);
            UnicodeString rest1 = line.SubString(pos1 + 1, line.Length());

            int pos2 = rest1.Pos(",");
            if (pos2 <= 0) continue;

            UnicodeString part2 = rest1.SubString(1, pos2);
            UnicodeString rest2 = rest1.SubString(pos2 + 1, rest1.Length());

            int pos3 = rest2.Pos(",");
            if (pos3 <= 0) continue;

            UnicodeString part3 = rest2.SubString(1, pos3);
            UnicodeString path = rest2.SubString(pos3 + 1, rest2.Length());

            // Vérification si le nœud est de type 2 ou si bypass est activé
            if (part3 == "2" || bypass) {
                // Vérification si le chemin commence par le chemin de musique
                UnicodeString musicPath = Core->Settings->MusicPath;
                if (path.Pos(musicPath) == 1) {
                    // Conversion du chemin absolu en chemin relatif
                    UnicodeString relativePath = path.SubString(musicPath.Length() + 1, path.Length());

                    // Mise à jour de la ligne
                    PFile->Strings[i] = part1 + "," + part2 + "," + part3 + "," + relativePath;
                }
            }
        }
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
    }
    catch (...) {
        // Capture des exceptions inconnues
    }
}

/**
 * @brief Sauvegarde l'arborescence et la playlist si nécessaire
 * @return true si la sauvegarde a réussi, false sinon
 */
bool __fastcall XFile::Save() {
    if (!IsValid() || !Core->Loader) return false;
    if (!Core->Loader->Loaded) return false;

    try {
        // Sauvegarde de l'arborescence
        PFile->Clear();
        PLevel = 0;

        // Vérification si le nœud racine existe
        if (!Core->TvList || !Core->TvList->NRArbo) return false;

        // Sauvegarde récursive de l'arborescence
        SaveRecurseNode(Core->TvList->NRArbo);

        // Sauvegarde du fichier
        PFile->SaveToFile(PathArbo);

        // Mise à jour du flag de sauvegarde
        Core->Loader->SaveArboNeeded = false;

        // Sauvegarde de la playlist si nécessaire
        if (Core->Loader->SavePlayListNeeded) {
            ArchivePlayList();
            SavePlaylist(PathPlaylist);
        }

        return true;
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
        return false;
    }
    catch (...) {
        // Capture des exceptions inconnues
        return false;
    }
}

/**
 * @brief Sauvegarde une playlist dans un fichier
 * @param filename Nom du fichier de destination
 * @return true si la sauvegarde a réussi, false sinon
 */
bool __fastcall XFile::SavePlaylist(const UnicodeString& filename) {
    if (!IsValid() || !Core->Loader || !Core->TvPlayList) return false;
    if (!Core->Loader->Loaded) return false;

    try {
        // Préparation du fichier
        PFile->Clear();
        PLevel = 0;

        // Ajout du marqueur V2.3
        PFile->Add(L"Playlist V2.3");

        // Vérification si le nœud racine existe
        if (!Core->TvPlayList->NR) return false;

        // Sauvegarde récursive de la playlist
        SaveRecurseNode(Core->TvPlayList->NR);

        // Conversion des chemins absolus en chemins relatifs
        RemoveRootPath(false);

        // Sauvegarde du fichier
        PFile->SaveToFile(filename);

        // Mise à jour du flag de sauvegarde
        Core->Loader->SavePlayListNeeded = false;

        return true;
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
        return false;
    }
    catch (...) {
        // Capture des exceptions inconnues
        return false;
    }
}

/**
 * @brief Archive le fichier d'arborescence
 * @return true si l'archivage a réussi, false sinon
 */
bool __fastcall XFile::ArchiveArbo() {
    if (!IsValid()) return false;
    if (!HaveArbo) return false;

    try {
        // Réinitialisation
        PLevel = 0;
        PFile->Clear();

        // Archivage du fichier
        if (!MoveToArchive(PathArbo)) return false;

        // Purge des anciennes archives
        return Purge("Arbo");
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
        return false;
    }
    catch (...) {
        // Capture des exceptions inconnues
        return false;
    }
}

/**
 * @brief Archive le fichier de playlist
 * @return true si l'archivage a réussi, false sinon
 */
bool __fastcall XFile::ArchivePlayList() {
    if (!IsValid()) return false;
    if (!HavePlaylist) return false;

    try {
        // Réinitialisation
        PLevel = 0;
        PFile->Clear();

        // Archivage du fichier
        if (!MoveToArchive(PathPlaylist)) return false;

        // Purge des anciennes archives
        return Purge("PlayList");
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
        return false;
    }
    catch (...) {
        // Capture des exceptions inconnues
        return false;
    }
}

void __fastcall XFile::SaveRecurseNode(XNode *node) {
    if (!node || !PFile) return;

    try {
        // Préparation du chemin à sauvegarder
        UnicodeString pathToSave = node->Path;
        
        // Si c'est le nœud racine, sauvegarder le chemin complet
        if (node->NodeType == NODETYPE::ROOT) {
            // Rien à faire, sauvegarder le chemin complet
        }
        // Si c'est un dossier ou une chanson, enlever MusicPath du chemin
        else if ((node->NodeType == NODETYPE::FOLDER || node->NodeType == NODETYPE::SONG) && IsValid() && Core->Settings) {
            UnicodeString musicPath = Core->Settings->MusicPath;
            if (pathToSave.Pos(musicPath) == 1) {
                // Enlever MusicPath du chemin
                pathToSave = pathToSave.SubString(musicPath.Length() + 2, pathToSave.Length());
            }
        }
        
        // Ajout de la ligne pour ce nœud
        // Format: niveau,compteur_lecture,type_nœud,chemin
        PFile->Add(UnicodeString(PLevel) + L"," +
                  UnicodeString(node->CountPlay) + L"," +
                  UnicodeString(node->NodeType) + L"," +
                  pathToSave);

        // Sauvegarde récursive des enfants
        for (unsigned i = 0; i < node->CountChild; i++) {
            PLevel++;
            SaveRecurseNode(node->ChildAt(i));
            PLevel--;
        }
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
    }
    catch (...) {
        // Capture des exceptions inconnues
    }
}

/**
 * @brief Sauvegarde récursivement un nœud et ses enfants
 * @param node Nœud à sauvegarder
 
void __fastcall XFile::SaveRecurseNode(XNode *node) {
    if (!node || !PFile) return;

    try {
        // Préparation du chemin à sauvegarder
        UnicodeString pathToSave = node->Path;
        
        // Si c'est une chanson, enlever MusicPath du chemin
        if (node->NodeType == NODETYPE::SONG && IsValid() && Core->Settings) {
            UnicodeString musicPath = Core->Settings->MusicPath;
            if (pathToSave.Pos(musicPath) == 1) {
                // Enlever MusicPath du chemin
                // pathToSave = pathToSave.SubString(musicPath.Length() + 2, pathToSave.Length());
            }
        }
        
        // Ajout de la ligne pour ce nœud
        // Format: niveau,compteur_lecture,type_nœud,chemin
        PFile->Add(UnicodeString(PLevel) + L"," +
                  UnicodeString(node->CountPlay) + L"," +
                  UnicodeString(node->NodeType) + L"," +
                  pathToSave);

        // Sauvegarde récursive des enfants
        for (unsigned i = 0; i < node->CountChild; i++) {
            PLevel++;
            SaveRecurseNode(node->ChildAt(i));
            PLevel--;
        }
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
    }
    catch (...) {
        // Capture des exceptions inconnues
    }
}
*/
/**
 * @brief Obtient le chemin du fichier d'arborescence
 * @return Chemin du fichier d'arborescence
 */
UnicodeString __fastcall XFile::GetPathArbo() {
    if (!IsValid() || !Core->Settings) return "";
    return Core->Settings->SettingPath + "\\Arbo.sav";
}

/**
 * @brief Obtient le chemin du fichier de playlist
 * @return Chemin du fichier de playlist
 */
UnicodeString __fastcall XFile::GetPathPlaylist() {
    if (!IsValid() || !Core->Settings) return "";
    return Core->Settings->SettingPath + "\\PlayList.sav";
}

/**
 * @brief Vérifie si le fichier d'arborescence existe
 * @return true si le fichier existe, false sinon
 */
bool __fastcall XFile::GetHaveArbo() {
    return FileExists(PathArbo);
}

/**
 * @brief Vérifie si le fichier de playlist existe
 * @return true si le fichier existe, false sinon
 */
bool __fastcall XFile::GetHavePlaylist() {
    return FileExists(PathPlaylist);
}

/**
 * @brief Obtient l'extension de date pour les fichiers d'archive
 * @return Extension de date au format .yyyy_mm_dd__hh_nn_ss
 */
UnicodeString __fastcall XFile::GetExtendDate() {
    return FormatDateTime(L".yyyy_mm_dd__hh_nn_ss", Now());
}

/**
 * @brief Purge les anciennes archives d'un type de fichier
 * @param filename Nom de base du fichier (sans extension)
 * @return true si la purge a réussi, false sinon
 */
bool __fastcall XFile::Purge(const UnicodeString& filename) {
    if (!IsValid() || !Core->Settings) return false;

    try {
        // Vérification si le dossier d'archive existe
        UnicodeString backupPath = Core->Settings->BackupPath;
        if (!DirectoryExists(backupPath)) return false;

        // Nombre de fichiers à conserver
        int filesToKeep = Core->Settings->AutoSaveToKeep;
        if (filesToKeep < 1) filesToKeep = 1;

        // Structure pour stocker les informations sur les fichiers
        struct FileInfo {
            UnicodeString path;
            TDateTime timestamp;
        };

        // Collecte des informations sur les fichiers
        std::vector<FileInfo> files;

        // Recherche des fichiers correspondants
        TSearchRec searchRec;
        UnicodeString searchPattern = backupPath + L"\\" + filename + ".sav.*";

        if (FindFirst(searchPattern, faAnyFile, searchRec) == 0) {
            do {
                // Ignorer les fichiers cachés
                if (searchRec.Name[1] == '.') continue;

                // Ajout du fichier à la liste
                FileInfo info;
                info.path = backupPath + L"\\" + searchRec.Name;
                info.timestamp = searchRec.TimeStamp;
                files.push_back(info);

            } while (FindNext(searchRec) == 0);

            // Fermeture de la recherche
            FindClose(searchRec);
        }

        // Tri des fichiers par date (du plus ancien au plus récent)
        std::sort(files.begin(), files.end(),
            [](const FileInfo& a, const FileInfo& b) {
                return a.timestamp < b.timestamp;
            });

        // Suppression des fichiers les plus anciens si nécessaire
        int filesToDelete = files.size() - filesToKeep;
        for (int i = 0; i < filesToDelete; i++) {
            if (FileExists(files[i].path)) {
                DeleteFile(files[i].path);
            }
        }

        return true;
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
        return false;
    }
    catch (...) {
        // Capture des exceptions inconnues
        return false;
    }
}

/*
void __fastcall XFile::ConvertArbo(void) {
	char s1[1000];
	UnicodeString p1, p2;
	int i;

	// Analyse Arbo V22 ou V23
	// 0,0,0,chemin
	if ( HaveArbo ) {
		strcpy(s1, "");
		Pf.open( PathArbo.c_str() );
		Pf.getline(s1, 999, '\n');
		i = strlen(s1)-6;
		p1 = UnicodeString(s1).SubString(7,i);
		Pf.getline(s1, 999, '\n');
		Pf.close();
		p2 = UnicodeString(s1).SubString(7,i);
		if ( p1.LowerCase() == p2.LowerCase() ) {
			// Convertir en V23
			PFile->LoadFromFile(PathArbo);
			RemoveRootPath(true);
			PFile->Strings[0] = PFile->Strings[0] + p1;
			PFile->SaveToFile(PathArbo+".Convert");
		}
	}
}

*/
