//---------------------------------------------------------------------------

#ifndef ClassTreeViewPastH
#define ClassTreeViewPastH
//---------------------------------------------------------------------------

class XTvPast;

#include "ClassCore.h"

// -----------------------------------------------------
// ClassTreeViewPast
// -----------------------------------------------------
class XTvPast : public TTreeView {
	XCore       *Core;
	TTreeNode   *PTNR;
	XTNodeTool  *PTNTool;
	XNode       *PNR;


	void __fastcall ODblClick(TObject *Sender);
	void __fastcall OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw);
	void __fastcall OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall OResize(TObject *Sender);
	void __fastcall SetIcon(TTreeNode *tn, int a, int b, int c);
public:
	TImageList *ILList;

	__fastcall XTvPast(TComponent *owner, XCore *core);
	__fastcall ~XTvPast(void);

	void __fastcall Reset(void);

	__property TTreeNode *TNR = {read = PTNR};
	__property XNode     *NR  = {read = PNR};

};
#endif
