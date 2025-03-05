//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassNode.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   ClassNode    Constructor
// ---------------------------------------------------------------------------
__fastcall XNode::XNode(XNode *dest, UnicodeString path, int nodetype) {
	// New Node
	PLinkNode        = NULL;
	PNodeType        = nodetype;
	PPath            = path;
	Init();
	SetParent(dest);
}

__fastcall XNode::XNode(XNode *dest, XNode *link) {
	// New Node Linked (Song only)
	PLinkNode        = link;
	PNodeType        = NODETYPE::SONG;
	PPath            = L"";
	Init();
	SetParent(dest);
}

void __fastcall XNode::Init(void) {
	// Node
	PParent          = NULL;
	Child            = new TList();
	Child->Clear();
	PCountFolder     = 0;
	PCountSong       = 0;
	PCountSongAll    = 0;
	PLevel           = 0;

	// Song
	Populated        = false;
	PWaveLoaded      = false;
	PWaveLoading     = false;
	PWave            = NULL;
	PBitmap          = NULL;
	PMsFadeIn        = 0;
	PMsFadeOut       = 0;
	PMsLength        = 0;
	Peak             = NULL;
	PCountPlay       = 0;
	PCountPlayList   = 0;
	PCountSession    = 0;
	ShowWave         = true;
	PIsSort          = false;
}

__fastcall XNode::~XNode(void) {
	if ( IsSong ) {
		PParent->PCountSong--;
		PParent->UpdateSongAll(-1);
	} else if (!IsRoot) {
		PParent->PCountFolder--;
	}
	if ( !IsSong ) Clear();
	if ( PWave   != NULL ) delete PWave;
	if ( PBitmap != NULL ) delete PBitmap;
	if ( Child   != NULL)  delete Child;
	if ( Parent  != NULL)  Parent->Child->Extract(this);
};

// ---------------------------------------------------------------------------
//   ClassNode    Node   Add  Link  CopyTo  MoveTo  Extract  Clear
// ---------------------------------------------------------------------------

void __fastcall XNode::SetParent(XNode *dest) {
	UnicodeString a;

	if (dest == NULL) {
		PNodeType = 0;
		PLevel    = 0;
		PParent   = NULL;
		return;
	}

	if ( HaveParent ) {
		// MoveTo
		if ( dest->IsSong ) {
			if (IsSong) {
				// Source: Song >> Dest: Song
				PParent->PCountSong--;
				PParent->UpdateSongAll(-1);
			} else if ( IsFolder || !IsRoot ) {
				// Source: Folder >> Dest: Song
				PParent->PCountFolder--;
				PParent->UpdateSongAll(-(int)PCountSongAll);
			}
		} else {
			if (IsSong) {
				// Source: Song >> Dest: Folder
				PParent->PCountSong--;
				PParent->UpdateSongAll(-1);
			} else if ( IsFolder || !IsRoot ) {
				// Source: Folder >> Dest: Folder
				PParent->PCountFolder--;
				PParent->UpdateSongAll(-(int)PCountSongAll);
			}
		}
		PParent->Child->Extract(this);
		PParent = NULL;
	}

	if ( dest->IsFolder || dest->IsRoot ) {
		if (IsFolder || IsRoot ) {
			dest->PCountFolder++;
			dest->UpdateSongAll(PCountSongAll);
		} else if ( IsSong ) {
			dest->PCountSong++;
			dest->UpdateSongAll(1);
		}
	} else if ( dest->IsSong ) {
		if (IsFolder || IsRoot ) {
			dest->Parent->PCountFolder++;
			dest->Parent->UpdateSongAll(PCountSongAll);
		} else if ( IsSong ) {
			dest->Parent->PCountSong++;
			dest->Parent->UpdateSongAll(1);
		}
	}

	// ADD a la fin du folder si dest = FOLDER ou ROOT
	if ( ( IsRoot || IsFolder ) && ( dest->IsRoot || dest->IsFolder ) ) {
		dest->Child->Add(this);
		PNodeType = NODETYPE::FOLDER;
		PLevel = dest->PLevel + 1;
		PParent = dest;
		return;
	} else
	if ( IsSong && ( dest->IsRoot || dest->IsFolder ) ) {
		dest->Child->Add(this);
		PLevel = dest->PLevel + 1;
		PParent = dest;
		return;
	} else
	// INSERT au dessus de la song si dest = SONG
	if ( ( IsRoot || IsFolder ) && dest->IsSong ) {
		dest->PParent->Child->Insert(dest->Index,this);
		PLevel = dest->PParent->PLevel + 1;
		PParent = dest->PParent;
		return;
	} else
	if ( IsSong && dest->IsSong ) {
		dest->PParent->Child->Insert(dest->Index,this);
		PLevel = dest->PParent->PLevel + 1;
		PParent = dest->PParent;
		return;
	}
}

