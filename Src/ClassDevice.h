//---------------------------------------------------------------------------

#ifndef ClassDeviceH
#define ClassDeviceH

class ClassDevice;

#include "ClassCore.h"

//---------------------------------------------------------------------------
class ClassDevice {
	XCore *Core;

public:

	__fastcall ClassDevice(XCore *core);
	__fastcall ~ClassDevice(void);

	void __fastcall List(void);
};
#endif
