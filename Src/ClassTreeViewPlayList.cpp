//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassTreeViewPlayList.h"

//---------------------------------------------------------------------------

// -----------------------------------------------------
// CLASS ClassTreeViewPlayList
// -----------------------------------------------------
__fastcall XTvPlayList::XTvPlayList(TComponent *owner, XCore *core) : TTreeView(owner){
    // Initialisation des pointeurs à NULL
    Core             = core;
    PTNTool          = NULL;
    PNR              = NULL;
    PTNR             = NULL;
    
    // Configuration de base
    Parent           = (TPanel*) owner;
    Images           = Core->Intf->TvIcones;
    Align            = alClient;
    ShowRoot         = false;
    ShowLines        = false;
    ShowButtons      = false;
    ReadOnly         = false;
    DoubleBuffered   = true;
    
    // Assignation des gestionnaires d'événements
    OnCustomDrawItem = OCustomDrawItem;
    OnEdited         = OEdited;
    OnDblClick       = ODblClick;
    OnMouseDown      = OMouseDown;
    OnDragOver       = ODragOver;
    OnDragDrop       = ODragDrop;
    OnEndDrag        = OEndDrag;
    OnResize         = OResize;

    // Configuration de la sélection multiple
    MultiSelect      = true;
    MultiSelectStyle = TMultiSelectStyle() << msShiftSelect << msControlSelect;

    // Configuration de la police
    Font->Size       = Core->FontSize;
    StyleElements    >> seFont;
    Font->Color      = Core->Settings->ColorPlaylist;
    
    try {
        // Création des objets
        PTNTool = new XTNodeTool();
        PNR     = new XNode(NULL, L"Playlist", NODETYPE::ROOT);
        
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

__fastcall XTvPlayList::~XTvPlayList(void) {
    // Nettoyage des ressources
    if (PNR) {
        PNR->Clear();
        delete PNR;
        PNR = NULL;
    }
    
    if (PTNTool) {
        delete PTNTool;
        PTNTool = NULL;
    }
}

void __fastcall XTvPlayList::Reset(void) {
    // Vérification des pointeurs avant utilisation
    if (!PNR) return;
    
    // Création du nœud racine si nécessaire
    if (PTNR == NULL) {
        PTNR = Items->AddObjectFirst(NULL, L"PlayList", PNR);
    }
    
    // Si le nœud racine n'a pas d'enfants, rien à faire
    if (PTNR->Count == 0) return;
    
    try {
        // Mise à jour groupée pour améliorer les performances
        Items->BeginUpdate();
        
        // Suppression des enfants du nœud visuel
        PTNR->DeleteChildren();
        
        // Suppression des enfants du nœud de données
        PNR->Clear();
        
        Items->EndUpdate();
    }
    catch (...) {
        // S'assurer que EndUpdate est appelé même en cas d'exception
        Items->EndUpdate();
    }
    
    // Rafraîchissement de l'affichage
    Refresh();
}

XNode* __fastcall XTvPlayList::GetNode(TTreeNode *tn) {
    // Optimisation: retour direct avec cast sécurisé
    return tn ? static_cast<XNode*>(tn->Data) : NULL;
}

void __fastcall XTvPlayList::Remove(TTreeNode *treenode) {
    // Vérifications préliminaires
    if (!treenode) return;
    
    XNode *cn = static_cast<XNode*>(treenode->Data);
    if (!cn || cn == PNR) return;
    
    // Sauvegarde du parent avant suppression
    TTreeNode *parentNode = treenode->Parent;
    
    // Détacher les données pour les chansons pour éviter les problèmes de mémoire
    if (cn->NodeType == NODETYPE::SONG) {
        treenode->Data = NULL;
    }
    
    // Marquer que la playlist a été modifiée
    if (Core && Core->Loader) {
        Core->Loader->SavePlayListNeeded = true;
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
        if (parentData &&
            parentData->NodeType == NODETYPE::FOLDER &&
            !parentNode->HasChildren) {
            Remove(parentNode);
        }
    }
}

void __fastcall XTvPlayList::OEdited(TObject *Sender, TTreeNode *Node, UnicodeString &S) {
    // Vérifications préliminaires
    if (!Node) return;
    
    XNode *cn = static_cast<XNode*>(Node->Data);
    if (!cn) return;
    
    // Mise à jour du chemin uniquement pour les nœuds non-chansons
    if (cn->NodeType != NODETYPE::SONG) {
        // Marquer que la playlist a été modifiée
        if (Core && Core->Loader) {
            Core->Loader->SavePlayListNeeded = true;
        }
        
        // Mise à jour du chemin
        cn->Path = S;
    }
}

void __fastcall XTvPlayList::ODblClick(TObject *Sender) {
    // Protection contre les doubles clics rapides avec une approche RAII
    // pour garantir que isProcessing est toujours réinitialisé à false
    struct ProcessingGuard {
        bool &flag;
        ProcessingGuard(bool &f) : flag(f) { flag = true; }
        ~ProcessingGuard() { flag = false; }
    };
    
    static bool isProcessing = false;
    if (isProcessing) return;
    
    // Cette garde garantit que isProcessing sera remis à false même en cas d'exception
    ProcessingGuard guard(isProcessing);
    
    try {
        // Vérification du nœud sélectionné
        TTreeNode* selectedNode = Selected;
        if (!selectedNode) {
            return;
        }
        
        XNode* cn = static_cast<XNode*>(selectedNode->Data);
        if (!cn) {
            return;
        }
        
        // Traitement selon le type de nœud
        if (cn->NodeType == NODETYPE::SONG) {
            // Vérification des pointeurs avant utilisation
            if (!Core || !Core->Mix || !Core->TvPast || !PTNTool) {
                return;
            }
            
            // Vérification si une chanson est déjà en cours de lecture
            if (Core->Mix->IsMixing) {
                // Attendre la fin du mixage en cours avant de lancer une nouvelle chanson
                Core->Mix->Current->FadeOutNow(Core->Settings->FadeOutDuration / 2);
            }
            
            XNode* nodepast = NULL;
            
            try {
                // Copie vers l'historique (Past)
                nodepast = PTNTool->CopyTo(Core->TvPast, selectedNode, Core->TvPast->TNR);
                if (!nodepast) {
                    return;
                }
                
                // Analyse de la chanson
                Core->Mix->Wave->Scan(nodepast);
                
                // Lecture de la chanson
                Core->Mix->NodeNow(nodepast);
                
                // Expansion de l'historique pour montrer la chanson ajoutée
                Core->TvPast->TNR->Expand(false);
            }
            catch (...) {
                // En cas d'erreur, s'assurer que les ressources sont libérées
                if (nodepast) {
                    delete nodepast;
                }
            }
        }
        else {
            // Chargement asynchrone du dossier si nécessaire
            if (!cn->Populated && cn->HaveChild) {
                try {
                    // Le thread se libérera automatiquement grâce à FreeOnTerminate = true
                    ThreadPopulate *thread = new ThreadPopulate(this, selectedNode, false);
                }
                catch (...) {
                    // Ignorer les exceptions lors de la création du thread
                }
            }
        }
    }
    catch (...) {
        // Capture des exceptions pour éviter les problèmes
    }
}

void __fastcall XTvPlayList::ExpandRoot(void) {
    // Vérification préliminaire
    if (!PTNR) return;
    
    // Récupération du nœud de données
    XNode *cn = static_cast<XNode*>(PTNR->Data);
    if (!cn) return;
    
    // Chargement asynchrone du dossier si nécessaire
    if (!cn->Populated && cn->HaveChild) {
        try {
            // Le thread se libérera automatiquement grâce à FreeOnTerminate = true
            ThreadPopulate *thread = new ThreadPopulate(this, PTNR, false);
        }
        catch (...) {
            // Ignorer les exceptions lors de la création du thread
        }
    }
    
    // Expansion du nœud racine
    PTNR->Expand(false);
}

void __fastcall XTvPlayList::OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw) {
    // Vérification des pointeurs avant utilisation
    if (!Sender || !Node) return;
    
    XNode *cn = static_cast<XNode*>(Node->Data);
    if (!cn) return;
    
    // Configuration commune pour tous les types
    Sender->Canvas->Brush->Color = clBlack;
    
    // Définir explicitement les styles de police (aucun style spécial par défaut)
    Sender->Canvas->Font->Style = TFontStyles();  // Ensemble vide = texte normal
    
    // Variables déclarées en dehors du switch pour éviter les erreurs de portée
    int baseIcon = 0;
    int selectedIcon = 0;
    int playlistIcon = 0;
    
    // Utilisation d'un switch pour une meilleure performance avec des types énumérés
    switch (cn->NodeType) {
        case NODETYPE::ROOT:
            SetIcon(Node, 0, 0, 0);
            return;
            
        case NODETYPE::SONG:
            // Optimisation: réduction des conditions et calculs redondants
            if (cn->CountSession > 0) {        // Rouge
                Sender->Canvas->Font->Color = clRed;
                baseIcon = 8;
                selectedIcon = 6;
                
                // Traitement spécial pour les éléments de playlist
                if (cn->CountPlayList > 0) {
                    SetIcon(Node, 12, 12, 10); // Trombone
                    return;
                }
            } else if (cn->CountPlay > 0) {    // Vert
                Sender->Canvas->Font->Color = clLime;
                baseIcon = 7;
                selectedIcon = 6;
            } else {                           // Jaune (CountPlay == 0)
                Sender->Canvas->Font->Color = clYellow;
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
            SetIcon(Node, 13, 13, 13); // PlayList
            return;
            
        default:
            return;
    }
}

void __fastcall XTvPlayList::SetIcon(TTreeNode *tn, int a, int b, int c) {
    // Optimisation: vérification de validité du nœud
    if (!tn) return;
    
    // Assignation directe des valeurs d'icônes
    tn->ImageIndex = a;
    tn->ExpandedImageIndex = b;
    tn->SelectedIndex = c;
}

void __fastcall XTvPlayList::AddPlayList(void) {
    // Vérification des pointeurs avant utilisation
    if (!Core || !Core->Loader || !PNR) return;
    
    // Marquer que la playlist a été modifiée
    Core->Loader->SavePlayListNeeded = true;
    
    XNode *cn = NULL;
    
    try {
        // Création d'un nouveau nœud de dossier
        cn = new XNode(PNR, L"List", NODETYPE::FOLDER);
        
        // Ajout du nœud à l'arborescence visuelle si le nœud racine est peuplé
        if (PNR->Populated) {
            try {
                Items->BeginUpdate();
                
                TTreeNode *newNode = Items->AddChildObject(PTNR, L"List", cn);
                
                Items->EndUpdate();
                
                // Expansion du nœud racine pour montrer le nouveau dossier
                if (PTNR) {
                    PTNR->Expand(false);
                }
            }
            catch (...) {
                // S'assurer que EndUpdate est appelé même en cas d'exception
                Items->EndUpdate();
                throw; // Relancer l'exception pour le traitement dans le bloc catch externe
            }
        }
    }
    catch (...) {
        // En cas d'erreur, nettoyer le nœud créé
        if (cn && !PNR->Populated) {
            // Si le nœud n'a pas été ajouté à l'arborescence visuelle, le supprimer
            delete cn;
        }
    }
}


// -----------------------------------------------------
//          DRAG DROP
// -----------------------------------------------------

void __fastcall XTvPlayList::OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
    // Vérification rapide pour éviter le traitement inutile
    if (Button != mbLeft || !Sender) return;
    
    // Éviter le cast inutile puisque nous savons que Sender est un TTreeView
    // (cette méthode est liée à l'événement OnMouseDown de this, qui est un TTreeView)
    TTreeView *tv = static_cast<TTreeView*>(Sender);
    
    // Vérification directe si un nœud existe à la position du clic
    TTreeNode* node = tv->GetNodeAt(X, Y);
    if (node) {
        tv->BeginDrag(false, -1);
    }
}

void __fastcall XTvPlayList::ODragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept) {
    // Vérification des pointeurs avant utilisation
    if (!Source) {
        Accept = false;
        return;
    }
    
    // Approche simplifiée pour vérifier les sources valides
    UnicodeString sourceClass = Source->ClassName();
    
    // Vérification directe avec des comparaisons de chaînes
    Accept = (sourceClass == L"XTvList") ||
             (sourceClass == L"XTvNext") ||
             (sourceClass == L"XTvPast") ||
             (sourceClass == L"XTvPlayList");
    
    // Si aucune source n'est acceptée, acceptons au moins les TreeView
    if (!Accept) {
        Accept = Source->InheritsFrom(__classid(TTreeView));
    }
}

