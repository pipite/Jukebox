//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "ThreadPopulate.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

__fastcall ThreadPopulate::ThreadPopulate(TTreeView *tv, TTreeNode *tn, bool CreateSuspended) : TThread(CreateSuspended) {
	FreeOnTerminate = true;
	PTv = tv;
	PTn = tn;
	PNode = (XNode*)PTn->Data;
	if (PNode == NULL) return;
}

void __fastcall ThreadPopulate::Done() {
	PNode->Populated = true;
	PTn->Expand(false);
	ShowScrollBar(PTv->Handle, SB_HORZ, false);
}

//---------------------------------------------------------------------------
void __fastcall ThreadPopulate::Execute() {
	if (PNode == NULL) { Synchronize( Done ); return; }
	PTv->Items->BeginUpdate();
	if (!PNode->Populated ) {
		for (idx = 0; idx < PNode->CountChild ; idx++) Synchronize( AddNode );
	}
	Synchronize( Done );
	PTv->Items->EndUpdate();
}

void __fastcall ThreadPopulate::AddNode() {
	XNode *cn;

	cn = PNode->ChildAt(idx);
	PTv->Items->AddChildObject( PTn, cn->FileName, cn );
}

void __fastcall ThreadPopulate::SetIcon(TTreeNode *tn, int a, int b, int c) {
	tn->ImageIndex = a;
	tn->ExpandedImageIndex = b;
	tn->SelectedIndex = c;
}


//---------------------------------------------------------------------------
