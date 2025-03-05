//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassTreeViewList.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

// -----------------------------------------------------
// CLASS XTvList
// -----------------------------------------------------
__fastcall XTvList::XTvList(TComponent *owner, XCore *core) : TTreeView(owner){
	Parent           = (TPanel*) owner;
	Core             = core;
	PViewFlat        = VIEWMODE::ARBO;
	PViewAz          = AZMODE::NONE;

	Images           = Core->Intf->TvIcones;
	Align            = alClient;
	ShowLines        = false;
	ShowButtons      = false;
	ReadOnly         = true;
	DoubleBuffered   = true;
	OnCustomDrawItem = OCustomDrawItem;
	OnDblClick       = ODblClick;
	OnMouseDown      = OMouseDown;
	OnStartDrag      = OStartDrag;
	OnResize         = OResize;
	SortType         = Comctrls::stNone;

	MultiSelect      = true;
	MultiSelectStyle = TMultiSelectStyle() << msShiftSelect << msControlSelect;

	StyleElements    >> seFont;
	Font->Color      = Core->Settings->ColorList;
	Font->Size       = Core->FontSize;

	PTNTool    = new XTNodeTool();

	PNR        = NULL;
	PNRSong    = new XNode(NULL, L"Song"     , NODETYPE::ROOT);
	PNRArbo    = new XNode(NULL, L"Arbo"     , NODETYPE::ROOT);
	PNRFlat    = new XNode(NULL, L"Flat"     , NODETYPE::ROOT);
	PNRArboAz  = new XNode(NULL, L"Arbo A..Z", NODETYPE::ROOT);
	PNRFlatAz  = new XNode(NULL, L"Flat A..Z", NODETYPE::ROOT);
    Reset();
}

__fastcall XTvList::~XTvList(void) {
	if (PNRFlatAz != NULL) delete PNRFlatAz;
	if (PNRArboAz != NULL) delete PNRArboAz;
	if (PNRFlat   != NULL) delete PNRFlat;
	if (PNRArbo   != NULL) delete PNRArbo;
	if (PNRSong   != NULL) delete PNRSong;
	if (PTNTool   != NULL) delete PTNTool;
}

void __fastcall XTvList::Reset(void) {
	NRSong->Clear();
	NRSong->Path = L"Song";

	NRArbo->Clear();
	NRArbo->Path = Core->Settings->MusicPath;

	NRFlat->Clear();
	NRFlat->Path = L"Flat";

	NRFlatAz->Clear();
	NRFlatAz->Path = L"Flat A..Z";
	NRFlatAz->Add( L"0..9",NODETYPE::FOLDER );

	NRArboAz->Clear();
	NRArboAz->Path = L"Arbo A..Z";
	NRArboAz->Add( L"0..9",NODETYPE::FOLDER );

	for (char a = 'A'; a <= 'Z'; a++) {
		NRFlatAz->Add( UnicodeString(a), NODETYPE::FOLDER );
		NRArboAz->Add( UnicodeString(a), NODETYPE::FOLDER );
	}
	ResetTNR();
}

void __fastcall XTvList::ResetTNR(void) {
	Items->Clear();
	PTNR = Items->AddObjectFirst(NULL,L"List", PNR );
}

void __fastcall XTvList::SetViewFlat(int flat) {
	PViewFlat = flat;
	Core->Intf->ArboButton = flat;
	SetNodeRoot();
}

void __fastcall XTvList::SetViewAz(int az) {
	PViewAz = az;
	Core->Intf->AzButton = az;
	SetNodeRoot();
}

void __fastcall XTvList::SetNodeRoot(void) {
	if ( ( PViewFlat == VIEWMODE::ARBO ) && ( PViewAz == AZMODE::NONE ) ) { NR = NRArbo;   return; }
	if ( ( PViewFlat == VIEWMODE::FLAT ) && ( PViewAz == AZMODE::NONE ) ) { NR = NRFlat;   return; }
	if ( ( PViewFlat == VIEWMODE::ARBO ) && ( PViewAz == AZMODE::AZ )   ) { NR = NRArboAz; return; }
	if ( ( PViewFlat == VIEWMODE::FLAT ) && ( PViewAz == AZMODE::AZ )   ) { NR = NRFlatAz; return; }
}

