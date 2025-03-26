//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ThreadScanPlaylist.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

__fastcall ThreadScanPlaylist::ThreadScanPlaylist(XCore *core, bool CreateSuspended) : TThread(CreateSuspended) {
	Core = core;
	Core->TvPlayList->TNR->Text = "Loading Playlist, Please wait...";
	Core->TvPlayList->Enabled = false;
	PFile = new TStringList();
	PFile->Clear();
}

void __fastcall ThreadScanPlaylist::Done(void) {
	Core->TvPlayList->ExpandRoot();
	Core->TvPlayList->Enabled = true;
	Core->TvPlayList->TNR->Text = "PlayList";
	delete PFile;
}

void __fastcall ThreadScanPlaylist::Abort(void) {
	Core->TvPlayList->ExpandRoot();
	Core->TvPlayList->Enabled = true;
	Core->TvPlayList->TNR->Text = "PlayList";
	delete PFile;
}

//---------------------------------------------------------------------------
void __fastcall ThreadScanPlaylist::Execute() {
	int i, j, letter;
	char a;
	XNode *na[50], *naaz[50], *ns;
	UnicodeString s, f, fullPath;
	unsigned p, nt, cpttile , cp;

	na[0] = NULL;

	if ( !FileExists(Core->Settings->SettingPath + "\\PlayList.sav") ) { Synchronize ( Abort ); return; }
	Core->TvPlayList->Reset();
	if ( !Core->SavFile->ConvertPlaylist() ) { Synchronize ( Abort ); return; }
	PFile->LoadFromFile(Core->Settings->SettingPath + "\\PlayList.sav");
	
	// Première passe : créer tous les dossiers et les ajouter au modèle de données
	for (i = 1; i < PFile->Count ; i++) {
		s      = PFile->Strings[i]; p = s.Pos(',');
		PLevel = (unsigned) s.SubString(1,p-1).ToInt() ;
		s      = s.SubString(p+1,s.Length()); p = s.Pos(',');
		cp     = (unsigned) s.SubString(1,p-1).ToInt();
		s      = s.SubString(p+1,s.Length()); p = s.Pos(',');
		nt     = (unsigned) s.SubString(1,p-1).ToInt();
		s      = s.SubString(p+1,s.Length());
		
		// Si c'est le nœud racine ou un dossier, utiliser le chemin tel quel
		if (nt == NODETYPE::ROOT || nt == NODETYPE::FOLDER) {
			fullPath = s;
		}
		// Si c'est une chanson, ajouter MusicPath au chemin
		else if (nt == NODETYPE::SONG) {
			// Vérifier si le chemin est déjà absolu
			if (s.Pos(":") > 0) {
				fullPath = s;
			} else {
				fullPath = Core->Settings->MusicPath + "\\" + s;
			}
		}
		
		f = UpperCase(ExtractFileName(fullPath));

		switch (nt) {
			case NODETYPE::ROOT   : {
				Core->TvPlayList->NR->Path = fullPath;
				na[0] = Core->TvPlayList->NR;
				break;
			}
			case NODETYPE::FOLDER : {
				// Créer le nœud de dossier
				na[PLevel] = na[PLevel-1]->Add(fullPath,nt);
				break;
			}
			case NODETYPE::SONG : {
				// Rechercher la chanson dans l'arborescence
				ns = Core->TvList->NRSong->Find(fullPath);
				if (ns == NULL) {
					// Journalisation : chanson non trouvée
					// OutputDebugString((L"Chanson non trouvée : " + fullPath).c_str());
					break;
				}
				
				// Créer le lien vers la chanson
				XNode *songNode = na[PLevel-1]->Link(ns);
				ns->CountPlayList += 1;
				
				// Journalisation : chanson ajoutée
				// OutputDebugString((L"Chanson ajoutée : " + songNode->FileName).c_str());
				break;
			}
		}
	}
	
	// Deuxième passe : ajouter tous les dossiers et les chansons à l'arbre visuel
	Core->TvPlayList->Items->BeginUpdate();
	
	// Fonction récursive pour ajouter un nœud et tous ses enfants à l'arbre visuel
	std::function<void(XNode*, TTreeNode*)> addNodeAndChildren = [&](XNode* node, TTreeNode* parentTn) {
		if (!node) return;
		
		// Marquer le nœud comme peuplé
		node->Populated = true;
		
		// Parcourir tous les enfants du nœud
		for (unsigned i = 0; i < node->CountChild; i++) {
			XNode* childNode = node->ChildAt(i);
			if (!childNode) continue;
			
			// Ajouter l'enfant à l'arbre visuel
			TTreeNode* childTn = Core->TvPlayList->Items->AddChildObject(parentTn, childNode->FileName, childNode);
			
			// Journalisation : nœud ajouté
			// OutputDebugString((L"Nœud ajouté : " + childNode->FileName + L" (Type : " + UnicodeString(childNode->NodeType) + L")").c_str());
			
			// Si l'enfant est un dossier, ajouter récursivement ses enfants
			if (!childNode->IsSong && childNode->HaveChild) {
				addNodeAndChildren(childNode, childTn);
			}
		}
	};
	
	// Ajouter le nœud racine et tous ses enfants à l'arbre visuel
	addNodeAndChildren(Core->TvPlayList->NR, Core->TvPlayList->TNR);
	
	Core->TvPlayList->Items->EndUpdate();
	
	Synchronize ( Done );
}
//---------------------------------------------------------------------------