void __fastcall XTvPlayList::ODragDrop(TObject *Sender, TObject *Source, int X, int Y) {
    // Vérification des pointeurs avant utilisation
    if (!Source || !PTNTool) return;
    
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
        { L"XTvPast",     0 },
        { L"XTvPlayList", 1 }
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
    if (src == -1) return;

    // Cast de la source en TTreeView
    TTreeView *tvs = static_cast<TTreeView*>(Source);
    if (!tvs || tvs->SelectionCount == 0) return;

    // Détermination de la destination
    TTreeNode *tnd = GetNodeAt(X, Y);
    XNode *nd = tnd ? static_cast<XNode*>(tnd->Data) : NULL;

    // Si aucun nœud n'est trouvé à la position, utiliser le nœud racine
    if (tnd == NULL) {
        tnd = TNR;
        nd = NR;
    }

    // S'assurer que le nœud de destination est peuplé
    if (nd && !nd->Populated && !nd->IsSong) {
        nd->Populated = true;
    }

    try {
        // Début de la mise à jour groupée
        Items->BeginUpdate();
        
        try {
            // Traitement de tous les nœuds sélectionnés
            bool success = false;
            for (unsigned i = 0; i < tvs->SelectionCount; i++) {
                TTreeNode *tns = tvs->Selections[i];
                if (!tns) continue;
                
                try {
                    // Optimisation: utilisation d'une table de décision pour le traitement
                    if (src == 0) {
                        // Source externe: copier
                        XNode* result = PTNTool->CopyTo(this, tns, tnd);
                        if (result != NULL) success = true;
                    } else { // src == 1
                        // Source interne: déplacer
                        PTNTool->MoveTo(this, tns, tnd);
                        success = true;
                    }
                    
                    // Marquer que la playlist a été modifiée
                    if (Core && Core->Loader) {
                        Core->Loader->SavePlayListNeeded = true;
                    }
                }
                catch (...) {
                    // Ignorer les exceptions potentielles lors de la copie/déplacement
                }
            }
            
            // Expansion du nœud de destination pour montrer les nouveaux éléments
            if (success && tnd) {
                tnd->Expand(false);
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

void __fastcall XTvPlayList::OEndDrag(TObject *Sender, TObject *Target, int X, int Y) {
    // Vérification des pointeurs avant utilisation
    if (!Target || !Core || !Core->Loader) return;
    
    // Optimisation: mise à jour conditionnelle uniquement si nécessaire
    try {
        // Mise à jour de l'interface après un glisser-déposer réussi
        Update();
        
        // Marquer que la playlist a été modifiée
        Core->Loader->SavePlayListNeeded = true;
    }
    catch (...) {
        // Ignorer les exceptions potentielles
    }
}

void __fastcall XTvPlayList::OResize(TObject *Sender) {
    // Optimisation: masquage de la barre de défilement horizontale lors du redimensionnement
    ShowScrollBar(Handle, SB_HORZ, false);
    
    // Ajustement de la largeur des colonnes si nécessaire
    if (PTNR && PTNR->IsVisible) {
        // Expansion du nœud racine pour assurer la visibilité des éléments
        PTNR->Expand(false);
    }
}