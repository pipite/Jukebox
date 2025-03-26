//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassTreeNodeTool.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   XTNodeTool
// ---------------------------------------------------------------------------
__fastcall XTNodeTool::XTNodeTool(void) {
}

__fastcall XTNodeTool::~XTNodeTool(void) {
}

// Fonction auxiliaire pour ajouter visuellement tous les enfants d'un nœud à l'arbre
void __fastcall XTNodeTool::AddChildrenToTree(TTreeView *tv, TTreeNode *parentTn, XNode *parentNode) {
    if (!tv || !parentTn || !parentNode) return;

    // Vérification supplémentaire pour s'assurer que parentNode->Child existe
    if (!parentNode->Child) {
        // Si Child n'existe pas, marquer le nœud comme peuplé et retourner
        parentNode->Populated = true;
        return;
    }

    try {
        // Parcourir tous les enfants du nœud parent
        for (int i = 0; i < parentNode->Child->Count; i++) {
            try {
                XNode *childNode = parentNode->ChildAt(i);
                if (!childNode) continue;

                // Ajouter l'enfant à l'arbre
                TTreeNode *childTn = NULL;
                try {
                    childTn = tv->Items->AddChildObject(parentTn, childNode->FileName, childNode);

                    // Si l'enfant est un dossier, ajouter récursivement ses enfants
                    if (childTn && !childNode->IsSong && childNode->HaveChild) {
                        // Marquer le dossier enfant comme peuplé pour éviter le double peuplement
                        childNode->Populated = true;
                        AddChildrenToTree(tv, childTn, childNode);
                    }
                }
                catch (...) {
                    // En cas d'erreur lors de l'ajout de l'enfant, continuer avec le suivant
                    // Mais s'assurer que le nœud est marqué comme peuplé
                    childNode->Populated = true;
                }
            }
            catch (...) {
                // En cas d'erreur avec un enfant spécifique, continuer avec le suivant
            }
        }
    }
    catch (...) {
        // En cas d'erreur globale, marquer le nœud comme peuplé et retourner
        parentNode->Populated = true;
    }
}

void __fastcall XTNodeTool::MoveTo(TTreeView *tvd, TTreeNode *tns, TTreeNode *tnd) {
    XNode *ns, *nd;

    if (tvd == NULL) return;
    if (tns == NULL) return;
    ns = (XNode*) tns->Data;
    if (ns == NULL) return;
    if (ns->IsRoot) return;

    // Si pas de TreeNodeDest Alors prendre le TreeNodeRoot du TreeViewDest
    if (tnd == NULL) tnd = tvd->Items->GetFirstNode();
    if (tnd == NULL) return; // Protection supplémentaire
    nd = (XNode*)tnd->Data;
    if (nd == NULL) return; // Protection supplémentaire

    // Verifier que le nodedest ne soit pas enfant du node source!!
    if (nd->HasAsParent(ns)) return;

    tvd->Items->BeginUpdate();
    try {
        // Song -> Song     Insertion au dessus
        if ( ns->IsSong && nd->IsSong ) {
            tns->MoveTo(tnd, naInsert);
        // Folder -> Song   Insertion au dessus
        } else if ( !ns->IsSong && nd->IsSong ) {
            tns->MoveTo(tnd, naInsert);
        // Song -> Folder ou Root  Ajouter un child au Folder
        } else if ( ns->IsSong && !nd->IsSong ) {
            tns->MoveTo(tnd, naAddChild);
        // Folder -> Root   Ajouter un child au Root
        } else if ( !ns->IsSong && nd->IsRoot ) {
            tns->MoveTo(tnd, naAddChild);
        // Folder -> Folder Ajouter comme enfant au lieu d'insérer au-dessus
        } else {
            tns->MoveTo(tnd, naAddChild);
        }
        ns->MoveTo(nd);
    }
    catch (...) {
        // Capture des exceptions pour éviter les problèmes
    }
    tvd->Items->EndUpdate();
}

// Méthode originale pour maintenir la compatibilité
void __fastcall XTNodeTool::RecursePopulate(TTreeView *tv, TTreeNode *tn) {
    // Par défaut, nous ne déplions pas les nœuds pour TVList
    bool expandNodes = (tv->ClassName() != "XTvList");
    RecursePopulate(tv, tn, expandNodes);
}

// Nouvelle méthode avec paramètre pour contrôler l'expansion des nœuds
void __fastcall XTNodeTool::RecursePopulate(TTreeView *tv, TTreeNode *tn, bool expandNodes) {
    if (!tv || !tn) return;

    XNode *node = static_cast<XNode*>(tn->Data);
    if (!node) return;

    // Si le nœud n'est pas peuplé mais a des enfants, le peupler
    if (!node->Populated && node->HaveChild) {
        try {
            // Créer un thread pour peupler le nœud
            // Le thread se libérera automatiquement grâce à FreeOnTerminate = true
            ThreadPopulate *thread = new ThreadPopulate(tv, tn, false);

            // Attendre que le thread termine (synchrone) avec un timeout
            int timeout = 0;
            const int MAX_TIMEOUT = 5000; // 5 secondes maximum

            while (!node->Populated && timeout < MAX_TIMEOUT) {
                Application->ProcessMessages();
                Sleep(10);
                timeout += 10;
            }

            // Si le timeout est atteint, forcer le nœud à être considéré comme peuplé
            if (timeout >= MAX_TIMEOUT && !node->Populated) {
                node->Populated = true;
            }

            // Si expandNodes est false, nous devons replier le nœud qui a été déplié par ThreadPopulate
            if (!expandNodes && tn->Expanded) {
                tn->Collapse(true);
            }
        }
        catch (...) {
            // En cas d'erreur, marquer le nœud comme peuplé pour éviter de bloquer
            node->Populated = true;
        }

        // Maintenant que le nœud est peuplé, peupler récursivement ses enfants
        // Vérifier que tn est toujours valide et a des enfants
        if (tn && tn->Count > 0) {
            for (int i = 0; i < tn->Count; i++) {
                if (tn->Item[i]) {
                    RecursePopulate(tv, tn->Item[i], expandNodes);
                }
            }
        }
    }

    // Si expandNodes est true et que le nœud est peuplé, déplier le nœud
    // Cela permet de plier/déplier les dossiers dans TvNext
    if (expandNodes && node->Populated && tv->ClassName() == "XTvNext") {
        try {
            tn->Expanded = !tn->Expanded;
        }
        catch (...) {
            // Ignorer les exceptions lors du pliage/dépliage
        }
    }
}

