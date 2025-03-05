//---------------------------------------------------------------------------

#pragma hdrstop


#include "ClassSpectrum.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   ClassSpectrum
// ---------------------------------------------------------------------------
__fastcall XSpectrum::XSpectrum(XMix *mix) {
	Mix    = mix;
	PBkg   = new TBitmap();
	PImage = NULL;
}

__fastcall XSpectrum::~XSpectrum(void) {
	delete PBkg;
};

void __fastcall XSpectrum::SetImage(TImage *image) {
	PImage        = image;
	PBkg->Width   = PImage->Width;
	PBkg->Height  = PImage->Height;
}

void __fastcall XSpectrum::Draw(void) {
	DrawOscilo();
}

void __fastcall XSpectrum::DrawOscilo(void) {
	static float pic[128];
	float *fftcur, *fftlast;
	float volcur, vollast;
	int i;
	float s, o;
	int h;
	TCanvas *c;
	float chute = 1.5;
	bool ismix;

	PBkg->Canvas->Pen->Color   = clGray;
	PBkg->Canvas->Brush->Color = clBlack;
	PBkg->Canvas->Rectangle(0, 0, PBkg->Width, PBkg->Height);
	PBkg->Canvas->Pen->Color   = clSkyBlue;

	try {
		if (Mix->Current->IsPlaying) {
			ismix   = Mix->IsMixing;
			fftcur  = Mix->Current->FFT;
			fftlast = Mix->Last->FFT;
			c = PBkg->Canvas;
			h = PBkg->Height;
			vollast = Mix->Last->Volume;
			volcur  = Mix->Current->Volume;
			for ( i = 1; i <= 126; i++) {
				s = fabs( fftcur[i + 5] ) * volcur;
				if (ismix) {
					s += fabs( fftlast[i + 5] ) * vollast;
					s /= 2;
				}
				if ( s > 0 ) s = sqrt(sqrt(s)) * h * 1.5;
				if ( s > h ) s = h;
				if ( s >= pic[i]-chute ) pic[i] = s; else pic[i] -= chute;
				if ( i > 1 ) {
					PBkg->Canvas->Pen->Color   = clYellow;
					c->MoveTo( i*2-2, h-o-1);
					c->LineTo( i*2  , h-s-1);
					PBkg->Canvas->Pen->Color   = clSkyBlue;
					c->MoveTo( i*2-2, h-pic[i-1]-1);
					c->LineTo( i*2  , h-pic[i]-1);
				}
				o = s;
			}
		}
		BitBlt(PImage->Canvas->Handle, 0, 0, PBkg->Width, PBkg->Height, PBkg->Canvas->Handle, 0, 0, SRCCOPY);
		PImage->Refresh();
	} catch (...) {};
}

void __fastcall XSpectrum::DrawBar(void) {
	static float pic[128];
	float *fftcur, *fftlast;
	float volcur, vollast;
	int i;
	float s, o;
	int h;
	TCanvas *c;
	float chute = 1.5;
	bool ismix;

	PBkg->Canvas->Pen->Color   = clGray;
	PBkg->Canvas->Brush->Color = clBlack;
	PBkg->Canvas->Rectangle(0, 0, PBkg->Width, PBkg->Height);
	PBkg->Canvas->Pen->Color   = clSkyBlue;

	try {
		if (Mix->Current->IsPlaying) {
			ismix   = Mix->IsMixing;
			fftcur  = Mix->Current->FFT;
			fftlast = Mix->Last->FFT;
			c = PBkg->Canvas;
			h = PBkg->Height;
			vollast = Mix->Last->Volume;
			volcur  = Mix->Current->Volume;
			for ( i = 1; i <= 126; i++) {
				s = fabs( fftcur[i + 5] ) * volcur;
				if (ismix) {
					s  += fabs( fftlast[i + 5] ) * vollast;
					s /=2;
				}
				if ( s > 0 ) s = sqrt(sqrt(s)) * h * 1.5;
				if ( s > h ) s = h;
				if ( s >= pic[i]-chute ) pic[i] = s; else pic[i] -= chute;
				if ( i > 1 ) {
					PBkg->Canvas->Pen->Color   = clYellow;
					c->MoveTo( i*2-2, h-o-1);
					c->LineTo( i*2  , h-s-1);
					PBkg->Canvas->Pen->Color   = clSkyBlue;
					c->MoveTo( i*2-2, h-pic[i-1]-1);
					c->LineTo( i*2  , h-pic[i]-1);
				}
				o = s;
			}
		}
		BitBlt(PImage->Canvas->Handle, 0, 0, PBkg->Width, PBkg->Height, PBkg->Canvas->Handle, 0, 0, SRCCOPY);
		PImage->Refresh();
	} catch (...) {};
}





