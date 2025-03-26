//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassTreeViewList.h"

//---------------------------------------------------------------------------
//#pragma package(smart_init)

// -----------------------------------------------------
// CLASS XTvList
// -----------------------------------------------------
__fastcall XTvList::XTvList(TComponent *owner, XCore *core) : TTreeView(owner) {
    // Initialisation optimisée des propriétés en utilisant des listes d'initialisation
    // et en regroupant les opérations similaires
    
    // Initialisation des propriétés de base
    Parent           = static_cast<TPanel*>(owner);
    Core             = core;
    PViewFlat        = VIEWMODE::ARBO;
    PViewAz          = AZMODE::NONE;
    PNR              = NULL;
    
    // Initialisation des pointeurs à NULL
    PNRSong          = NULL;
    PNRArbo          = NULL;
    PNRFlat          = NULL;
    PNRArboAz        = NULL;
    PNRFlatAz        = NULL;
    PTNTool          = NULL;

    // Configuration optimisée de l'apparence en un bloc
    Images           = Core->Intf->TvIcones;
    Align            = alClient;
    ShowLines        = false;
    ShowButtons      = false;
    ReadOnly         = true;
    DoubleBuffered   = true;
    SortType         = Comctrls::stNone;
    
    // Configuration de la sélection multiple
    MultiSelect      = true;
    MultiSelectStyle = TMultiSelectStyle() << msShiftSelect << msControlSelect;

    // Configuration de la police en un bloc
    StyleElements    = StyleElements >> seFont;
    Font->Color      = Core->Settings->ColorList;
    Font->Size       = Core->FontSize;

    // Assignation des gestionnaires d'événements en un bloc
    OnCustomDrawItem = OCustomDrawItem;
    OnDblClick       = ODblClick;
    OnMouseDown      = OMouseDown;
    OnStartDrag      = OStartDrag;
    OnResize         = OResize;

    try {
        // Initialisation des outils
        PTNTool = new XTNodeTool();
        
        // Création optimisée des nœuds racines avec des types spécifiques
        // Structure pour initialiser les nœuds de manière uniforme
        struct NodeInfo {
            XNode** nodePtr;
            const wchar_t* name;
        };
        
        // Tableau d'initialisation pour réduire la duplication de code
        NodeInfo rootNodes[] = {
            { &PNRSong,   L"Song"      },
            { &PNRArbo,   L"Arbo"      },
            { &PNRFlat,   L"Flat"      },
            { &PNRArboAz, L"Arbo A..Z" },
            { &PNRFlatAz, L"Flat A..Z" }
        };
        
        // Création des nœuds en une seule boucle
        for (int i = 0; i < sizeof(rootNodes)/sizeof(rootNodes[0]); i++) {
            *(rootNodes[i].nodePtr) = new XNode(NULL, rootNodes[i].name, NODETYPE::ROOT);
        }
        
        // Initialisation des données
        Reset();
    }
    catch (...) {
        // Nettoyage en cas d'exception
        // Tableau des pointeurs à libérer
        XNode* nodesToDelete[] = {
            PNRFlatAz,
            PNRArboAz,
            PNRFlat,
            PNRArbo,
            PNRSong
        };
        
        // Libération des nœuds en une seule boucle
        for (int i = 0; i < sizeof(nodesToDelete)/sizeof(nodesToDelete[0]); i++) {
            if (nodesToDelete[i]) {
                delete nodesToDelete[i];
            }
        }
        
        // Libération de l'outil
        if (PTNTool) {
            delete PTNTool;
        }
        
        // Relancer l'exception
        throw;
    }
}

__fastcall XTvList::~XTvList(void) {
    // Optimisation du destructeur avec une approche plus structurée
    // Utilisation d'un tableau pour éviter la duplication de code
    
    // Tableau des pointeurs à libérer
    XNode* nodesToDelete[] = {
        PNRFlatAz,
        PNRArboAz,
        PNRFlat,
        PNRArbo,
        PNRSong
    };
    
    // Libération des nœuds en une seule boucle
    for (int i = 0; i < sizeof(nodesToDelete)/sizeof(nodesToDelete[0]); i++) {
        if (nodesToDelete[i]) {
            delete nodesToDelete[i];
            nodesToDelete[i] = NULL;
        }
    }
    
    // Libération de l'outil
    if (PTNTool) {
        delete PTNTool;
        PTNTool = NULL;
    }
}

