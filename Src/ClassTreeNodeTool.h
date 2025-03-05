//---------------------------------------------------------------------------

#ifndef ClassTreeNodeToolH
#define ClassTreeNodeToolH

class XTNodeTool;

#include "ClassCore.h"

//---------------------------------------------------------------------------
class XTNodeTool {

public:

	__fastcall XTNodeTool(void);
	__fastcall ~XTNodeTool(void);

	XNode* __fastcall CopyTo(TTreeView *tvd, TTreeNode *tns, TTreeNode *tnd);
	void   __fastcall MoveTo(TTreeView *tvd, TTreeNode *tns, TTreeNode *tnd);
};
#endif
