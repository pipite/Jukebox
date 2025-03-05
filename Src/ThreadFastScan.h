//---------------------------------------------------------------------------

#ifndef ThreadFastScanH
#define ThreadFastScanH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>

class ThreadFastScan;

#include "ClassCore.h"
#include "ThreadScanPlayList.h"

//---------------------------------------------------------------------------
class ThreadFastScan : public TThread {
private:
	XCore      *Core;
	TStringList *PFile;
	unsigned    PLevel;

	void __fastcall FastLoadedtrue(void);
	void __fastcall Done(void);
	void __fastcall TileInfo(void);
	void __fastcall Abort(void);

protected:
	void __fastcall Execute();
public:
	__fastcall ThreadFastScan(XCore *core, bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
