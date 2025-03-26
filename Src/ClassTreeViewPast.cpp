//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassTreeViewPast.h"

//---------------------------------------------------------------------------

// -----------------------------------------------------
// CLASS XTvPast
// -----------------------------------------------------
__fastcall XTvPast::XTvPast(TComponent *owner, XCore *core) : TTreeView(owner) {
    // Initialisation des pointeurs à NULL
    Core             = NULL;
    PTNTool          = NULL;
    PNR              = NULL;
    PTNR             = NULL;

    try {
        // Propriétés de base
        Core             = core;
        Parent           = static_cast<TPanel*>(owner);

        // Configuration de l'apparence en un bloc
        Align            = alClient;
        Images           = Core->Intf->TvIcones;
        ShowRoot         = false;
        ShowLines        = true;
        ShowButtons      = true;
        ReadOnly         = true;
        DoubleBuffered   = true;

        // Assignation des gestionnaires d'événements en un bloc
        OnCustomDrawItem = OCustomDrawItem;
        OnDblClick       = ODblClick;
        OnMouseDown      = OMouseDown;
        OnResize         = OResize;
        OnDragOver       = ODragOver;
        OnDragDrop       = ODragDrop;
        OnEndDrag        = OEndDrag;

        // Configuration de la sélection multiple
        MultiSelect      = true;
        MultiSelectStyle = TMultiSelectStyle() << msShiftSelect << msControlSelect;

        // Configuration de la police
        StyleElements    = StyleElements >> seFont;
        Font->Size       = Core->FontSize;
        Font->Color      = Core->Settings->ColorPast;

        // Initialisation des outils et des nœuds
        PTNTool          = new XTNodeTool();

        // Création du nœud racine avec horodatage
        UnicodeString rootLabel = L"Starting at: " + FormatDateTime(L"hh:nn", Now());
        PNR              = new XNode(NULL, rootLabel, NODETYPE::ROOT);

        // Initialisation de l'arborescence
        Reset();
    }
    catch (...) {
        // Nettoyage en cas d'exception
        if (PNR) {
            delete PNR;
            PNR = NULL;
        }
        if (PTNTool) {
            delete PTNTool;
            PTNTool = NULL;
        }
        throw; // Relancer l'exception
    }
}

__fastcall XTvPast::~XTvPast(void) {
    // Nettoyage optimisé des ressources

    // Libération du nœud racine
    if (PNR) {
        PNR->Clear();  // Libération des enfants d'abord
        delete PNR;    // Puis libération du nœud lui-même
        PNR = NULL;    // Éviter les doubles libérations
    }

    // Libération de l'outil de nœuds
    if (PTNTool) {
        delete PTNTool;
        PTNTool = NULL;  // Éviter les doubles libérations
    }

    // Réinitialisation des autres pointeurs
    Core = NULL;
    PTNR = NULL;
}

void __fastcall XTvPast::Reset(void) {
	// Initialisation optimisée de l'arborescence
	
	// Création du nœud racine visuel si nécessaire
	if (!PTNR) {
		// Création avec horodatage actuel
		UnicodeString rootLabel = L"Starting at: " + FormatDateTime(L"hh:nn", Now());
		PTNR = Items->AddObjectFirst(NULL, rootLabel, PNR);
	}
	
	// Vérification si le nettoyage est nécessaire
	if (PTNR->Count == 0) {
		return;  // Rien à nettoyer
	}
	
	// Mise à jour groupée pour améliorer les performances
	Items->BeginUpdate();
	
	// Suppression des enfants du nœud visuel
	PTNR->DeleteChildren();
	
	// Suppression des enfants du nœud de données
	PNR->Clear();
	
	Items->EndUpdate();
	
	// Rafraîchissement de l'affichage
	Refresh();
}

