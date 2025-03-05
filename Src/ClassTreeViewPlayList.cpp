//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassTreeViewPlayList.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

// -----------------------------------------------------
// CLASS ClassTreeViewPlayList
// -----------------------------------------------------
__fastcall XTvPlayList::XTvPlayList(TComponent *owner, XCore *core) : TTreeView(owner){
	Core             = core;
	Parent           = (TPanel*) owner;
	Images 			 = Core->Intf->TvIcones;
	Align            = alClient;
	ShowRoot         = false;
	ShowLines        = false;
	ShowButtons      = false;
	ReadOnly         = false;
	DoubleBuffered   = true;
	OnCustomDrawItem = OCustomDrawItem;
	OnEdited         = OEdited;
	OnDblClick       = ODblClick;
	OnMouseDown      = OMouseDown;
	OnDragOver       = ODragOver;
	OnDragDrop       = ODragDrop;
	OnEndDrag        = OEndDrag;
	OnResize         = OResize;

	MultiSelect      = true;
	MultiSelectStyle = TMultiSelectStyle() << msShiftSelect << msControlSelect;

	Font->Size       = Core->FontSize;
	StyleElements    >> seFont;
	Font->Color      = Core->Settings->ColorPlaylist;
	PTNTool          = new XTNodeTool();
	PNR              = new XNode(NULL,L"Playlist", NODETYPE::ROOT);
	Reset();
}

__fastcall XTvPlayList::~XTvPlayList(void) {
	PNR->Clear();
	if (PNR     != NULL) delete PNR;
	if (PTNTool != NULL) delete PTNTool;
}

void __fastcall XTvPlayList::Reset(void) {
	if (PTNR == NULL) PTNR = Items->AddObjectFirst(NULL,L"PlayList" , PNR );
	if (PTNR->Count == 0) return;
	PTNR->DeleteChildren();
	PNR->Clear();
	Refresh();
}

XNode* __fastcall XTvPlayList::GetNode(TTreeNode *tn) {
	if (tn != NULL) return (XNode*) tn->Data;
	return NULL;
}

void __fastcall XTvPlayList::Remove(TTreeNode *treenode) {
	XNode *cn, *cnp;
	TTreeNode *tn;

	if (treenode == NULL) return;
	cn = (XNode*) treenode->Data;
	if (cn == PNR) return;
	if (cn->NodeType == NODETYPE::SONG) treenode->Data = NULL;
	tn = treenode->Parent;
	Core->Loader->SavePlayListNeeded = true;
	delete treenode;
	delete cn;
	if (tn == NULL) return;
	cnp = (XNode*) tn->Data;
	if ( (cnp->NodeType == NODETYPE::FOLDER) && (!tn->HasChildren) ) Remove(tn);
}

void __fastcall XTvPlayList::OEdited(TObject *Sender, TTreeNode *Node, UnicodeString &S)
{
	if (Node == NULL) return;
	XNode *cn = (XNode*) Node->Data;
	if (cn == NULL) return;
	if ( cn->NodeType != NODETYPE::SONG ) {
		Core->Loader->SavePlayListNeeded = true;
		cn->Path = S;
	}
}

void __fastcall XTvPlayList::ODblClick(TObject *Sender) {
	XNode *nodepast;

	if (Selected == NULL) return;
	XNode *cn = (XNode*) Selected->Data;
	if (cn == NULL) return;
	if ( cn->NodeType == NODETYPE::SONG ) {
		nodepast = PTNTool->CopyTo(Core->TvPast, Selected, Core->TvPast->TNR);
		Core->Mix->Wave->Scan(nodepast);
		Core->Mix->NodeNow(nodepast);
	} else {
		if ( !cn->Populated && cn->HaveChild ) new ThreadPopulate(this, Selected, false);
	}
}

void __fastcall XTvPlayList::ExpandRoot(void) {
	XNode *cn = (XNode*) PTNR->Data;
	if ( !cn->Populated && cn->HaveChild ) new ThreadPopulate(this, PTNR, false);
	PTNR->Expand(false);
}

