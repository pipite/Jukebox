//---------------------------------------------------------------------------

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

void __fastcall XTNodeTool::MoveTo(TTreeView *tvd, TTreeNode *tns, TTreeNode *tnd) {
	XNode *ns, *nd;

	if (tvd == NULL) return;
	if (tns == NULL) return;
	ns = (XNode*) tns->Data;
	if (ns == NULL) return;
	if (ns->IsRoot) return;

	// Si pas de TreeNodeDest Alors prendre le TreeNodeRoot du TreeViewDest
	if (tnd == NULL) tnd = tvd->Items->GetFirstNode();
	nd = (XNode*)tnd->Data;

	// Verifier que le nodedest ne soit pas enfant du node source!!
	if (nd->HasAsParent(ns)) return;

	tvd->Items->BeginUpdate();
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
	// Folder -> Folder Insertion au dessus
	} else {
		tns->MoveTo(tnd, naInsert);
	}
	tvd->Items->EndUpdate();
	ns->MoveTo(nd);
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
	nd = (XNode*)tnd->Data;

	newn = ns->CopyTo(nd);
	if (newn->IsRoot) newn->NodeType = NODETYPE::FOLDER;

	tvd->Items->BeginUpdate();
	// Song -> Song     Insertion au dessus
	if ( ns->IsSong && nd->IsSong ) {
		newtn = tvd->Items->InsertObject(tnd, tns->Text, newn);
	// Folder -> Song   Insertion au dessus
	} else if ( !ns->IsSong && nd->IsSong ) {
		newtn = tvd->Items->InsertObject(tnd, tns->Text, newn);
	// Song -> Folder ou Root  Ajouter un child au Folder
	} else if ( ns->IsSong && !nd->IsSong ) {
		if ( nd->Populated ) newtn = tvd->Items->AddChildObject(tnd, tns->Text, newn);
	// Folder -> Root   Ajouter un child au Root
	} else if ( !ns->IsSong && nd->IsRoot ) {
		newtn = tvd->Items->AddChildObject(tnd, tns->Text, newn);
	// Folder -> Folder Insertion au dessus
	} else {
		tvd->Items->InsertObject(tnd, tns->Text, newn);
	}
	tvd->Items->EndUpdate();
	if (newtn == NULL) return newn;

	if ( (!newn->Populated) && (newn->HaveChild) ) {
		new ThreadPopulate(tvd, newtn, false);
	}
	return newn;
}