void __fastcall XTvPast::ODblClick(TObject *Sender) {
    // Protection contre les doubles clics rapides
    static bool isProcessing = false;
    if (isProcessing) return;
    isProcessing = true;

    try {
        // Vérifications préliminaires optimisées
        TTreeNode* selectedNode = Selected;
        if (!selectedNode) {
            isProcessing = false;
            return;
        }

        XNode* cn = static_cast<XNode*>(selectedNode->Data);
        if (!cn) {
            isProcessing = false;
            return;
        }

        // Variable déclarée en dehors du switch pour éviter les erreurs de portée
        XNode* newnode = NULL;

        // Traitement selon le type de nœud
        switch (cn->NodeType) {
            case NODETYPE::ROOT:
            case NODETYPE::FOLDER:
                // Optimisation: combinaison des conditions pour réduire les vérifications
                if (!cn->Populated && cn->HaveChild) {
                    try {
                        // Créer un thread pour peupler le nœud
                        ThreadPopulate *thread = new ThreadPopulate(this, selectedNode, false);

                        // Attendre que le thread termine (synchrone) avec un timeout
                        int timeout = 0;
                        const int MAX_TIMEOUT = 5000; // 5 secondes maximum

                        while (!cn->Populated && timeout < MAX_TIMEOUT) {
                            Application->ProcessMessages();
                            Sleep(10);
                            timeout += 10;
                        }

                        // Si le timeout est atteint, forcer le nœud à être considéré comme peuplé
                        if (timeout >= MAX_TIMEOUT && !cn->Populated) {
                            cn->Populated = true;
                        }

                        // Plier/déplier le dossier
                        if (selectedNode->Expanded) {
                            selectedNode->Collapse(false);
                        } else {
                            selectedNode->Expand(false);
                        }
                    }
                    catch (...) {
                        // Ignorer les exceptions lors du peuplement
                    }
                } else {
                    // Si le dossier est déjà peuplé, simplement le plier/déplier
                    if (selectedNode->Expanded) {
                        selectedNode->Collapse(false);
                    } else {
                        selectedNode->Expand(false);
                    }
                }
                break;

            case NODETYPE::SONG:
                try {
                    // Traitement optimisé pour les chansons
                    // Analyse de la chanson
                    Core->Mix->Wave->Scan(cn);

                    // Lecture directe de la chanson sans la copier à nouveau dans l'historique
                    // Cela évite la boucle infinie de copies
                    Core->Mix->NodeNow(cn);
                }
                catch (...) {
                    // Ignorer les exceptions lors de la lecture
                }
                break;

            default:
                // Gestion des types non reconnus
                break;
        }
    }
    catch (...) {
        // Capture des exceptions pour éviter les problèmes
    }

    isProcessing = false;
}

void __fastcall XTvPast::OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw) {
	XNode *cn = static_cast<XNode*>(Node->Data);
	if (!cn) return;
	
	// Configuration commune pour tous les types
	Sender->Canvas->Brush->Color = clBlack;
	
	// Variables déclarées en dehors du switch pour éviter les erreurs de portée
	int baseIcon = 0;
	int selectedIcon = 0;
	int iconIndex = 0;
	int selectedIndex = 0;
	int playlistIcon = 0;
	
	// Utilisation d'un switch pour une meilleure performance avec des types énumérés
	switch (cn->NodeType) {
		case NODETYPE::ROOT:
			SetIcon(Node, 0, 0, 0);
			return;
			
		case NODETYPE::SONG:
			// Configuration de la couleur du texte
			Sender->Canvas->Font->Color = clRed;
			
			// Optimisation: réduction des conditions et calculs redondants
			if (cn->CountSession > 0) {        // Rouge
				baseIcon = 8;
				selectedIcon = 6;
				
				// Traitement spécial pour les éléments de playlist
				if (cn->CountPlayList > 0) {
					SetIcon(Node, 12, 12, 10); // Trombone
					return;
				}
			} else if (cn->CountPlay > 0) {    // Vert
				baseIcon = 7;
				selectedIcon = 6;
			} else {                           // Gris (CountPlay == 0)
				baseIcon = 5;
				selectedIcon = 6;
			}
			
			// Application des icônes pour les éléments sans playlist ou cas par défaut
			if (cn->CountPlayList > 0) {
				// Calcul de l'icône pour les éléments avec playlist
				playlistIcon = (cn->CountPlay > 0) ? 11 : 9;
				SetIcon(Node, playlistIcon, playlistIcon, 10);
			} else {
				SetIcon(Node, baseIcon, baseIcon, selectedIcon);
			}
			return;
			
		case NODETYPE::FOLDER:
			// Optimisation pour les dossiers
			iconIndex = cn->CountFolder > 0 ? 3 : 1;
			selectedIndex = cn->CountFolder > 0 ? 4 : 2;
			SetIcon(Node, iconIndex, iconIndex, selectedIndex);
			return;
			
		default:
			return;
	}
}

