//---------------------------------------------------------------------------

#ifndef ClassTreeViewNextH
#define ClassTreeViewNextH
//---------------------------------------------------------------------------

class XTvNext;

#include "ClassCore.h"
#include "ClassTreeNodeTool.h"
#include "ThreadPopulate.h"

// -----------------------------------------------------
// ClassTreeViewNext
// -----------------------------------------------------
class XTvNext : public TTreeView {
	XCore       *Core;

	XNode       *PNR;
	TTreeNode   *PTNR;
	XTNodeTool  *PTNTool;

	TTreeNode*   __fastcall GetTNNext(void);
	XNode*       __fastcall GetNext(void);
	unsigned     __fastcall GetCountSong(void);

	void 		 __fastcall RecurseShuffle(TTreeNode *rstart);
	void         __fastcall SetIcon(TTreeNode *tn, int a, int b, int c);

	void         __fastcall OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw);
	void         __fastcall ODblClick(TObject *Sender);
	void         __fastcall OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void         __fastcall ODragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept);
	void         __fastcall ODragDrop(TObject *Sender, TObject *Source, int X, int Y);
	void         __fastcall OEndDrag(TObject *Sender, TObject *Target, int X, int Y);
	void         __fastcall OResize(TObject *Sender);

public:
				 __fastcall XTvNext(TComponent *owner, XCore *core);
				 __fastcall ~XTvNext(void);

	XNode*       __fastcall GetNode(TTreeNode *tn);
	void         __fastcall Reset(void);
	void         __fastcall Remove(TTreeNode *treenode);
	void         __fastcall Randomize(void);
	void         __fastcall Fill(void);
	void 		 __fastcall Shuffle(void);
	XNode*       __fastcall Pop(void);

	__property TTreeNode *TNR         = { read = PTNR};
	__property TTreeNode *TNNext      = { read = GetTNNext};
	__property XNode     *Next		  = { read = GetNext};
	//__property XNode     *Pop		  = { read = GetPop};
	__property XNode     *NR    	  = { read = PNR};
	__property unsigned   CountSong	  = { read = GetCountSong};

};
#endif