void __fastcall XTvList::SetNR(XNode *noderoot) {
	PNR = noderoot;
	ResetTNR();
	PTNR->Text = PNR->Path;
	PTNR->Data = PNR;
	PNR->ResetPopulated();
	PNR->Sort();
	new ThreadPopulate(this, PTNR, false);
}

void __fastcall XTvList::RollArboFlat(void) {
	if ( PViewFlat      == VIEWMODE::ARBO ) ViewFlat = VIEWMODE::FLAT;
	else if ( PViewFlat == VIEWMODE::FLAT ) ViewFlat = VIEWMODE::ARBO;
}

void __fastcall XTvList::RollAz(void) {
	if ( PViewAz == AZMODE::NONE )    ViewAz = VIEWMODE::FLAT;
	else if ( PViewAz == AZMODE::AZ ) ViewAz = VIEWMODE::ARBO;
}


void __fastcall XTvList::AlphaFind(char letter) {
	TTreeNode *n;

	if ( PViewAz == AZMODE::NONE ) {
		if (letter == '#') {
			TopItem = PTNR->getFirstChild();
			return;
		}
		n = PTNR->getFirstChild();
		while (n != NULL) {
			if (n->Text[1] == letter) {
				TopItem = n;
				return;
			}
			n = n->GetNext();
		}
		return;
	}
	if ( PViewFlat == VIEWMODE::ARBO ) {
		if (letter != '#') NR = NRArboAz->ChildAt(letter - 'A' + 1);
		else NR = NRArboAz->ChildAt(0);
		return;
	}
	if ( PViewFlat == VIEWMODE::FLAT ) {
		if (letter != '#') NR = NRFlatAz->ChildAt(letter - 'A' + 1);
		else NR = NRFlatAz->ChildAt(0);
		return;
	}
}

void __fastcall XTvList::ODblClick(TObject *Sender) {
	if (Selected == NULL) return;
	XNode *cn = (XNode*) Selected->Data;

	if (cn == NULL) return;
	switch (cn->NodeType) {
		case NODETYPE::ROOT   :
		case NODETYPE::FOLDER : {
			if (!cn->Populated) {
				if (cn->HaveChild) {
					cn->Sort();
					new ThreadPopulate(this, Selected, false);
				}
			}
			break;
		}
		case NODETYPE::SONG : {
			Core->Mix->Wave->Scan(cn);
			XNode *newnode = PTNTool->CopyTo(Core->TvPast, Selected, Core->TvPast->TNR);
			Core->Mix->NodeNow(newnode);
			break;
		}
	}
}

void __fastcall XTvList::OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw) {
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
			else SetIcon(Node, 5,5,6);                          // Note
		}
	} else if ( cn->IsFolder ) {
		if (cn->CountFolder > 0) SetIcon(Node, 3,3,4);   // Folder +
		else SetIcon(Node, 1,1,2);                       // Folder
		return;
	}
}

void __fastcall XTvList::SetIcon(TTreeNode *tn, int a, int b, int c) {
	tn->ImageIndex = a;
	tn->ExpandedImageIndex = b;
	tn->SelectedIndex = c;
}

// -----------------------------------------------------
//          DRAG DROP
// -----------------------------------------------------

void __fastcall XTvList::OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
	if (Button == mbLeft) {
		TTreeView *tv = (TTreeView*)Sender;
		if ( tv->GetNodeAt(X, Y) >= 0 ) tv->BeginDrag(false, -1);
	}
}

void __fastcall XTvList::OStartDrag(TObject *Sender, TDragObject *&DragObject) {
}

void __fastcall XTvList::OResize(TObject *Sender) {
	ShowScrollBar(Handle, SB_HORZ, false);
}

