//---------------------------------------------------------------------------
#ifndef ClassLoaderH
#define ClassLoaderH

class XLoader;

#include "ClassCore.h"
#include "ThreadFastScan.h"

//---------------------------------------------------------------------------
class XLoader {
	XCore *Core;

public:
	bool  Loaded;
	bool  Loading;

	bool  SavePlayListNeeded;
	bool  SaveArboNeeded;

	XLoader(XCore *core);
	~XLoader(void);

	void __fastcall Load(void);
};

#endif
