//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassTreeViewNext.h"

//---------------------------------------------------------------------------

// -----------------------------------------------------
// CLASS XTvNext
// -----------------------------------------------------
__fastcall XTvNext::XTvNext(TComponent *owner, XCore *core) : TTreeView(owner) {
    // Initialisation optimisée des propriétés en regroupant les opérations similaires
    
    // Propriétés de base
    Parent         = static_cast<TPanel*>(owner);
    Core           = core;
    
    // Initialisation des pointeurs à NULL
    PTNTool        = NULL;
    PNR            = NULL;
    PTNR           = NULL;
    
    // Configuration de l'apparence en un bloc
    Images         = Core->Intf->TvIcones;
    Align          = alClient;
    ShowRoot       = false;
    ShowLines      = true;
    ShowButtons    = true;
    ReadOnly       = true;
    DoubleBuffered = true;

    // Activer le pliage/dépliage automatique des dossiers lors d'un double-clic
    HideSelection  = false;

    // Configuration des gestes tactiles
    Touch->StandardGestures = TStandardGestures() << sgRight << sgLeft;

    // Assignation des gestionnaires d'événements en un bloc
    OnDblClick       = ODblClick;
    OnMouseDown      = OMouseDown;
    OnDragOver       = ODragOver;
    OnDragDrop       = ODragDrop;
    OnEndDrag        = OEndDrag;
    OnCustomDrawItem = OCustomDrawItem;
    OnResize         = OResize;
    OnExpanding      = OExpanding;

    // Activation des boutons +/- pour plier/déplier les nœuds
    ShowButtons      = true;

    // Configuration de la sélection multiple
    MultiSelect      = true;
    MultiSelectStyle = TMultiSelectStyle() << msShiftSelect << msControlSelect;

    // Configuration de la police
    StyleElements    = StyleElements >> seFont;
    Font->Size       = Core->FontSize;
    Font->Color      = Core->Settings->ColorNext;

    try {
        // Initialisation des outils et des nœuds
        PTNTool = new XTNodeTool();
        PNR     = new XNode(NULL, L"Next", NODETYPE::ROOT);
        
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

__fastcall XTvNext::~XTvNext(void) {
    // Nettoyage optimisé des ressources
    
    // Libération du nœud racine
    if (PNR) {
        PNR->Clear();  // Libération des enfants d'abord
        delete PNR;    // Puis libération du nœud lui-même
        PNR = NULL;
    }
    
    // Libération de l'outil de nœuds
    if (PTNTool) {
        delete PTNTool;
        PTNTool = NULL;
    }
}

XNode* __fastcall XTvNext::GetNode(TTreeNode *tn) {
    // Optimisation: retour direct avec cast sécurisé
    return tn ? static_cast<XNode*>(tn->Data) : NULL;
}

void __fastcall XTvNext::Reset(void) {
    // Vérification des pointeurs avant utilisation
    if (!PNR) return;
    
    // Initialisation optimisée de l'arborescence
    
    // Création du nœud racine si nécessaire
    if (!PTNR) {
        PTNR = Items->AddObjectFirst(NULL, L"Next", PNR);
    }
    // Nettoyage des enfants existants
    else if (PTNR->Count > 0) {
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
    }
    
    // Marquer le nœud comme peuplé
    PNR->Populated = true;
    
    // Rafraîchissement de l'affichage
    Refresh();
}

void __fastcall XTvNext::Randomize(void) {
    // Optimisation: utilisation de BeginUpdate/EndUpdate pour réduire les rafraîchissements
    try {
        Items->BeginUpdate();
        
        // Réinitialisation de l'arborescence
        Reset();
        
        // Remplissage avec des chansons aléatoires
        Fill();
        
        Items->EndUpdate();
    }
    catch (...) {
        // S'assurer que EndUpdate est appelé même en cas d'exception
        Items->EndUpdate();
    }
}

XNode* __fastcall XTvNext::Pop(void) {
    // Optimisation: réduction des variables locales et des vérifications redondantes
    
    // Obtention du prochain nœud à jouer
    TTreeNode *nextNode = GetTNNext();
    if (!nextNode) {
        return NULL;  // Aucun nœud disponible
    }
    
    XNode *newNode = NULL;
    
    try {
        // Copie du nœud vers l'historique (Past)
        if (Core && Core->TvPast && PTNTool) {
            newNode = PTNTool->CopyTo(Core->TvPast, nextNode, Core->TvPast->TNR);
        }
        
        // Suppression du nœud de la liste "Next"
        Remove(nextNode);
        
        // Retour du nouveau nœud copié
        return newNode;
    }
    catch (...) {
        // En cas d'erreur, s'assurer que les ressources sont libérées
        return NULL;
    }
}

void __fastcall XTvNext::Remove(TTreeNode *treenode) {
    // Protection contre les appels récursifs avec une approche RAII
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
        // Vérifications préliminaires
        if (!treenode) {
            return;
        }

        XNode *cn = static_cast<XNode*>(treenode->Data);
        if (!cn || cn == PNR) {
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
            if (parentData && parentData->NodeType == NODETYPE::FOLDER && !parentNode->HasChildren) {
                // Appel récursif sans Fill() pour éviter des appels multiples
                bool needFill = true;
                RemoveInternal(parentNode, needFill);
            }
        }

        // Remplissage de la liste seulement si nécessaire
        // Vérification du nombre de chansons actuel
        if (Core && Core->Settings) {
            unsigned currentSongCount = CountSong;
            int targetCount = Core->Settings->AutoSelect;

            if (currentSongCount < targetCount) {
                // Utiliser un try-catch pour éviter les problèmes si Fill() échoue
                try {
                    Fill();
                }
                catch (...) {
                    // Ignorer les exceptions potentielles lors du remplissage
                }
            }

            // Mise à jour de la prochaine chanson à jouer
            if (Core->Mix) {
                Core->Mix->Next->Node = GetNext();
            }
        }
    }
    catch (...) {
        // Capture des exceptions pour éviter les problèmes
    }
}

void __fastcall XTvNext::RemoveInternal(TTreeNode *treenode, bool &needFill) {
    // Méthode interne pour la suppression récursive sans Fill() à chaque étape
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
    
    // Suppression du nœud et de ses données
    try {
        delete treenode;
        delete cn;
    } catch (...) {
        // Ignorer les exceptions potentielles lors de la suppression
        // Cela peut arriver si le nœud est déjà en cours de suppression par un autre thread
    }
    
    // Vérification et nettoyage du parent si nécessaire
    if (parentNode) {
        XNode *parentData = static_cast<XNode*>(parentNode->Data);
        if (parentData && parentData->NodeType == NODETYPE::FOLDER && !parentNode->HasChildren) {
            RemoveInternal(parentNode, needFill);
        }
    }
}

TTreeNode* __fastcall XTvNext::GetTNNext(void) {
    // Vérification rapide pour éviter le traitement inutile
    if (!PTNR || !PTNR->HasChildren) return NULL;
    
    // Parcours optimisé des nœuds enfants
    for (TTreeNode* n = PTNR->getFirstChild(); n != NULL; n = n->GetNext()) {
        XNode* c = static_cast<XNode*>(n->Data);
        if (!c) continue;
        
        // Traitement selon le type de nœud
        switch (c->NodeType) {
            case NODETYPE::FOLDER:
                // Chargement asynchrone du dossier si nécessaire
                if (!c->Populated && c->HaveChild) {
                    try {
                        // Utiliser la méthode RecursePopulate de XTNodeTool pour peupler le dossier
                        // Cette méthode est plus sûre que de créer directement un thread
                        if (PTNTool) {
                            PTNTool->RecursePopulate(this, n, false);
                        }
                        else {
                            // Si PTNTool n'est pas disponible, marquer le nœud comme peuplé
                            // pour éviter de tenter de le peupler à nouveau
                            c->Populated = true;
                        }
                    }
                    catch (...) {
                        // En cas d'erreur, marquer le nœud comme peuplé
                        // pour éviter de tenter de le peupler à nouveau
                        c->Populated = true;
                    }
                }
                break;
                
            case NODETYPE::SONG:
                // Retour immédiat si une chanson est trouvée
                return n;
                
            default:
                // Ignorer les autres types de nœuds
                break;
        }
    }
    
    // Aucune chanson trouvée
    return NULL;
}

XNode* __fastcall XTvNext::GetNext(void) {
    // Optimisation: simplification avec retour direct
    TTreeNode *nextNode = GetTNNext();
    
    // Retour direct avec cast sécurisé
    return nextNode ? static_cast<XNode*>(nextNode->Data) : NULL;
}

void __fastcall XTvNext::Fill(void) {
    // Protection contre les appels récursifs avec une approche RAII
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
        // Vérification Loader terminé
        if (!Core || !Core->Loader || !Core->Loader->Loaded) {
            return;
        }

        // Vérification préalable pour éviter le traitement inutile
        if (!Core->TvList || !Core->TvList->NRSong || Core->TvList->NRSong->CountSong == 0) {
            return;
        }

        // Calcul du nombre de chansons à ajouter
        unsigned currentSongCount = CountSong;
        int targetCount = Core->Settings->AutoSelect;

        // Vérification si des chansons doivent être ajoutées
        if (currentSongCount >= targetCount) {
            return;
        }

        // Structure pour garantir que EndUpdate est appelé même en cas d'exception
        struct UpdateGuard {
            TTreeView *tree;
            UpdateGuard(TTreeView *t) : tree(t) { tree->Items->BeginUpdate(); }
            ~UpdateGuard() { tree->Items->EndUpdate(); }
        };

        try {
            // Mise à jour groupée pour améliorer les performances avec une garde
            UpdateGuard updateGuard(this);

            // Préallocation pour les nouveaux nœuds
            int numToAdd = targetCount - currentSongCount;

            // Ajout des chansons en une seule boucle
            for (int i = 0; i < numToAdd; i++) {
                // Obtention d'une chanson aléatoire
                XNode* randomChild = Core->TvList->NRSong->RandomChild;
                if (!randomChild) continue;

                XNode* newSong = PNR->Link(randomChild);
                if (!newSong) continue;

                // Ajout à l'arborescence
                Items->AddChildObject(PTNR, newSong->FileName, newSong);

                // Analyse de la chanson
                if (Core->Mix && Core->Mix->Wave) {
                    try {
                        Core->Mix->Wave->Scan(newSong);
                    }
                    catch (...) {
                        // Ignorer les exceptions potentielles lors de l'analyse
                    }
                }
            }

            // Finalisation de l'affichage
            Refresh();
            if (PTNR) {
                PTNR->Expand(false);
            }

            // Masquage de la barre de défilement horizontale
            ShowScrollBar(Handle, SB_HORZ, false);

            // Mise à jour de la prochaine chanson à jouer si nécessaire
            if (Core->Mix && !Core->Mix->Next->Node) {
                Core->Mix->Next->Node = GetNext();
            }
        }
        catch (...) {
            // Les exceptions sont gérées par UpdateGuard qui appelle EndUpdate
        }
    }
    catch (...) {
        // Capture des exceptions pour éviter les problèmes
    }
}

