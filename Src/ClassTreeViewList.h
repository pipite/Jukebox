//---------------------------------------------------------------------------

#ifndef ClassTreeViewListH
#define ClassTreeViewListH
//---------------------------------------------------------------------------

class XTvList;

#include "ClassCore.h"
#include "ClassNode.h"
#include "ClassTreeNodeTool.h"

enum VIEWMODE { ARBO, FLAT };
enum AZMODE { NONE, AZ };

// -----------------------------------------------------
// XTvList
// -----------------------------------------------------
class XTvList : public TTreeView {
	XCore      *Core;
	int         PViewFlat;
	int         PViewAz;

	XTNodeTool *PTNTool;

	TTreeNode  *PTNR;

	XNode      *PNR;
	XNode      *PNRSong;
	XNode      *PNRArbo;
	XNode      *PNRFlat;
	XNode      *PNRArboAz;
	XNode      *PNRFlatAz;

	void __fastcall SetNR(XNode *noderoot);
	void __fastcall ODblClick(TObject *Sender);
	void __fastcall OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall OStartDrag(TObject *Sender, TDragObject *&DragObject);
	void __fastcall SetViewFlat(int flat);
	void __fastcall SetViewAz(int az);
	void __fastcall OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw);
	void __fastcall OResize(TObject *Sender);
	void __fastcall SetIcon(TTreeNode *tn, int a, int b, int c);

public:
		 __fastcall XTvList(TComponent *owner, XCore *core);
		 __fastcall ~XTvList(void);

	void __fastcall ResetTNR(void);
	void __fastcall Reset(void);
	void __fastcall RollArboFlat(void);
	void __fastcall RollAz(void);
	void __fastcall AlphaFind(char letter);
	void __fastcall SetNodeRoot(void);

	__property TTreeNode *TNR      = { read = PTNR};

	__property XNode     *NR       = { read = PNR, 		 write = SetNR};
	__property XNode     *NRArbo   = { read = PNRArbo};
	__property XNode     *NRSong   = { read = PNRSong} ;
	__property XNode     *NRFlat   = { read = PNRFlat};
	__property XNode     *NRArboAz = { read = PNRArboAz};
	__property XNode     *NRFlatAz = { read = PNRFlatAz};
	__property int        ViewFlat = { read = PViewFlat, write = SetViewFlat};
	__property int        ViewAz   = { read = PViewAz,   write = SetViewAz};

};
#endif
