//---------------------------------------------------------------------------
#ifndef ClassSavFileH
#define ClassSavFileH

#include <fstream>

class XFile;

#include "ClassCore.h"

class XFile {
private:
    XCore        *Core;        // Référence au noyau de l'application
	std::ifstream Pf;          // Flux d'entrée pour la lecture des fichiers
    TStringList  *PFile;       // Liste de chaînes pour manipuler le contenu des fichiers
    unsigned      PLevel;      // Niveau actuel dans l'arborescence lors de la sauvegarde récursive

    // Méthodes pour obtenir des informations sur les fichiers
    UnicodeString __fastcall GetPathArbo();
    UnicodeString __fastcall GetPathPlaylist();
    bool          __fastcall GetHaveArbo();
	bool          __fastcall GetHavePlaylist();
    UnicodeString __fastcall GetExtendDate();

    // Méthodes pour les opérations de fichier
    bool          __fastcall MoveToArchive(const UnicodeString& src);
    void          __fastcall SaveRecurseNode(XNode *node);
    void          __fastcall RemoveRootPath(bool bypass);

    // Vérifie si le Core est valide
    bool          __fastcall IsValid() const { return Core != nullptr; }

public:
    // Constructeur et destructeur
	XFile(XCore *core);
    ~XFile();

    // Méthodes de conversion entre formats
    bool __fastcall ConvertPlaylist();

    // Méthodes de sauvegarde
    bool __fastcall Save();
    bool __fastcall SavePlaylist(const UnicodeString& filename);

    // Méthodes d'archivage
    bool __fastcall ArchivePlayList();
    bool __fastcall ArchiveArbo();

    // Méthode de purge des anciennes sauvegardes
    bool __fastcall Purge(const UnicodeString& filename);

    // Propriétés
    __property UnicodeString PathArbo     = {read = GetPathArbo};
    __property UnicodeString PathPlaylist = {read = GetPathPlaylist};
    __property bool          HaveArbo     = {read = GetHaveArbo};
    __property bool          HavePlaylist = {read = GetHavePlaylist};
    __property UnicodeString ExtendDate   = {read = GetExtendDate};
};



#endif
