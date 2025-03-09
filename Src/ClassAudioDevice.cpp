//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassAudioDevice.h"
#include <MMsystem.h>
//---------------------------------------------------------------------------

#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   ClassAudioDevice
// ---------------------------------------------------------------------------
__fastcall ClassAudioDevice::ClassAudioDevice(XCore *core) {
	Core        = core;
}

__fastcall ClassAudioDevice::~ClassAudioDevice(void) {
}

void __fastcall ClassAudioDevice::List(void) {
	unsigned int i;
	WAVEOUTCAPS outcaps;

	for (i = 0; i < waveOutGetNumDevs(); i++) {
		waveOutGetDevCaps(i, &outcaps, sizeof(outcaps));
		//PCore->Form->Memo1->Lines->Add(UnicodeString(i) + L" = " + UnicodeString(outcaps.szPname));
	}
}