void __fastcall XTvNext::Shuffle(void) {
    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL));
    
    // Référence directe à l'instance courante pour éviter les indirections
    XTvNext *tv = Core->TvNext;
    if (!tv) return;
    
    // Vérification rapide pour éviter le traitement inutile
    if (tv->CountSong < 2) return;
    
    // Détermination du nœud de départ pour le mélange
    TTreeNode *nodestart = Selected;
    
    // Si aucun nœud n'est sélectionné, utiliser le premier nœud
    if (!nodestart) {
        nodestart = tv->Items->GetFirstNode();
        if (!nodestart) return;  // Aucun nœud disponible
    }
    
    // Si le nœud sélectionné est une chanson, utiliser son parent
    XNode* nodeData = static_cast<XNode*>(nodestart->Data);
    if (nodeData && nodeData->IsSong) {
        nodestart = nodestart->Parent;
        if (!nodestart) return;  // Nœud parent invalide
    }
    
    try {
        // Mise à jour groupée pour améliorer les performances
        tv->Updating();
        
        // Mélange récursif à partir du nœud de départ
        RecurseShuffle(nodestart);
        
        // Fin de la mise à jour
        tv->Updated();
    }
    catch (...) {
        // Capture des exceptions pour éviter les problèmes
        tv->Updated();
    }
}

