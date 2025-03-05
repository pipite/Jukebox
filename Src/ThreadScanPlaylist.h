//---------------------------------------------------------------------------

#ifndef ThreadScanPlaylistH
#define ThreadScanPlaylistH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>

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

protected:
	void __fastcall Execute();
public:
	__fastcall ThreadScanPlaylist(XCore *core, bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
