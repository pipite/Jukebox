//---------------------------------------------------------------------------
//#include <System.SysUtils.hpp>
#include "JukeboxPCH1.h"
#pragma hdrstop

class XDropFolder;

#include "ClassDropFolder.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

__fastcall XDropFolder::XDropFolder(TComponent *owner) : TPanel(owner) {
	// Initialisation des membres
	Core = NULL;
	Parent = static_cast<TPanel*>(owner);
	IsProcessing = false;
	
	// Création et configuration de l'image
	Img = new TImage(this);
	Img->Parent = this;
	Img->Align = alClient;
	Img->Center = true;
	Img->Transparent = true;
	
	// Chargement de l'image avec gestion d'erreur
	try {
		Img->Picture->Bitmap->LoadFromFile("..\\Icon\\Drop.bmp");
	}
	catch (...) {
		// Si l'image ne peut pas être chargée, utiliser une couleur de fond distinctive
		Img->Visible = false;
		Color = clNavy;
	}
	
	// Activation du glisser-déposer
	DragAcceptFiles(Handle, true);
	
	// Configuration de l'apparence
	BevelOuter = bvLowered;
	Caption = "";
}

__fastcall XDropFolder::~XDropFolder(void) {
	// Désactivation du glisser-déposer
	DragAcceptFiles(Handle, false);
	
	// Libération des ressources
	if (Img) {
		delete Img;
		Img = NULL;
	}
}

void __fastcall XDropFolder::WMDropFiles(TMessage msg) {
	// Protection contre les appels multiples
	if (IsProcessing) return;
	IsProcessing = true;
	
	// Récupération du handle de drop
	HDROP hDrop = reinterpret_cast<HDROP>(msg.WParam);
	
	try {
		// Récupération du nombre de fichiers déposés
		UINT fileCount = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
		
		// Traitement du premier fichier/dossier uniquement
		if (fileCount > 0) {
			// Récupération de la taille du chemin
			UINT pathSize = DragQueryFile(hDrop, 0, NULL, 0);
			if (pathSize > 0) {
				// Allocation du buffer pour le chemin
				TCHAR* filePath = new TCHAR[pathSize + 1];
				
				try {
					// Récupération du chemin
					if (DragQueryFile(hDrop, 0, filePath, pathSize + 1)) {
						UnicodeString path = UnicodeString(filePath);
						
						// Vérification si c'est un dossier
						if (DirectoryExists(path)) {
							// Mise à jour du chemin de musique
							Core->Settings->MusicPath = path;
							
							// Changement de l'apparence pour indiquer le succès
							Color = clGreen;
							
							// Déclenchement du chargement
							if (Core && Core->Loader) {
								Core->Loader->Load();
							}
						}
						else {
							// Changement de l'apparence pour indiquer l'échec
							Color = clRed;
							
							// Affichage d'un message d'erreur
							if (Core && Core->Intf) {
								Core->Intf->ShowMessage("Please drop a valid folder.");
							}
						}
					}
				}
				__finally {
					// Libération du buffer
					delete[] filePath;
				}
			}
		}
	}
	__finally {
		// Libération des ressources de drop
		DragFinish(hDrop);
		
		// Réinitialisation de l'apparence après un délai
		SetTimer(Handle, 1, 1000, NULL);
		
		// Fin du traitement
		IsProcessing = false;
	}
}

void __fastcall XDropFolder::WMTimer(TMessage &msg) {
	// Réinitialisation de l'apparence
	if (msg.WParam == 1) {
		ResetAppearance();
		KillTimer(Handle, 1);
	}
	
	// Appel du gestionnaire par défaut
	TPanel::Dispatch(&msg);
}

void __fastcall XDropFolder::ResetAppearance() {
	// Restauration de l'apparence par défaut
	Color = clBtnFace;
	
	// Restauration de la visibilité de l'image
	if (Img) {
		Img->Visible = true;
	}
}


