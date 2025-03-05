//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassTreeViewNext.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

// -----------------------------------------------------
// CLASS XTvNext
// -----------------------------------------------------
__fastcall XTvNext::XTvNext(TComponent *owner, XCore *core) : TTreeView(owner){
	Parent         = (TPanel*) owner;
	Core           = core;
	Images         = Core->Intf->TvIcones;
	Align          = alClient;
	ShowRoot       = false;
	ShowLines      = false;
	ShowButtons    = false;
	ReadOnly       = true;
	DoubleBuffered = true;

	Touch->StandardGestures    = TStandardGestures() << sgRight << sgLeft;

	OnDblClick  = ODblClick;
	OnMouseDown = OMouseDown;
	OnDragOver  = ODragOver;
	OnDragDrop  = ODragDrop;
	OnEndDrag   = OEndDrag;
	OnCustomDrawItem = OCustomDrawItem;
	OnResize         = OResize;

	MultiSelect      = true;
	MultiSelectStyle = TMultiSelectStyle() << msShiftSelect << msControlSelect;

	StyleElements    >> seFont;
	Font->Size       = Core->FontSize;
	Font->Color      = Core->Settings->ColorNext;

	PTNTool          = new XTNodeTool();
	PNR              = new XNode(NULL,L"Next", NODETYPE::ROOT);
	Reset();
}

__fastcall XTvNext::~XTvNext(void) {
	PNR->Clear();
	if (PNR != NULL)     delete PNR;
	if (PTNTool != NULL) delete PTNTool;
}

XNode* __fastcall XTvNext::GetNode(TTreeNode *tn) {
	if (tn != NULL) return (XNode*) tn->Data;
	return NULL;
}

void __fastcall XTvNext::Reset(void) {
	if (PTNR == NULL) PTNR = Items->AddObjectFirst(NULL,L"Next", PNR );
	if (PTNR->Count > 0) {
		PTNR->DeleteChildren();
		PNR->Clear();
	}
	PNR->Populated = true;
	Refresh();
}

void __fastcall XTvNext::Randomize(void) {
	Reset();
	Fill();
}

XNode* __fastcall XTvNext::GetPop(void) {
	TTreeNode *tn;
	XNode *newnode;

	tn = GetTNNext();
	if (tn != NULL) {
		newnode = PTNTool->CopyTo(Core->TvPast, tn, Core->TvPast->TNR);
		Remove(tn);
		return newnode;
	}
	return NULL;
}

void __fastcall XTvNext::Remove(TTreeNode *treenode) {
	XNode *cn, *cnp;
	TTreeNode *tn;

	if (treenode == NULL) return;
	cn = (XNode*) treenode->Data;
	if (cn == PNR) return;
	if (cn->NodeType == NODETYPE::SONG) treenode->Data = NULL;
	tn = treenode->Parent;
	delete treenode;
	delete cn;
	if (tn == NULL) return;
	cnp = (XNode*) tn->Data;
	if ( (cnp->NodeType == NODETYPE::FOLDER) && (!tn->HasChildren) ) Remove(tn);
    Fill();
}

TTreeNode* __fastcall XTvNext::GetTNNext(void) {
	TTreeNode *n;
	XNode *c;

	if (!PTNR->HasChildren) return NULL;
	n = PTNR->getFirstChild();
	while (n != NULL) {
		c = (XNode*) n->Data;
		if ( c->NodeType == NODETYPE::FOLDER ) {
			if (!c->Populated) {
				if (c->HaveChild) {
					new ThreadPopulate(this, n, false);
				}
			}
		}
		if ( c->NodeType == NODETYPE::SONG ) return n;
		n = n->GetNext();
	}
	return NULL;
}

XNode* __fastcall XTvNext::GetNext(void) {
	TTreeNode *tn;

	tn = GetTNNext();
	if (tn == NULL) return NULL;
	return (XNode*)tn->Data;
}

void __fastcall XTvNext::Fill(void) {
	XNode *cn;
	unsigned  cs;
	int i;

	if (Core->TvList->NRSong->CountSong == 0) return;
	cs = CountSong;
	Items->BeginUpdate();
	for (i = cs; i < Core->Settings->AutoSelect; i++) {
		cn = PNR->Link(Core->TvList->NRSong->RandomChild);
		Items->AddChildObject( PTNR, cn->FileName, cn );
		Core->Mix->Wave->Scan(cn);
	}
	Refresh();
	PTNR->Expand(false);
	Items->EndUpdate();
	ShowScrollBar(Handle, SB_HORZ, false);
}

void __fastcall XTvNext::Shuffle(void) {
	TTreeNode *nodestart;
	XTvNext *tv;

	srand (time(NULL));

	tv = Core->TvNext;
	if (tv->CountSong < 2) return;

	nodestart = Selected;
	if (nodestart == NULL) nodestart = tv->Items->GetFirstNode();
	if (nodestart == NULL) return;

	if ( ((XNode*) nodestart->Data)->IsSong ) nodestart = nodestart->Parent;
	if (nodestart == NULL) return;

	tv->Updating();
	RecurseShuffle(nodestart);
	tv->Updated();
}

