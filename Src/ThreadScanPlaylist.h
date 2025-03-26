//---------------------------------------------------------------------------
#ifndef ThreadScanPlaylistH
#define ThreadScanPlaylistH
//---------------------------------------------------------------------------

class ThreadScanPlaylist;

#include "ClassCore.h"

//---------------------------------------------------------------------------
class ThreadScanPlaylist : public TThread {
private:
	XCore      *Core;
	TStringList *PFile;
	unsigned    PLevel;

	void __fastcall Done(void);
	void __fastcall Abort(void);
	void __fastcall PopulateAllFolders(TTreeView *tv, TTreeNode *rootNode);

protected:
	void __fastcall Execute();
public:
	__fastcall ThreadScanPlaylist(XCore *core, bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif