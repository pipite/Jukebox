//---------------------------------------------------------------------------

#ifndef ClassLoaderH
#define ClassLoaderH

class XLoader;

#include "ClassCore.h"

//---------------------------------------------------------------------------
class XLoader {
	XCore *Core;

public:
	bool  Loaded;
	bool  Loading;

	bool  SavePlayListNeeded;
	bool  SaveArboNeeded;

	__fastcall XLoader(XCore *core);
	__fastcall ~XLoader(void);

	void __fastcall Load(void);
};


class ThreadLoader : public TThread {
private:
	XCore      *Core;

protected:
	void __fastcall Execute();
public:
	__fastcall ThreadLoader(XCore *core, bool CreateSuspended);
};

#endif
