//---------------------------------------------------------------------------

#ifndef ThreadScanDiskH
#define ThreadScanDiskH
//---------------------------------------------------------------------------

class ThreadScanDisk;

#include "ClassCore.h"
#include "ThreadScanPlaylist.h"
#include "ClassNode.h"

//---------------------------------------------------------------------------
class ThreadScanDisk : public TThread {
	XCore        *Core;
	UnicodeString Folder;
	int           Level;
	unsigned      c;


	void __fastcall RecurseScan(XNode *arbofolder, XNode *flatfolder, XNode *flatazfolder, XNode *arboazfolder);
	void __fastcall SetFolder(UnicodeString path);

	void __fastcall Execute(void);
	void __fastcall Done(void);
	void __fastcall Abort(void);
	void __fastcall TileInfo(void);

public:

		 __fastcall ThreadScanDisk(XCore *core, bool CreateSuspended);

};
#endif
