//---------------------------------------------------------------------------

#ifndef ClassDropFolderH
#define ClassDropFolderH

#include <System.Classes.hpp>
#include <Vcl.ComCtrls.hpp>

#include "ClassCore.h"

//---------------------------------------------------------------------------
class XDropFolder : public TPanel {
 protected:
	void __fastcall WMDropFiles(TMessage msg);

	BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_DROPFILES,TMessage,WMDropFiles)
	END_MESSAGE_MAP (TControl);

 public:
	XCore *Core;
	TImage *Img;
	__fastcall XDropFolder(TComponent *owner);
	__fastcall ~XDropFolder(void);

};
#endif
