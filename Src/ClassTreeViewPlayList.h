//---------------------------------------------------------------------------

#ifndef ClassTreeViewPlayListH
#define ClassTreeViewPlayListH
//---------------------------------------------------------------------------

class XTvPlayList;

#include "ClassCore.h"

// -----------------------------------------------------
// ClassTreeViewPlayList
// -----------------------------------------------------
class XTvPlayList : public TTreeView {
	XCore       *Core;
	XTNodeTool  *PTNTool;
	TTreeNode   *PTNR;
	XNode       *PNR;
	TPanel      *PPanel;

	void __fastcall OEdited(TObject *Sender, TTreeNode *Node, UnicodeString &S);
	void __fastcall ODblClick(TObject *Sender);
	void __fastcall OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw);
	void __fastcall OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall ODragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept);
	void __fastcall ODragDrop(TObject *Sender, TObject *Source, int X, int Y);
	void __fastcall OEndDrag(TObject *Sender, TObject *Target, int X, int Y);
	void __fastcall OResize(TObject *Sender);
	void __fastcall SetIcon(TTreeNode *tn, int a, int b, int c);
	void __fastcall SetPanel(TPanel *panel);

public:
//	TImageList *ILList;

	__fastcall XTvPlayList(TComponent *panel, XCore *core);
	__fastcall ~XTvPlayList(void);

	void       __fastcall Reset(void);
	void 	   __fastcall ExpandRoot(void);
	void       __fastcall AddPlayList(void);
	XNode*     __fastcall GetNode(TTreeNode *tn);
	void       __fastcall Remove(TTreeNode *treenode);

	__property TTreeNode *TNR = { read = PTNR};
	__property XNode     *NR  = { read = PNR};

};
#endif