void __fastcall XTvList::Reset(void) {
    // Réinitialisation optimisée des nœuds racines
    // Utilisation de références pour simplifier le code et améliorer la lisibilité
    
    // Vérification des pointeurs avant utilisation
    if (!NRSong || !NRArbo || !NRFlat || !NRArboAz || !NRFlatAz) {
        return;
    }
    
    // Réinitialisation du nœud Song
    NRSong->Clear();
    NRSong->Path = L"Song";

    // Réinitialisation du nœud Arbo
    NRArbo->Clear();
    NRArbo->Path = Core->Settings->MusicPath;

    // Réinitialisation du nœud Flat
    NRFlat->Clear();
    NRFlat->Path = L"Flat";

    // Réinitialisation des nœuds avec tri alphabétique
    // Optimisation: traitement similaire pour les deux nœuds
    struct {
        XNode* node;
        const wchar_t* path;
    } azNodes[] = {
        { NRFlatAz, L"Flat A..Z" },
        { NRArboAz, L"Arbo A..Z" }
    };
    
    // Traitement en lot pour réduire la duplication de code
    for (int i = 0; i < 2; i++) {
        XNode* node = azNodes[i].node;
        node->Clear();
        node->Path = azNodes[i].path;
        node->Add(L"0..9", NODETYPE::FOLDER);
        
        // Ajout des dossiers A-Z en une seule passe
        for (char a = 'A'; a <= 'Z'; a++) {
            node->Add(UnicodeString(a), NODETYPE::FOLDER);
        }
    }
    
    // Réinitialisation de l'arbre
    ResetTNR();
}

void __fastcall XTvList::ResetTNR(void) {
    Items->Clear();
    PTNR = Items->AddObjectFirst(NULL,L"List", PNR);
}

void __fastcall XTvList::SetViewFlat(int flat) {
    PViewFlat = flat;
    if (Core && Core->Intf) {
        Core->Intf->ArboButton = flat;
    }
    SetNodeRoot();
}

void __fastcall XTvList::SetViewAz(int az) {
    PViewAz = az;
    if (Core && Core->Intf) {
        Core->Intf->AzButton = az;
    }
    SetNodeRoot();
}

void __fastcall XTvList::SetNodeRoot(void) {
    // Vérification des pointeurs avant utilisation
    if (!NRArbo || !NRArboAz || !NRFlat || !NRFlatAz) {
        return;
    }
    
    // Optimisation: utilisation d'une table de décision pour un accès direct
    // Tableau statique pour éviter les allocations répétées
    static XNode* nodeRootTable[2][2] = {
        // AZMODE::NONE      AZMODE::AZ
        { NULL,              NULL },            // Initialisation
        { NULL,              NULL }             // Sera rempli au premier appel
    };
    
    // Initialisation paresseuse du tableau (une seule fois)
    if (!nodeRootTable[0][0]) {
        // VIEWMODE::ARBO
        nodeRootTable[0][0] = NRArbo;    // AZMODE::NONE
        nodeRootTable[0][1] = NRArboAz;  // AZMODE::AZ
        
        // VIEWMODE::FLAT
        nodeRootTable[1][0] = NRFlat;    // AZMODE::NONE
        nodeRootTable[1][1] = NRFlatAz;  // AZMODE::AZ
    }
    
    // Accès direct au nœud racine approprié via la table
    NR = nodeRootTable[PViewFlat == VIEWMODE::FLAT ? 1 : 0][PViewAz == AZMODE::AZ ? 1 : 0];
}

void __fastcall XTvList::SetNR(XNode *noderoot) {
    if (!noderoot) return;
    
    PNR = noderoot;
    ResetTNR();
    
    if (!PTNR) return;
    
    PTNR->Text = PNR->Path;
    PTNR->Data = PNR;
    PNR->ResetPopulated();
    PNR->Sort();
    
    try {
        // Création du thread avec FreeOnTerminate = true
        ThreadPopulate *thread = new ThreadPopulate(this, PTNR, false);
    }
    catch (...) {
        // Gestion des exceptions lors de la création du thread
        // Pas besoin de libérer le thread car il n'a pas été créé
    }
}

void __fastcall XTvList::RollArboFlat(void) {
    if (PViewFlat == VIEWMODE::ARBO) ViewFlat = VIEWMODE::FLAT;
    else if (PViewFlat == VIEWMODE::FLAT) ViewFlat = VIEWMODE::ARBO;
}

void __fastcall XTvList::RollAz(void) {
    if (PViewAz == AZMODE::NONE) ViewAz = AZMODE::AZ;
    else if (PViewAz == AZMODE::AZ) ViewAz = AZMODE::NONE;
}


void __fastcall XTvList::AlphaFind(char letter) {
    // Mode sans tri alphabétique
    if (PViewAz == AZMODE::NONE) {
        // Vérification que PTNR existe
        if (!PTNR) return;
        
        // Optimisation: traitement direct pour le cas spécial '#'
        if (letter == '#') {
            TTreeNode *firstChild = PTNR->getFirstChild();
            if (firstChild) {
                TopItem = firstChild;
            }
            return;
        }
        
        // Recherche optimisée avec vérification préalable
        TTreeNode *firstChild = PTNR->getFirstChild();
        if (!firstChild) return;
        
        // Parcours linéaire optimisé
        for (TTreeNode *n = firstChild; n != NULL; n = n->GetNext()) {
            // Vérification plus efficace de la longueur
            UnicodeString text = n->Text;
            if (!text.IsEmpty() && text[1] == letter) {
                TopItem = n;
                return;
            }
        }
        return;
    }
    
    // Mode avec tri alphabétique (A-Z)
    // Vérification des pointeurs avant utilisation
    if (!NRArboAz || !NRFlatAz) return;
    
    // Sélection directe du nœud racine approprié
    XNode* targetRoot = (PViewFlat == VIEWMODE::ARBO) ? NRArboAz : NRFlatAz;
    
    // Calcul optimisé de l'index
    int childIndex;
    if (letter == '#') {
        childIndex = 0; // Index pour "0..9"
    } else if (letter >= 'A' && letter <= 'Z') {
        childIndex = letter - 'A' + 1; // +1 car "0..9" est à l'index 0
    } else {
        return; // Caractère non supporté
    }
    
    // Vérification de la validité de l'index et accès direct
    if (childIndex >= 0 && childIndex < targetRoot->CountChild) {
        NR = targetRoot->ChildAt(childIndex);
    }
}