void __fastcall XTvPast::SetIcon(TTreeNode *tn, int a, int b, int c) {
	// Optimisation: vérification de validité du nœud
	if (!tn) return;
	
	// Assignation directe des valeurs d'icônes
	tn->ImageIndex = a;
	tn->ExpandedImageIndex = b;
	tn->SelectedIndex = c;
}

// -----------------------------------------------------
//          DRAG DROP
// -----------------------------------------------------

void __fastcall XTvPast::OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
	// Vérification rapide pour éviter le traitement inutile
	if (Button != mbLeft) return;
	
	// Éviter le cast inutile puisque nous savons que Sender est un TTreeView
	// (cette méthode est liée à l'événement OnMouseDown de this, qui est un TTreeView)
	TTreeView *tv = static_cast<TTreeView*>(Sender);
	
	// Vérification directe si un nœud existe à la position du clic
	TTreeNode* node = tv->GetNodeAt(X, Y);
	if (node) {
		tv->BeginDrag(false, -1);
	}
}

void __fastcall XTvPast::ODragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept) {
	// Optimisation: utilisation d'une table de hachage statique pour vérifier les sources valides
	// Cette approche est plus efficace pour un grand nombre de vérifications
	static std::unordered_set<UnicodeString> validSources = {
		"XTvList",
		"XTvNext",
		"XTvPast",
		"XTvPlayList"
	};
	
	// Vérification directe si la classe source est dans l'ensemble des sources valides
	Accept = validSources.find(Source->ClassName()) != validSources.end();
}

void __fastcall XTvPast::ODragDrop(TObject *Sender, TObject *Source, int X, int Y) {
    // Protection contre les appels récursifs
    static bool isProcessing = false;
    if (isProcessing) return;
    isProcessing = true;

    try {
        // Détermination optimisée de la source
        int src;
        UnicodeString sourceClassName = Source->ClassName();

        // Utilisation d'une table de correspondance pour déterminer la source
        static struct {
            const wchar_t* className;
            int sourceType;
        } sourceTypes[] = {
            { L"XTvList",     0 },
            { L"XTvNext",     0 },
            { L"XTvPast",     1 },
            { L"XTvPlayList", 0 }
        };

        // Recherche du type de source
        src = -1;
        for (int i = 0; i < sizeof(sourceTypes)/sizeof(sourceTypes[0]); i++) {
            if (sourceClassName == sourceTypes[i].className) {
                src = sourceTypes[i].sourceType;
                break;
            }
        }

        // Vérification de la validité de la source
        if (src == -1) {
            isProcessing = false;
            return;
        }

        // Cast de la source en TTreeView
        TTreeView *tvs = static_cast<TTreeView*>(Source);
        if (!tvs || tvs->SelectionCount == 0) {
            isProcessing = false;
            return;
        }

        // Détermination de la destination
        TTreeNode *tnd = GetNodeAt(X, Y);
        XNode *nd = tnd ? static_cast<XNode*>(tnd->Data) : NULL;

        try {
            // Début de la mise à jour groupée
            Items->BeginUpdate();

            try {
                // Traitement de tous les nœuds sélectionnés
                for (unsigned i = 0; i < tvs->SelectionCount; i++) {
                    TTreeNode *tns = tvs->Selections[i];
                    if (!tns) continue;

                    // Vérification pour éviter de déposer un nœud sur lui-même ou sur un de ses descendants
                    XNode *ns = static_cast<XNode*>(tns->Data);
                    if (!ns) continue;

                    // Si la destination est un descendant de la source, ignorer
                    if (nd && nd->HasAsParent(ns)) continue;

                    try {
                        // Optimisation: utilisation d'une table de décision pour le traitement
                        if (nd == NULL) {
                            // Destination: racine
                            if (src == 0) {
                                PTNTool->CopyTo(this, tns, TNR);
                            } else { // src == 1
                                PTNTool->MoveTo(this, tns, TNR);
                            }
                        } else {
                            // Destination: nœud spécifique
                            if (src == 0) {
                                PTNTool->CopyTo(this, tns, tnd);
                            } else { // src == 1
                                PTNTool->MoveTo(this, tns, tnd);
                            }
                        }
                    }
                    catch (...) {
                        // Ignorer les exceptions potentielles lors de la copie/déplacement
                    }
                }
            }
            catch (...) {
                // Ignorer les exceptions potentielles
            }

            // Fin de la mise à jour groupée
            Items->EndUpdate();
        }
        catch (...) {
            // S'assurer que EndUpdate est appelé même en cas d'exception
            Items->EndUpdate();
        }
    }
    catch (...) {
        // Capture des exceptions pour éviter les problèmes
    }

    isProcessing = false;
}