void __fastcall XTvNext::RecurseShuffle(TTreeNode *rstart) {
    // Vérification préalable pour éviter le traitement inutile
    if (!rstart) return;
    
    int count = rstart->Count;
    if (count <= 1) return;  // Rien à mélanger avec 0 ou 1 élément
    
    try {
        Items->BeginUpdate();
        
        // Tableau temporaire pour stocker les nœuds à traiter récursivement
        // Évite de modifier l'arbre pendant qu'on le parcourt
        std::vector<TTreeNode*> folderNodes;
        
        // Premier passage: identifier les dossiers pour le traitement récursif
        for (int i = 0; i < count; i++) {
            TTreeNode* node = rstart->Item[i];
            if (!node) continue;
            
            XNode* nodeData = static_cast<XNode*>(node->Data);
            
            if (nodeData && nodeData->IsFolder) {
                folderNodes.push_back(node);
            }
        }
        
        // Traitement récursif des dossiers identifiés
        for (size_t i = 0; i < folderNodes.size(); i++) {
            RecurseShuffle(folderNodes[i]);
        }
        
        // Mélange des nœuds enfants
        for (int i = 0; i < count; i++) {
            TTreeNode* node = rstart->Item[i];
            if (!node) continue;
            
            int r = (int)(Random() * count);
            if (r >= count) r = count - 1;
            if (r < 0) r = 0;
            
            TTreeNode* nodedest = rstart->Item[r];
            if (!nodedest) continue;
            
            // Déplacement du nœud
            node->MoveTo(nodedest, naInsert);
        }
        
        Items->EndUpdate();
    }
    catch (...) {
        // S'assurer que EndUpdate est appelé même en cas d'exception
        Items->EndUpdate();
    }
}

