//---------------------------------------------------------------------------

#ifndef ThreadPopulateH
#define ThreadPopulateH
//---------------------------------------------------------------------------

#include "ClassCore.h"

//---------------------------------------------------------------------------
class ThreadPopulate : public TThread
{
private:
	unsigned   idx;
	TTreeView  *PTv;
	TTreeNode  *PTn;
	XNode      *PNode;

	void __fastcall SetIcon(TTreeNode *tn, int a, int b, int c);

	protected:
	void __fastcall Execute();
	void __fastcall AddNode();
	void __fastcall Done();

public:
	__fastcall ThreadPopulate(TTreeView *tv, TTreeNode *tvi, bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
