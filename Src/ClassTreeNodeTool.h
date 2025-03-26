//---------------------------------------------------------------------------
#ifndef ClassTreeNodeToolH
#define ClassTreeNodeToolH

class XTNodeTool;

#include "ClassCore.h"

//---------------------------------------------------------------------------
class XTNodeTool {

public:

	XTNodeTool(void);
	~XTNodeTool(void);

	XNode* __fastcall CopyTo(TTreeView *tvd, TTreeNode *tns, TTreeNode *tnd);
	void   __fastcall MoveTo(TTreeView *tvd, TTreeNode *tns, TTreeNode *tnd);
	void   __fastcall AddChildrenToTree(TTreeView *tv, TTreeNode *parentTn, XNode *parentNode);
	void   __fastcall RecursePopulate(TTreeView *tv, TTreeNode *tn);
	void   __fastcall RecursePopulate(TTreeView *tv, TTreeNode *tn, bool expandNodes);
};
#endif