void __fastcall XNode::Clear() {
	int i;
	if ( !IsSong ) {
		while ( (i = Child->Count) > 0 ) {
			if (ChildAt(i-1)->IsFolder) ChildAt(i-1)->Clear();
			if (ChildAt(i-1) != NULL) delete (XNode*) ChildAt(i-1);
		}
		Child->Clear();
	}
	Populated = false;
	PIsSort   = false;
}

XNode* __fastcall XNode::Add(UnicodeString path, int nodetype) {
	return new XNode(this, path, nodetype);
}

XNode* __fastcall XNode::Add(XNode *nodesrc) {
	// this = dest     : Source est un obligatoirement un folder sinon, link
	XNode *newnode = new XNode(this, nodesrc->Path, nodesrc->NodeType);
	for (int i = 0; i < nodesrc->Child->Count; i++) nodesrc->ChildAt(i)->CopyTo(newnode);
	return newnode;
}

XNode* __fastcall XNode::Link(XNode *nodesrc) {
	return new XNode(this, nodesrc);
}

XNode* __fastcall XNode::CopyTo(XNode *dest) {
	// this = source
	if (dest == NULL)       return NULL;
	if (IsSong)             return dest->Link(this->PLinkNode);
	if (IsFolder || IsRoot) return dest->Add(this);
	return NULL;
}

void __fastcall XNode::MoveTo(XNode *dest) {
	if (dest->HasAsParent(this)) return;
	if (dest != NULL) Parent = dest;
}

void __fastcall XNode::UpdateSongAll(int nbsong) {
	PCountSongAll += nbsong;
	if (PLevel != 0) Parent->UpdateSongAll(nbsong);
}

XNode* __fastcall XNode::ChildAt(unsigned index) {
	if (Child->Count == 0) return NULL;
	if ( index <= unsigned(Child->Count) ) return (XNode*) Child->Items[index];
	return NULL;
}


void __fastcall XNode::ResetPopulated(void) {
	Populated = false;
	for (int i = 0; i < Child->Count ; i++) {
		ChildAt(i)->ResetPopulated();
	}
}

XNode* __fastcall XNode::Find(UnicodeString path) {
	for (int i = 0; i < (Child->Count); i++) {
		if (ChildAt(i)->Path == path)
			return ChildAt(i);
	}
	return NULL;
}

bool __fastcall XNode::HasAsParent(XNode *node) {
	XNode *dest = this;
	while ( dest->Level >= node->Level ) {
		if ( dest == node ) return true;
		dest = dest->Parent;
		if (dest == NULL) return false;
	}
	return false;
}


// ---------------------------------------------------------------------------
//   ClassNode    Get  Set
// ---------------------------------------------------------------------------

unsigned __fastcall XNode::GetIndex(void) {
	if (PLevel == 0) return 0;
	return Parent->Child->IndexOf(this);
}

DWORD* __fastcall XNode::GetWave(void) {
	if (PWave == NULL) SongNode->PWave = new DWORD[1000];
	return SongNode->PWave;
}

TBitmap* __fastcall XNode::GetBitmap(void) {
	if (PBitmap == NULL) SongNode->PBitmap = new TBitmap();
	return SongNode->PBitmap;
}

XNode* __fastcall XNode::GetSongNode(void) {
	if (PLinkNode == NULL) return this;
	return PLinkNode;
}

UnicodeString __fastcall XNode::GetFileName(void) {
	UnicodeString s = ExtractFileName(SongNode->Path) ;
	if (IsSong) s = s.SubString(1, s.Length()-ExtractFileExt(s).Length()) ;
	return s;
}

XNode* __fastcall XNode::GetRandomChild(void) {
	if (Child->Count == 0) return NULL;
	int i = Random()*Child->Count;
	return ChildAt( i );
}

