//---------------------------------------------------------------------------
#ifndef ClassSpectrumH
#define ClassSpectrumH

class XSpectrum;

#include "ClassMix.h"

// Énumération pour les différents modes d'affichage du spectre
enum class SpectrumMode {
    Oscilloscope,
    Bar
};

//---------------------------------------------------------------------------
class XSpectrum {
private:
    XMix         *Mix;        // Référence au mixeur audio
    TImage       *PImage;     // Image cible pour l'affichage
    TBitmap      *PBkg;       // Bitmap de fond pour le double buffering
    SpectrumMode PMode;       // Mode d'affichage actuel
    float        PPic[128];   // Tableau pour stocker les valeurs de pic
    bool         PNeedRedraw; // Indicateur de besoin de redessiner
    int          PBarWidth;   // Largeur des barres en mode Bar
    int          PBarSpacing; // Espacement entre les barres en mode Bar

    // Méthode pour définir l'image cible
    void __fastcall SetImage(TImage *image);

    // Méthode pour définir le mode d'affichage
    void __fastcall SetMode(SpectrumMode mode);

    // Méthode interne pour dessiner le spectre
    void __fastcall DrawSpectrum(void);

public:
    // Constructeur et destructeur
	XSpectrum(XMix *mix);
    ~XSpectrum(void);

    // Méthodes publiques
    void __fastcall Draw(void);
    void __fastcall DrawOscilo(void);
    void __fastcall DrawBar(void);
    void __fastcall Reset(void);

    // Méthodes pour configurer l'apparence des barres
    void __fastcall SetBarWidth(int width);
    void __fastcall SetBarSpacing(int spacing);

    // Propriétés
    __property TImage      *Image      = { read = PImage,      write = SetImage };
    __property SpectrumMode Mode       = { read = PMode,       write = SetMode };
    __property bool         NeedRedraw = { read = PNeedRedraw, write = PNeedRedraw };
    __property int          BarWidth   = { read = PBarWidth,   write = SetBarWidth };
    __property int          BarSpacing = { read = PBarSpacing, write = SetBarSpacing };
};
#endif
