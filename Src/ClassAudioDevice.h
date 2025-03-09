//---------------------------------------------------------------------------

#ifndef ClassAudioDeviceH
#define ClassAudioDeviceH

class ClassAudioDevice;

#include "ClassCore.h"

//---------------------------------------------------------------------------
class ClassAudioDevice {
	XCore *Core;

public:

	__fastcall ClassAudioDevice(XCore *core);
	__fastcall ~ClassAudioDevice(void);

	void __fastcall List(void);
};
#endif