bool 		  __fastcall XNode::GetHaveParent(void)              { return (PParent != NULL); }
bool 		  __fastcall XNode::GetIsLink(void)                  { return (PLinkNode != NULL); }
bool          __fastcall XNode::GetHaveChild(void)               { return (Child->Count > 0); }
bool          __fastcall XNode::GetHaveSong(void)                { return (PCountSongAll > 0 ); }
unsigned      __fastcall XNode::GetCountChild(void)              { return Child->Count; }
bool          __fastcall XNode::GetIsRoot(void)                  { return (NodeType == NODETYPE::ROOT); }
bool          __fastcall XNode::GetIsFolder(void)                { return (NodeType == NODETYPE::FOLDER); }
bool          __fastcall XNode::GetIsSong(void)                  { return (NodeType == NODETYPE::SONG); }
bool          __fastcall XNode::GetWaveLoaded(void)              { return SongNode->PWaveLoaded; }
bool          __fastcall XNode::GetWaveLoading(void)             { return SongNode->PWaveLoading; }
unsigned      __fastcall XNode::GetCountPlay(void)               { return SongNode->PCountPlay; }
unsigned      __fastcall XNode::GetCountPlayList(void)           { return SongNode->PCountPlayList; }
unsigned      __fastcall XNode::GetCountSession(void)            { return SongNode->PCountSession; }
UnicodeString __fastcall XNode::GetPath(void)                    { return SongNode->PPath; }
int           __fastcall XNode::GetMsFadeIn(void)                { return SongNode->PMsFadeIn; }
int           __fastcall XNode::GetMsFadeOut(void)               { return SongNode->PMsFadeOut; }
int           __fastcall XNode::GetMsLength(void)                { return SongNode->PMsLength; }
int           __fastcall XNode::GetNodeType(void)                { return SongNode->PNodeType; }

void          __fastcall XNode::SetWaveLoaded(bool waveloaded)   { SongNode->PWaveLoaded = waveloaded; }
void          __fastcall XNode::SetWaveLoading(bool waveloading) { SongNode->PWaveLoading = waveloading; }
void          __fastcall XNode::SetCountPlay(unsigned value)     { SongNode->PCountPlay = value; }
void          __fastcall XNode::SetCountPlayList(unsigned value) { SongNode->PCountPlayList = value; }
void          __fastcall XNode::SetCountSession(unsigned value)  { SongNode->PCountSession = value; }
void          __fastcall XNode::SetMsFadeIn(int msfadein)        { SongNode->PMsFadeIn = msfadein; }
void          __fastcall XNode::SetMsFadeOut(int msfadeout)      { SongNode->PMsFadeOut = msfadeout; }
void          __fastcall XNode::SetMsLength(int mslength)        { SongNode->PMsLength = mslength; }

int __fastcall SortAscending(void *Item1, void *Item2) {
	XNode* MS1 = (XNode*)Item1;
	XNode* MS2 = (XNode*)Item2;
	if (MS1->NodeType != MS2->NodeType) {
		if (MS1->NodeType == NODETYPE::FOLDER) return -1;
		if (MS2->NodeType == NODETYPE::FOLDER) return 1;
	}
	return MS1->FileName.Compare(MS2->FileName);
}


void __fastcall XNode::Sort(void) {
	if (!PIsSort) Child->Sort(&SortAscending);
	PIsSort = true;
}

void __fastcall XNode::DrawWave(TImage *image) {
	int i, iii, lastiii, cpt, mspos, w, h;
	float hh, ii;
	TCanvas *c;
	TBitmap *bm;
	int msfi, msfo, msl;

	if (!ShowWave) return;
	bm = SongNode->Bitmap;
	w = image->Width;
	h = image->Height;
	bm->Width  = w;
	bm->Height = h;
	image->Picture->Bitmap->Width  = w;
	image->Picture->Bitmap->Height = h;
	c = bm->Canvas;

	c->Pen->Color   = clGray;
	c->Brush->Color = clBlack;
	c->Rectangle(0, 0, w, h);
	c->Pen->Color   = clRed;

	hh = (float) h - (float)Wave[0]/32767.0 * h;
	Peak = Wave[0];
	c->MoveTo(0,(int) h);
	lastiii = 0;
	cpt = 0;
	hh = 0.0;
	msfi = SongNode->MsFadeIn;
	msfo = SongNode->MsFadeOut;
	msl  = SongNode->MsLength;

	for (i = 1; i < 1000; i++) {
		mspos = (int) ( (float) i * msl / 1000);
		if ( mspos > msfo ) c->Pen->Color = clRed;
		else if ( mspos > msfi ) c->Pen->Color = clSkyBlue;
		if (Peak < Wave[i]) Peak = Wave[i];
		hh += (float) h - (float)Wave[i]/32767.0 * h;
		ii = (float)i/1000.0 * (float) w;
		iii = (int) ii;
		cpt++;
		if (lastiii != iii) {
			c->LineTo( ii,(int)(hh/cpt));
			hh  = 0.0;
			cpt = 0;
			lastiii = iii;
		}
	}
	BitBlt(image->Canvas->Handle, 0, 0, w, h, bm->Canvas->Handle, 0, 0, SRCCOPY);
	image->Refresh();
}













