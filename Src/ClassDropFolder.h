//---------------------------------------------------------------------------
#ifndef ClassDropFolderH
#define ClassDropFolderH

#include "ClassCore.h"

//---------------------------------------------------------------------------
class XDropFolder : public TPanel {
 private:
    bool IsProcessing;  // Flag pour éviter les traitements multiples

 protected:
    void __fastcall WMDropFiles(TMessage msg);
    void __fastcall WMTimer(TMessage &msg);

    BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_DROPFILES, TMessage, WMDropFiles)
    MESSAGE_HANDLER(WM_TIMER, TMessage, WMTimer)
    END_MESSAGE_MAP (TPanel);

 public:
    XCore *Core;       // Référence au noyau de l'application
    TImage *Img;       // Image affichée dans le panneau

	XDropFolder(TComponent *owner);
    ~XDropFolder(void);

    // Méthode pour réinitialiser l'apparence
    void __fastcall ResetAppearance();
};
#endif
