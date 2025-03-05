//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassTreeViewPast.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

// -----------------------------------------------------
// CLASS XTvPast
// -----------------------------------------------------
__fastcall XTvPast::XTvPast(TComponent *owner, XCore *core) : TTreeView(owner){
	Core             = core;
	Parent           = (TPanel*) owner;
	Align            = alClient;
	Images           = Core->Intf->TvIcones;

	ShowRoot         = false;
	ShowLines        = false;
	ShowButtons      = false;
	ReadOnly         = true;
	DoubleBuffered   = true;
	OnCustomDrawItem = OCustomDrawItem;
	OnDblClick       = ODblClick;
	OnMouseDown      = OMouseDown;
	OnResize         = OResize;

	MultiSelect      = true;
	MultiSelectStyle = TMultiSelectStyle() << msShiftSelect << msControlSelect;

	StyleElements    >> seFont;
	Font->Size       = Core->FontSize;
	Font->Color      = Core->Settings->ColorPast;
	PTNTool          = new XTNodeTool();
	PNR              = new XNode(NULL,L"Starting at: " + FormatDateTime(L"hh:nn",Now()), NODETYPE::ROOT);
	Reset();
}

__fastcall XTvPast::~XTvPast(void) {
	PNR->Clear();
	if (PNR != NULL)     delete PNR;
	if (PTNTool != NULL) delete PTNTool;
}

void __fastcall XTvPast::Reset(void) {
	if (PTNR == NULL) PTNR = Items->AddObjectFirst(NULL,L"Starting at: " + FormatDateTime(L"hh:nn",Now()), PNR );
	if (PTNR->Count == 0) return;
	PTNR->DeleteChildren();
	PNR->Clear();
	Refresh();
}

void __fastcall XTvPast::ODblClick(TObject *Sender) {
	if (Selected == NULL) return;
	XNode *cn = (XNode*) Selected->Data;
	switch (cn->NodeType) {
		case NODETYPE::ROOT   :
		case NODETYPE::FOLDER : {
			if (!cn->Populated) {
				if (cn->HaveChild) {
					new ThreadPopulate(this, Selected, false);
				}
			}
			break;
		}
		case NODETYPE::SONG : {
			Core->Mix->Wave->Scan(cn);
			XNode *newnode = PTNTool->CopyTo(Core->TvPast, Selected, PTNR);
			Core->Mix->NodeNow(newnode);
			break;
		}
	}
}

void __fastcall XTvPast::OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw) {
	XNode *cn = (XNode*) Node->Data;

	if (cn == NULL) return;
	if ( cn->IsRoot ) {
		SetIcon(Node, 0,0,0);
		return;
	} else if ( cn->IsSong ) {
		Sender->Canvas->Font->Color = clRed;
		Sender->Canvas->Brush->Color = clBlack;
		if (cn->CountSession > 0)        { // Rouge
			if (cn->CountPlayList > 0) SetIcon(Node, 12,12,10); // Trombone
			else SetIcon(Node, 8,8,6);                          // Note
			return;
		} else if (cn->CountPlay > 0)    { // Vert
			if (cn->CountPlayList > 0) SetIcon(Node, 11,11,10); // Trombone
			else SetIcon(Node, 7,7,6);                          // Note
		} else if (cn->CountPlay == 0)   { // Gris
			if (cn->CountPlayList > 0) SetIcon(Node, 9,9,10);   // Trombone
			else SetIcon(Node, 5,5  ,6);                        // Note
		}
	} else if ( cn->IsFolder ) {
		if (cn->CountFolder > 0) SetIcon(Node, 3,3,4);   // Folder +
		else SetIcon(Node, 1,1,2);                       // Folder
		return;
	}
}

void __fastcall XTvPast::SetIcon(TTreeNode *tn, int a, int b, int c) {
	tn->ImageIndex = a;
	tn->ExpandedImageIndex = b;
	tn->SelectedIndex = c;
}

// -----------------------------------------------------
//          DRAG DROP
// -----------------------------------------------------

void __fastcall XTvPast::OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
	if (Button == mbLeft) {
		TTreeView *tv = (TTreeView*)Sender;
		if ( tv->GetNodeAt(X, Y) >= 0 ) tv->BeginDrag(false, -1);
	}
}

void __fastcall XTvPast::OResize(TObject *Sender) {
	//ShowScrollBar(Handle, SB_HORZ, false);
}