unsigned __fastcall XTvNext::GetCountSong(void) {
    // Optimisation: accès direct à la propriété avec vérification de sécurité
    return PNR ? PNR->CountSongAll : 0;
}

void __fastcall XTvNext::SetIcon(TTreeNode *tn, int a, int b, int c) {

    // Vérification que le nœud existe
    if (!tn) return;

    // Optimisation: utilisation d'une condition ternaire pour simplifier le code
    bool isMinimized = (Parent && Parent->Name == "PanelMin");

    // Assignation directe des valeurs selon le mode
    tn->ImageIndex = isMinimized ? -1 : a;
    tn->ExpandedImageIndex = isMinimized ? -1 : b;
    tn->SelectedIndex = isMinimized ? -1 : c;
}

// Nouvelle méthode pour plier/déplier un nœud
void __fastcall XTvNext::ToggleNode(TTreeNode *node) {
    if (!node) return;

    XNode *nodeData = static_cast<XNode*>(node->Data);
    if (!nodeData) return;

    try {
        // Si le nœud n'est pas peuplé, le peupler d'abord
        if (!nodeData->Populated && nodeData->HaveChild) {
            // Créer un thread pour peupler le nœud
            // Note: ThreadPopulate déplie automatiquement le nœud dans sa méthode Done
            ThreadPopulate *thread = new ThreadPopulate(this, node, false);

            // Attendre que le thread termine (synchrone) avec un timeout
            int timeout = 0;
            const int MAX_TIMEOUT = 5000; // 5 secondes maximum

            while (!nodeData->Populated && timeout < MAX_TIMEOUT) {
                Application->ProcessMessages();
                Sleep(10);
                timeout += 10;
            }

            // Si le timeout est atteint, forcer le nœud à être considéré comme peuplé
            if (timeout >= MAX_TIMEOUT && !nodeData->Populated) {
                nodeData->Populated = true;
            }
        } else {
            // Si le nœud est déjà peuplé, inverser son état d'expansion
            if (node->Expanded) {
                node->Collapse(false);
            } else {
                node->Expand(false);
            }
        }
    }
    catch (...) {
        // Ignorer les exceptions lors du peuplement ou du pliage/dépliage
    }
}