XNode* __fastcall XTNodeTool::CopyTo(TTreeView *tvd, TTreeNode *tns, TTreeNode *tnd) {
    XNode *ns, *nd, *newn = NULL;
    TTreeNode *newtn = NULL;

    if (tvd == NULL) return NULL;
    if (tns == NULL) return NULL;
    ns = (XNode*) tns->Data;
    if (ns == NULL) return NULL;

    // Si pas de TreeNodeDest Alors prendre le TreeNodeRoot du TreeViewDest
    if (tnd == NULL) tnd = tvd->Items->GetFirstNode();
    if (tnd == NULL) return NULL; // Protection supplémentaire
    nd = (XNode*)tnd->Data;
    if (nd == NULL) return NULL; // Protection supplémentaire
    
    // Vérifier si la source est TVList
    TTreeView *tvs = static_cast<TTreeView*>(tns->TreeView);
    bool isSourceTVList = (tvs && tvs->ClassName() == "XTvList");

    // Si le nœud source est un dossier, s'assurer qu'il est peuplé avant de le copier
    if (!ns->IsSong && !ns->Populated && ns->HaveChild) {
        // Peupler récursivement le nœud source et tous ses enfants
        // Pour TVList, ne pas déplier les nœuds
        RecursePopulate(tvs, tns, !isSourceTVList);
    }

    try {
        // Copier le nœud source vers le nœud destination
        newn = ns->CopyTo(nd);
        if (newn == NULL) return NULL; // Protection contre les erreurs de copie
        if (newn->IsRoot) newn->NodeType = NODETYPE::FOLDER;

        tvd->Items->BeginUpdate();
        try {
            // Song -> Song     Insertion au dessus
            if ( ns->IsSong && nd->IsSong ) {
                newtn = tvd->Items->InsertObject(tnd, tns->Text, newn);
            // Folder -> Song   Insertion au dessus
            } else if ( !ns->IsSong && nd->IsSong ) {
                newtn = tvd->Items->InsertObject(tnd, tns->Text, newn);
            // Song -> Folder ou Root  Ajouter un child au Folder
            } else if ( ns->IsSong && !nd->IsSong ) {
                // Toujours ajouter l'élément, même si le nœud n'est pas peuplé
                newtn = tvd->Items->AddChildObject(tnd, tns->Text, newn);
            // Folder -> Root   Ajouter un child au Root
            } else if ( !ns->IsSong && nd->IsRoot ) {
                newtn = tvd->Items->AddChildObject(tnd, tns->Text, newn);
            // Folder -> Folder Ajouter comme enfant au lieu d'insérer au-dessus
            } else {
                newtn = tvd->Items->AddChildObject(tnd, tns->Text, newn);
            }
            
            // Vérification que newtn a été créé correctement
            if (newtn == NULL) {
                // Si l'ajout a échoué, nettoyer le nœud créé pour éviter les fuites de mémoire
                delete newn;
                return NULL;
            }

            // Si le nœud copié est un dossier, ajouter visuellement tous ses enfants à l'arbre
            if (!newn->IsSong && newn->HaveChild) {
                try {
                    // Marquer le nouveau nœud comme peuplé AVANT d'ajouter ses enfants
                    // Cela empêchera le double peuplement lorsque le nœud sera ouvert
                    newn->Populated = true;

                    // Ajouter récursivement tous les enfants à l'arbre
                    AddChildrenToTree(tvd, newtn, newn);
                }
                catch (...) {
                    // En cas d'erreur, s'assurer que le nœud est marqué comme peuplé
                    // pour éviter de tenter de le peupler à nouveau
                    newn->Populated = true;
                }
            }
            
            // Expansion du nœud parent pour rendre visible le nouvel élément
            if (tnd) tnd->Expand(false);

            // Expansion du nouveau nœud pour montrer ses enfants
            // Seulement si la destination n'est pas TVList
            if (newtn && tvd->ClassName() != "XTvList") {
                newtn->Expand(false);
            }
        }
        catch (...) {
            // En cas d'erreur, nettoyer le nœud créé
            if (newn) {
                delete newn;
                newn = NULL;
            }
        }
        tvd->Items->EndUpdate();
    }
    catch (...) {
        // En cas d'erreur dans la copie, s'assurer que newn est NULL
        if (newn) {
            delete newn;
            newn = NULL;
        }
    }
    
    return newn;
}