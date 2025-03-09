//---------------------------------------------------------------------------

#ifndef ClassSpectrumH
#define ClassSpectrumH

class XSpectrum;

#include "ClassMix.h"

//---------------------------------------------------------------------------
class XSpectrum {
	XMix     *Mix;
	TImage   *PImage;
	TBitmap  *PBkg;

	void __fastcall SetImage(TImage *image);

public:

	 __fastcall XSpectrum(XMix *mix);
	 __fastcall ~XSpectrum(void);

void __fastcall Draw(void);
void __fastcall DrawOscilo(void);
void __fastcall DrawBar(void);

	__property TImage   *Image = { read = PImage,    write = SetImage};
};
#endif