// -----------------------------------------------------
//          Event
// -----------------------------------------------------
void __fastcall XTvNext::ODblClick(TObject *Sender) {
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
        // Vérifications préliminaires optimisées
        TTreeNode* selectedNode = Selected;
        if (!selectedNode) {
            return;
        }

        XNode* cn = static_cast<XNode*>(selectedNode->Data);
        if (!cn) {
            return;
        }

        // Traitement selon le type de nœud
        switch (cn->NodeType) {
            case NODETYPE::ROOT:
            case NODETYPE::FOLDER:
                // Utiliser la nouvelle méthode ToggleNode pour plier/déplier le nœud
                //ToggleNode(selectedNode);
                return;

            case NODETYPE::SONG: {
                // Vérification des pointeurs avant utilisation
                if (!Core || !Core->Mix || !Core->TvPast || !PTNTool) {
                    return;
                }

                // Vérification si une chanson est déjà en cours de lecture
                if (Core->Mix->IsMixing) {
                    // Attendre la fin du mixage en cours avant de lancer une nouvelle chanson
                    Core->Mix->Current->FadeOutNow(Core->Settings->FadeOutDuration / 2);
                }

                XNode* newnode = NULL;

                try {
                    // Traitement optimisé pour les chansons
                    // Copie vers l'historique (Past)
                    newnode = PTNTool->CopyTo(Core->TvPast, selectedNode, Core->TvPast->TNR);
                    if (!newnode) {
                        return;
                    }

                    // Expansion de l'historique
                    Core->TvPast->TNR->Expand(false);

                    // Lecture de la chanson
                    Core->Mix->NodeNow(newnode);

                    // Suppression de la chanson de la liste "Next"
                    // Utilisation d'une variable temporaire pour éviter les problèmes de récursivité
                    TTreeNode* nodeToRemove = selectedNode;

                    try {
                        // Suppression du nœud
                        Items->BeginUpdate();
                        Remove(nodeToRemove);
                        Items->EndUpdate();
                    }
                    catch (...) {
                        // S'assurer que EndUpdate est appelé même en cas d'exception
                        Items->EndUpdate();
                    }

                    // Mise à jour de la prochaine chanson
                    Core->Mix->Next->Node = GetNext();
                }
                catch (...) {
                    // En cas d'erreur, s'assurer que les ressources sont libérées
                    if (newnode) {
                        delete newnode;
                    }
                }
                return;
            }

            default:
                return;
        }
    }
    catch (...) {
        // Capture des exceptions pour éviter les problèmes
    }
}

void __fastcall XTvNext::OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw) {
    // Vérification des pointeurs avant utilisation
    if (!Sender || !Node) return;
    
    XNode *cn = static_cast<XNode*>(Node->Data);
    if (cn == NULL) return;

    // Configuration commune pour tous les types
    Sender->Canvas->Brush->Color = clBlack;

    // Utilisation d'un switch pour une meilleure performance avec des types énumérés
    switch (cn->NodeType) {
        case NODETYPE::ROOT:
            SetIcon(Node, 0, 0, 0);
            return;

        case NODETYPE::SONG: {
            // Détermination des icônes et couleurs en fonction des compteurs
            int baseIcon, selectedIcon;
            
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
                baseIcon = 7;
                selectedIcon = 6;
            }
            
            // Application des icônes pour les éléments sans playlist
            if (cn->CountPlayList > 0) {
                SetIcon(Node, baseIcon + 2, baseIcon + 2, 10); // Ajustement pour playlist
            } else {
                SetIcon(Node, baseIcon, baseIcon, selectedIcon);
            }
            return;
        }
        
        case NODETYPE::FOLDER:
            // Simplification pour les dossiers (les deux branches sont identiques)
            SetIcon(Node, 13, 13, 13);
            return;
            
        default:
            return;
    }
}

// -----------------------------------------------------
//          DRAG DROP
// -----------------------------------------------------

void __fastcall XTvNext::OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
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

void __fastcall XTvNext::ODragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept) {
    // Vérification des pointeurs avant utilisation
    if (!Source) {
        Accept = false;
        return;
    }
    
    // Approche simplifiée pour une meilleure compatibilité
    UnicodeString sourceClassName = Source->ClassName();
    
    // Accepter les sources valides
    Accept = (sourceClassName == "XTvList") ||
             (sourceClassName == "XTvNext") ||
             (sourceClassName == "XTvPast") ||
             (sourceClassName == "XTvPlayList");
}