void __fastcall XTvPlayList::OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw) {
	XNode *cn = (XNode*) Node->Data;

	if (cn == NULL) return;
	Sender->Canvas->Brush->Color = clBlack;
	if ( cn->IsRoot ) {
		SetIcon(Node, 0,0,0);
		return;
	} else if ( cn->IsSong ) {
		if (cn->CountSession > 0)        { // Rouge
			Sender->Canvas->Font->Color = clRed;
			if (cn->CountPlayList > 0) SetIcon(Node, 12,12,10); // Trombone
			else SetIcon(Node, 8,8,6);                          // Note
			return;
		} else if (cn->CountPlay > 0)    { // Vert
			Sender->Canvas->Font->Color = clLime;
			if (cn->CountPlayList > 0) SetIcon(Node, 11,11,10); // Trombone
			else SetIcon(Node, 7,7,6);                          // Note
		} else if (cn->CountPlay == 0)   { // Gris
			Sender->Canvas->Font->Color = clYellow;
			if (cn->CountPlayList > 0) SetIcon(Node, 9,9,10);   // Trombone
			else SetIcon(Node, 5,5,6);                          // Note
		}
	} else if ( cn->IsFolder ) {
		SetIcon(Node, 13,13,13); // PlayList
		return;
	}
}

void __fastcall XTvPlayList::SetIcon(TTreeNode *tn, int a, int b, int c) {
	tn->ImageIndex = a;
	tn->ExpandedImageIndex = b;
	tn->SelectedIndex = c;
}

void __fastcall XTvPlayList::AddPlayList(void) {
	Core->Loader->SavePlayListNeeded = true;
	XNode *cn = new XNode( PNR,L"List", NODETYPE::FOLDER );
	if ( PNR->Populated ) Items->AddChildObject( PTNR,L"List", cn );
}


// -----------------------------------------------------
//          DRAG DROP
// -----------------------------------------------------

void __fastcall XTvPlayList::OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
	if (Button == mbLeft) {
		TTreeView *tv = (TTreeView*)Sender;
		if ( tv->GetNodeAt(X, Y) >= 0 ) tv->BeginDrag(false, -1);
	}
}

void __fastcall XTvPlayList::ODragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept) {
	if      (Source->ClassName() == "XTvList")     Accept = true;
	else if (Source->ClassName() == "XTvNext")     Accept = true;
	else if (Source->ClassName() == "XTvPast")     Accept = true;
	else if (Source->ClassName() == "XTvPlayList") Accept = true;
	else Accept = false;
}

void __fastcall XTvPlayList::ODragDrop(TObject *Sender, TObject *Source, int X, int Y) {
	TTreeNode *tns, *tnd;
	XNode *ns, *nd;
	TTreeView *tvs;
	int src;
	unsigned i;

	// TreeView source
	if      (Source->ClassName() == "XTvList")     src = 0;
	else if (Source->ClassName() == "XTvNext")     src = 0;
	else if (Source->ClassName() == "XTvPast")     src = 0;
	else if (Source->ClassName() == "XTvPlayList") src = 1;
	else return;
	tvs = ((TTreeView*)Source);

	// Dest
	tnd = GetNodeAt(X, Y);
	if (tnd != NULL) nd = (XNode*)tnd->Data; else nd = NULL;

	Items->BeginUpdate();
	for (i = 0; i < tvs->SelectionCount ; i++) {
		tns = tvs->Selections[i];
		if (nd == NULL) {
			if (src == 0) PTNTool->CopyTo(this, tns, TNR);
			if (src == 1) PTNTool->MoveTo(this, tns, TNR);
		} else {
			if (src == 0) PTNTool->CopyTo(this, tns, tnd);
			if (src == 1) PTNTool->MoveTo(this, tns, tnd);
		}
		Core->Loader->SavePlayListNeeded = true;
	}
    Items->EndUpdate();
}

void __fastcall XTvPlayList::OEndDrag(TObject *Sender, TObject *Target, int X, int Y) {
	if (Target) Update();
}

void __fastcall XTvPlayList::OResize(TObject *Sender) {
}



