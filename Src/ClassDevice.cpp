//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassDevice.h"
#include <MMsystem.h>
//---------------------------------------------------------------------------

#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   ClassDevice
// ---------------------------------------------------------------------------
__fastcall ClassDevice::ClassDevice(XCore *core) {
	Core        = core;
}

__fastcall ClassDevice::~ClassDevice(void) {
}

void __fastcall ClassDevice::List(void) {
	unsigned int i;
	WAVEOUTCAPS outcaps;

	for (i = 0; i < waveOutGetNumDevs(); i++) {
		waveOutGetDevCaps(i, &outcaps, sizeof(outcaps));
		//PCore->Form->Memo1->Lines->Add(UnicodeString(i) + L" = " + UnicodeString(outcaps.szPname));
	}
}