void __fastcall XTvList::ODblClick(TObject *Sender) {
    // Vérifications préliminaires optimisées en une seule condition
    TTreeNode* selectedNode = Selected;
    if (!selectedNode) return;
    
    XNode *cn = static_cast<XNode*>(selectedNode->Data);
    if (!cn) return;
    
    // Utilisation d'un switch pour une meilleure performance
    switch (cn->NodeType) {
        case NODETYPE::ROOT:
        case NODETYPE::FOLDER:
            // Optimisation: combinaison des conditions pour réduire les vérifications
            if (!cn->Populated && cn->HaveChild) {
                cn->Sort();
                try {
                    // Création du thread avec FreeOnTerminate = true
                    ThreadPopulate *thread = new ThreadPopulate(this, selectedNode, false);
                }
                catch (...) {
                    // Gestion des exceptions lors de la création du thread
                    // Pas besoin de libérer le thread car il n'a pas été créé
                }
            }
            break;
            
        case NODETYPE::SONG:
            // Vérification des pointeurs avant utilisation
            if (!Core || !Core->Mix || !Core->Mix->Wave || !Core->TvPast || !PTNTool) break;
            
            // Traitement optimisé pour les chansons
            Core->Mix->Wave->Scan(cn);
            try {
                XNode *newnode = PTNTool->CopyTo(Core->TvPast, selectedNode, Core->TvPast->TNR);
                if (newnode) {
                    Core->Mix->NodeNow(newnode);
                }
            }
            catch (...) {
                // Gestion des exceptions lors de la copie
            }
            break;
    }
}

void __fastcall XTvList::OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw) {
    if (!Sender || !Node) return;
    
    XNode *cn = (XNode*) Node->Data;
    if (cn == NULL) return;
    
    // Optimisation: configuration commune pour tous les types
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
            } else if (cn->CountPlay > 0) {    // Vert
                Sender->Canvas->Font->Color = clLime;
                baseIcon = 7;
            } else {                           // Jaune (CountPlay == 0)
                Sender->Canvas->Font->Color = clYellow;
                baseIcon = 5;
            }
            
            // Détermination de l'icône sélectionnée
            selectedIcon = cn->CountPlayList > 0 ? 10 : 6;
            
            // Application des icônes en fonction de la présence dans une playlist
            if (cn->CountPlayList > 0) {
                SetIcon(Node, baseIcon + 4, baseIcon + 4, selectedIcon);
            } else {
                SetIcon(Node, baseIcon, baseIcon, selectedIcon);
            }
            return;
        }
        
        case NODETYPE::FOLDER: {
            // Optimisation pour les dossiers - calcul unique de l'index d'icône
            int iconIndex = cn->CountFolder > 0 ? 3 : 1;
            int selectedIndex = cn->CountFolder > 0 ? 4 : 2;
            SetIcon(Node, iconIndex, iconIndex, selectedIndex);
            return;
        }
        
        default:
            return;
    }
}

void __fastcall XTvList::SetIcon(TTreeNode *tn, int a, int b, int c) {
    if (!tn) return;
    
    tn->ImageIndex = a;
    tn->ExpandedImageIndex = b;
    tn->SelectedIndex = c;
}

// -----------------------------------------------------
//          DRAG DROP
// -----------------------------------------------------

void __fastcall XTvList::OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
    // Optimisation: vérification rapide avant le cast et les opérations coûteuses
    if (Button != mbLeft || !Sender) return;
    
    // Éviter le dynamic_cast coûteux puisque nous savons que Sender est un TTreeView
    // (cette méthode est liée à l'événement OnMouseDown de this, qui est un TTreeView)
    TTreeView *tv = static_cast<TTreeView*>(Sender);
    
    // Vérification directe si un nœud existe à la position du clic
    TTreeNode* node = tv->GetNodeAt(X, Y);
    if (node) {
        tv->BeginDrag(false, -1);
    }
}

void __fastcall XTvList::OStartDrag(TObject *Sender, TDragObject *&DragObject) {
    // Cette méthode est vide mais pourrait être implémentée pour personnaliser le comportement de glisser-déposer
}

void __fastcall XTvList::OResize(TObject *Sender) {
    ShowScrollBar(Handle, SB_HORZ, false);
}