void __fastcall XTvPast::OEndDrag(TObject *Sender, TObject *Target, int X, int Y) {
	// Optimisation: mise à jour conditionnelle uniquement si nécessaire
	if (Target) {
		// Mise à jour de l'interface après un glisser-déposer réussi
		Update();
	}
}

void __fastcall XTvPast::OResize(TObject *Sender) {
	// Optimisation: masquage de la barre de défilement horizontale lors du redimensionnement
	ShowScrollBar(Handle, SB_HORZ, false);
	
	// Ajustement de la largeur des colonnes si nécessaire
	if (PTNR && PTNR->IsVisible) {
		// Expansion du nœud racine pour assurer la visibilité des éléments
		PTNR->Expand(false);
	}
}

XNode* __fastcall XTvPast::GetNode(TTreeNode *tn) {
	// Optimisation: retour direct avec cast sécurisé
	return tn ? static_cast<XNode*>(tn->Data) : NULL;
}

void __fastcall XTvPast::Remove(TTreeNode *treenode) {
    // Protection contre les appels récursifs
    static bool isProcessing = false;
    if (isProcessing) return;
    isProcessing = true;

    try {
        // Vérifications préliminaires
        if (!treenode) {
            isProcessing = false;
            return;
        }

        XNode *cn = static_cast<XNode*>(treenode->Data);
        if (!cn || cn == PNR) {
            isProcessing = false;
            return;
        }

        // Sauvegarde du parent avant suppression
        TTreeNode *parentNode = treenode->Parent;

        // Détacher les données pour les chansons pour éviter les problèmes de mémoire
        if (cn->NodeType == NODETYPE::SONG) {
            treenode->Data = NULL;
        }

        try {
            // Suppression du nœud et de ses données
            delete treenode;
            delete cn;
        }
        catch (...) {
            // Ignorer les exceptions potentielles lors de la suppression
        }

        // Vérification et nettoyage du parent si nécessaire
        if (parentNode) {
            XNode *parentData = static_cast<XNode*>(parentNode->Data);

            // Suppression récursive des dossiers vides
            // Utiliser une approche itérative pour éviter la récursion profonde
            while (parentData &&
                   parentData->NodeType == NODETYPE::FOLDER &&
                   !parentNode->HasChildren) {

                TTreeNode *grandParentNode = parentNode->Parent;

                // Détacher les données pour éviter les problèmes de mémoire
                parentNode->Data = NULL;

                try {
                    // Suppression du nœud et de ses données
                    delete parentNode;
                    delete parentData;
                }
                catch (...) {
                    // Ignorer les exceptions potentielles lors de la suppression
                }

                // Passer au parent suivant
                parentNode = grandParentNode;
                parentData = parentNode ? static_cast<XNode*>(parentNode->Data) : NULL;

                // Éviter de supprimer le nœud racine
                if (parentData == PNR) {
                    break;
                }
            }
        }
    }
    catch (...) {
        // Capture des exceptions pour éviter les problèmes
    }

    isProcessing = false;
}