void __fastcall XTvNext::RecurseShuffle(TTreeNode *rstart) {
	TTreeNode *node, *nodedest;
	XNode *n;
	int r, count;

	count = rstart->Count;
	Items->BeginUpdate();
	for (int i = 0; i < count; i++) {
		node = rstart->Item[i];
		r = (int) Random(count);
		nodedest = rstart->Item[r];
		if ( ((XNode*) node->Data)->IsFolder ) RecurseShuffle(node);
		if ( count > 1 )node->MoveTo(nodedest,naInsert);
	}
	Items->EndUpdate();
}

unsigned __fastcall XTvNext::GetCountSong(void) {
	return PNR->CountSongAll;
}

void __fastcall XTvNext::SetIcon(TTreeNode *tn, int a, int b, int c) {
	if (Parent->Name == "PanelMin") {
		tn->ImageIndex = -1;
		tn->ExpandedImageIndex = -1;
		tn->SelectedIndex = -1;
	} else {
		tn->ImageIndex = a;
		tn->ExpandedImageIndex = b;
		tn->SelectedIndex = c;
	}
}

// -----------------------------------------------------
//          Event
// -----------------------------------------------------

void __fastcall XTvNext::ODblClick(TObject *Sender) {
	XNode *newnode;

	if (Selected == NULL) return;
	XNode *cn = (XNode*) Selected->Data;
	if (cn == NULL) return;
	switch (cn->NodeType) {
		case NODETYPE::ROOT   :
		case NODETYPE::FOLDER : {
			if (!cn->Populated) {
				if (cn->HaveChild) {
					new ThreadPopulate(this, Selected, false);
				}
			}
			return;
		}
		case NODETYPE::SONG : {
			newnode = PTNTool->CopyTo(Core->TvPast, Selected, Core->TvPast->TNR);
			Core->TvPast->TNR->Expand(false);
			Core->Mix->NodeNow(newnode);
			Remove(Selected);
			Fill();
			Core->Mix->Next->Node = GetNext();
		}
	}
}

void __fastcall XTvNext::OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw) {
	XNode *cn = (XNode*) Node->Data;

	if (cn == NULL) return;
	if ( cn->IsRoot ) {
		SetIcon(Node, 0,0,0);
		return;
	} else if ( cn->IsSong ) {
		if (cn->CountSession > 0)        { // Rouge
			Sender->Canvas->Font->Color = clRed;
			Sender->Canvas->Brush->Color = clBlack;
			if (cn->CountPlayList > 0) SetIcon(Node, 12,12,10); // Trombone
			else SetIcon(Node, 8,8,6);                          // Note
			return;
		} else if (cn->CountPlay > 0)    { // Vert
			Sender->Canvas->Font->Color = clLime;
			Sender->Canvas->Brush->Color = clBlack;
			if (cn->CountPlayList > 0) SetIcon(Node, 11,11,10); // Trombone
			else SetIcon(Node, 7,7,6);                          // Note
		} else if (cn->CountPlay == 0)   { // Gris
			Sender->Canvas->Font->Color = clYellow;
			Sender->Canvas->Brush->Color = clBlack;
			if (cn->CountPlayList > 0) SetIcon(Node, 9,9,10);   // Trombone
			else SetIcon(Node, 7,7  ,6);                        // Note
		}
	} else if ( cn->IsFolder ) {
		if (cn->CountFolder > 0) SetIcon(Node, 13,13,13);   // Folder +
		else SetIcon(Node, 13,13,13);                       // Folder
		return;
	}
}

// -----------------------------------------------------
//          DRAG DROP
// -----------------------------------------------------

void __fastcall XTvNext::OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
	if (Button == mbLeft) {
		TTreeView *tv = (TTreeView*)Sender;
		if ( tv->GetNodeAt(X, Y) >= 0 ) tv->BeginDrag(false, -1);
	}
}

void __fastcall XTvNext::ODragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept) {
	if      (Source->ClassName() == "XTvList")     Accept = true;
	else if (Source->ClassName() == "XTvNext")     Accept = true;
	else if (Source->ClassName() == "XTvPast")     Accept = true;
	else if (Source->ClassName() == "XTvPlayList") Accept = true;
	else Accept = false;
}

void __fastcall XTvNext::ODragDrop(TObject *Sender, TObject *Source, int X, int Y) {
	TTreeNode *tns, *tnd;
	XNode *ns, *nd;
	TTreeView *tvs;
	int src;
	unsigned i;

	// TreeView source
	if      (Source->ClassName() == "XTvList")     src = 0;
	else if (Source->ClassName() == "XTvNext")     src = 1;
	else if (Source->ClassName() == "XTvPast")     src = 0;
	else if (Source->ClassName() == "XTvPlayList") src = 0;
	else return;
	tvs = ((TTreeView*)Source);

	// Dest
	tnd = GetNodeAt(X, Y);
	if (tnd != NULL) nd = (XNode*)tnd->Data; else nd = NULL;

	for (i = 0; i < tvs->SelectionCount ; i++) {
		tns = tvs->Selections[i];
		if (nd == NULL) {
			if (src == 0) PTNTool->CopyTo(this, tns, TNR);
			if (src == 1) PTNTool->MoveTo(this, tns, TNR);
		} else {
			if (src == 0) PTNTool->CopyTo(this, tns, tnd);
			if (src == 1) PTNTool->MoveTo(this, tns, tnd);
		}
	}
	Items->EndUpdate();
}

void __fastcall XTvNext::OEndDrag(TObject *Sender, TObject *Target, int X, int Y) {
	if (Target) Update();
}

void __fastcall XTvNext::OResize(TObject *Sender) {
}