void __fastcall XTvNext::ODragDrop(TObject *Sender, TObject *Source, int X, int Y) {
    // Vérification des pointeurs avant utilisation
    if (!Source || !PTNTool) return;

    // Détermination de la source
    int src = 0; // Par défaut, considérer comme une copie
    bool isSourceTVList = false;

    // Vérification simple du type de source
    UnicodeString sourceClassName = Source->ClassName();
    if (sourceClassName == "XTvNext") {
        src = 1; // Déplacement pour la même source
    } else if (sourceClassName == "XTvList") {
        isSourceTVList = true;
    }

    // Cast de la source en TTreeView
    TTreeView *tvs = static_cast<TTreeView*>(Source);
    if (!tvs || tvs->SelectionCount == 0) return;

    // Détermination de la destination
    TTreeNode *tnd = GetNodeAt(X, Y);

    try {
        // Début de la mise à jour groupée
        Items->BeginUpdate();

        try {
            // Traitement de tous les nœuds sélectionnés
            for (unsigned i = 0; i < tvs->SelectionCount; i++) {
                TTreeNode *tns = tvs->Selections[i];
                if (!tns) continue;

                // Si la source est TVList ou un autre TreeView, nous devons nous assurer que le nœud est peuplé
                // mais sans le déplier visuellement
                XNode *nodeData = static_cast<XNode*>(tns->Data);
                if (nodeData && !nodeData->IsSong && !nodeData->Populated && nodeData->HaveChild) {
                    try {
                        // Utiliser la méthode RecursePopulate pour peupler le nœud
                        // Pour TVList, ne pas déplier les nœuds
                        PTNTool->RecursePopulate(tvs, tns, !isSourceTVList);
                    }
                    catch (...) {
                        // En cas d'erreur, marquer le nœud comme peuplé
                        // pour éviter de tenter de le peupler à nouveau
                        nodeData->Populated = true;
                    }
                }

                // Destination: racine ou nœud spécifique
                if (!tnd) {
                    // Destination: racine
                    if (src == 0) {
                        PTNTool->CopyTo(this, tns, TNR);
                    } else {
                        PTNTool->MoveTo(this, tns, TNR);
                    }
                } else {
                    // Destination: nœud spécifique
                    if (src == 0) {
                        PTNTool->CopyTo(this, tns, tnd);
                    } else {
                        PTNTool->MoveTo(this, tns, tnd);
                    }
                }
            }

            // Expansion du nœud racine pour montrer les nouveaux éléments
            if (PTNR) {
                PTNR->Expand(false);
            }

            // Mise à jour de la prochaine chanson à jouer
            if (Core && Core->Mix) {
                Core->Mix->Next->Node = GetNext();
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

void __fastcall XTvNext::OEndDrag(TObject *Sender, TObject *Target, int X, int Y) {
    // Vérification des pointeurs avant utilisation
    if (!Target || !Core || !Core->Settings) return;
    
    // Optimisation: mise à jour conditionnelle uniquement si nécessaire
    try {
        // Mise à jour de l'interface après un glisser-déposer réussi
        Update();
        
        // Vérification du nombre de chansons actuel
        unsigned currentSongCount = CountSong;
        int targetCount = Core->Settings->AutoSelect;
        
        // Remplissage si nécessaire
        if (currentSongCount < targetCount) {
            Fill();
        }
    }
    catch (...) {
        // Ignorer les exceptions potentielles
    }
}

void __fastcall XTvNext::OResize(TObject *Sender) {
    // Optimisation: masquage de la barre de défilement horizontale lors du redimensionnement
    ShowScrollBar(Handle, SB_HORZ, false);

    // Ajustement de la largeur des colonnes si nécessaire
    if (PTNR && PTNR->IsVisible) {
        // Expansion du nœud racine pour assurer la visibilité des éléments
        PTNR->Expand(false);
    }
}

void __fastcall XTvNext::OExpanding(TObject *Sender, TTreeNode *Node, bool &AllowExpansion) {
    // Toujours autoriser l'expansion
    AllowExpansion = true;

    // Vérification des pointeurs avant utilisation
    if (!Node || !PTNTool) return;

    XNode *cn = static_cast<XNode*>(Node->Data);
    if (!cn) return;

    // Si le nœud n'est pas peuplé mais a des enfants, le peupler
    if (!cn->Populated && cn->HaveChild) {
        try {
            // Créer un thread pour peupler le nœud
            // Le thread se libérera automatiquement grâce à FreeOnTerminate = true
            ThreadPopulate *thread = new ThreadPopulate(this, Node, true);

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
        }
        catch (...) {
            // En cas d'erreur, marquer le nœud comme peuplé pour éviter de bloquer
            cn->Populated = true;
        }
    }
}