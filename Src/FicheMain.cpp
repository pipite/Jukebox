//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop
#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
#include "FicheMain.h"

using namespace std;

TMain *Main;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TMain::TMain(TComponent* Owner) : TForm(Owner)
{
    // Initialisation des variables
    Created = false;
    T1 = false;
    T2 = false;

    try {
        // Création des objets principaux
        Intf = new XIntf(this);
        Core = new XCore(Intf);
        Intf->Core = Core;

        // Vérification de la création du Core
        if (!Core->Created) return;

        // Configuration de l'interface
        Intf->ImgDropFolder->Core = Core;

        // Initialisation des dimensions de référence
        // S'assurer que les valeurs sont positives et non nulles
        OWidth = max(100, Width);
        OHeight = max(100, Height);

        // Configuration du volume
        SbVolume->Position = Core->Settings->Volume;

        // Assignation de l'événement de redimensionnement
        OnResize = FormResize;

        // Marquer l'initialisation comme terminée
        Created = true;
    }
    catch (...) {
        // Nettoyage en cas d'exception
        if (Core) {
            delete Core;
            Core = NULL;
        }
        if (Intf) {
            delete Intf;
            Intf = NULL;
        }
        throw; // Relancer l'exception
    }
}

void __fastcall TMain::FormClose(TObject *Sender, TCloseAction &Action)
{
	Caption = L"   Freeing Memory . . .";
	Core->Mix->Stop();
	DisableTimer();
	Core->Settings->Save();
	Core->SavFile->Save();

	if (Core != NULL) delete Core;
	if (Intf != NULL) delete Intf;
}

void __fastcall TMain::Panel_InitStart(void) {
	PanelInterface->Visible = false;
	PanelInterface->Align = alNone;
}

void __fastcall TMain::DisableTimer(void) {
	if (TimerLoop->Enabled) T1 = true;
	if (TimerTile->Enabled) T2 = true;
}

//---------------------------------------------------------------------------
//      Timer
//---------------------------------------------------------------------------
void __fastcall TMain::TimerLoopTimer(TObject *Sender)
{
	UnicodeString rep, song, title;

	if (T1) { TimerLoop->Enabled = T1 = false; return; }

	try {
		if (Core->TvNext->NR->CountSongAll == 0 ) return;
		Core->Mix->Auto();
		PbPosition->Position = (int) ( (double) (Core->Mix->Position * PbPosition->Max / Core->Mix->Current->MsLength) );

		if (Core->Mix->Current->Node != NULL) {
			rep   = Core->Mix->Current->Node->Path;
			song  = ExtractFileName(rep);
			rep   = ExtractFileName(rep.SubString(1,rep.Length()-1-song.Length()));
			int i = ExtractFileExt(song).Length();
			title = "     [" + rep + "] - " + song.SubString(1,song.Length()-i) + " (pic = " + UnicodeString(WORD(Core->Mix->Current->Node->Peak)) + ")";
			Caption = title;
			Mini->Title = title;
		}
	} catch (...) {
		TimerLoop->Enabled = false;
	}
}

void __fastcall TMain::TimerTileTimer(TObject *Sender)
{
	TDateTime duree, reste;

	if (T2) { TimerTile->Enabled = T2 = false; return; }

	try {
		if (Core->Mix->Current == NULL) return;
		duree = IncMilliSecond(TDateTime(0),Core->Mix->Current->MsLength);
		reste = IncMilliSecond(TDateTime(0),Core->Mix->Current->MsLength - Core->Mix->Current->Position);
		LbTimeLeft->Caption = FormatDateTime(L"nn:ss",reste);
	} catch (...) {
		TimerTile->Enabled = false;
	}
}

//---------------------------------------------------------------------------
//      Panel Interface
//---------------------------------------------------------------------------
void __fastcall TMain::ToolButton2Click(TObject *Sender)
{
	Mini->Show();
}

void __fastcall TMain::FormResize(TObject *Sender)
{
    // Protection contre les appels récursifs
    static bool isResizing = false;
    if (isResizing) return;
    isResizing = true;

    try {
        static bool max;
        static int LWidth, oLWidth;
        static int RWidth, oRWidth;
        static int WHeight, oWHeight;
        static int WWidth, oWWidth;
        static int NHeight, oNHeight;
        static int PHeight, oPHeight;

        // Si on revient d'un état maximisé, restaurer les dimensions précédentes
        if (max) {
            max = false;
            PanelList->Width = LWidth;
            PanelRight->Width = RWidth;
            PanelNext->Height = NHeight;
            PanelPast->Height = PHeight;
            Width = WWidth;
            Height = WHeight;
            Panel_FontSize();
            isResizing = false;
            return;
        }

        // Si on passe à l'état maximisé, sauvegarder les dimensions actuelles
        if (WindowState == wsMaximized) {
            LWidth = oLWidth;
            RWidth = oRWidth;
            WHeight = oWHeight;
            WWidth = oWWidth;
            NHeight = oNHeight;
            PHeight = oPHeight;
            max = true;
        }

        // Ajuster les proportions des panneaux
        Panel_Proportion();

        // Ajuster la taille de la police
        Panel_FontSize();

        // Sauvegarder les dimensions actuelles
        oLWidth = PanelList->Width;
        oRWidth = PanelRight->Width;
        oWWidth = Width;
        oWHeight = Height;
        oNHeight = PanelNext->Height;
        oPHeight = PanelPast->Height;
    }
    catch (...) {
        // Ignorer les exceptions potentielles
    }

    isResizing = false;
}

void __fastcall TMain::Panel_Proportion(void) {
    // Vérifications préliminaires
    if (!Created) return;
    if (!Core->Settings->PanelResize) return;

    // Constantes pour les limites de redimensionnement
    const int MIN_PANEL_WIDTH = 100;
    const int MAX_PANEL_WIDTH = ClientWidth - 200;
    const int MIN_PANEL_HEIGHT = 50;
    const int MAX_PANEL_HEIGHT = ClientHeight - 100;

    // Redimensionnement horizontal
    if (OWidth != Width && OWidth > 0) {
        // Calcul du ratio de redimensionnement avec protection contre la division par zéro
        float px = float(Width) / float(OWidth);

        // Limiter le ratio pour éviter les changements trop brusques
        px = max(0.5f, min(2.0f, px));

        // Calculer la nouvelle largeur avec arrondi
        int newWidth = int(PanelList->Width * px + 0.5);

        // Appliquer les limites
        newWidth = max(MIN_PANEL_WIDTH, min(MAX_PANEL_WIDTH, newWidth));

        // Appliquer la nouvelle largeur
        PanelList->Width = newWidth;

        // Mettre à jour la largeur de référence
        OWidth = Width;
    }

    // Redimensionnement vertical
    if (OHeight != Height && OHeight > 0) {
        // Calcul du ratio de redimensionnement avec protection contre la division par zéro
        float py = float(Height) / float(OHeight);

        // Limiter le ratio pour éviter les changements trop brusques
        py = max(0.5f, min(2.0f, py));

        // Calculer les nouvelles hauteurs avec arrondi
        int newNextHeight = int(PanelNext->Height * py + 0.5);
        int newPastHeight = int(PanelPast->Height * py + 0.5);

        // Appliquer les limites
        newNextHeight = max(MIN_PANEL_HEIGHT, min(MAX_PANEL_HEIGHT, newNextHeight));
        newPastHeight = max(MIN_PANEL_HEIGHT, min(MAX_PANEL_HEIGHT, newPastHeight));

        // Appliquer les nouvelles hauteurs
        PanelNext->Height = newNextHeight;
        PanelPast->Height = newPastHeight;

        // Mettre à jour la hauteur de référence
        OHeight = Height;
    }
}

void __fastcall TMain::Panel_FontSize(void) {
	static int w = Width;
	int fontsize;

	fontsize = max(Core->Settings->FontSizeMin ,int (Width/110) + 1);
	fontsize = min(Core->Settings->FontSizeMax ,fontsize);
	Core->FontSize = fontsize;
}

void __fastcall TMain::Panel9Resize(TObject *Sender)
{
	Core->Mix->Wave->NeedRefresh = true;
}

//---------------------------------------------------------------------------
//      Button
//-------------------------------------------------------------------------

void __fastcall TMain::TbScanClick(TObject *Sender)
{
	Options->Visible = !Options->Visible;
}
void __fastcall TMain::TbArboClick(TObject *Sender)
{
	Core->TvList->RollArboFlat();
}

void __fastcall TMain::TbAzClick(TObject *Sender)
{
	Core->TvList->RollAz();
}

void __fastcall TMain::TbStopPlayClick(TObject *Sender)
{
	Core->Intf->StopPlay();
}

void __fastcall TMain::TbMixNextNowClick(TObject *Sender)
{
	TbStopPlay->ImageIndex = 0;
	Core->Mix->NextNow();
}

void __fastcall TMain::TbRegenClick(TObject *Sender)
{
	Core->TvNext->Shuffle();
}

void __fastcall TMain::TbTrashClick(TObject *Sender)
{
	Core->TvNext->Randomize();
}

void __fastcall TMain::ToolButton4Click(TObject *Sender)
{
	Core->TvPlayList->AddPlayList();
}

//---------------------------------------------------------------------------
//          Interface
//---------------------------------------------------------------------------
void __fastcall TMain::SbVolumeChange(TObject *Sender)
{
	Core->Volume = 65535 - SbVolume->Position;
}

void __fastcall TMain::PbPositionMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
	Core->Mix->Current->Position = (int) ( (double) (X * Core->Mix->Current->MsLength / PbPosition->Width) );
}

//---------------------------------------------------------------------------
//               DRAG DROP Corbeille
//---------------------------------------------------------------------------
void __fastcall TMain::TbTrashDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	XNode *n;
	if (Source->InheritsFrom(__classid(XTvNext))) {
		TTreeNode *source = ((XTvNext*)Source)->Selected;
		if (source != NULL) {
			if ( ( Core->TvNext->GetNode(source)->NodeType == NODETYPE::SONG ) || ( Core->TvNext->GetNode(source)->NodeType == NODETYPE::FOLDER ) ){
				Accept = true;
				return;
			}
		}
	}
	if (Source->InheritsFrom(__classid(XTvPlayList))) {
		TTreeNode *source = ((XTvPlayList*)Source)->Selected;
		if (source != NULL) {
			if ( ( Core->TvPlayList->GetNode(source)->NodeType == NODETYPE::SONG ) || ( Core->TvPlayList->GetNode(source)->NodeType == NODETYPE::FOLDER ) ){
				Accept = true;
				return;
			}
		}
	}
	Accept = false;
}

void __fastcall TMain::TbTrashDragDrop(TObject *Sender, TObject *Source, int X, int Y)
{
	TTreeNode *source;
	unsigned i;

	if (Source->InheritsFrom(__classid(XTvNext))) {
		Core->TvNext->Items->BeginUpdate();
		for ( i = ((XTvNext*)Source)->SelectionCount; i > 0  ; i--) {
			source = ((XTvNext*)Source)->Selections[i-1];
			if (source == NULL) return;
			Core->TvNext->Remove(source);
			Core->Mix->Next->Node = Core->TvNext->Next;
		}
		Core->TvNext->Fill();
		Core->TvNext->Items->EndUpdate();
	}
	if (Source->InheritsFrom(__classid(XTvPlayList))) {
		Core->TvPlayList->Items->BeginUpdate();
		for ( i = ((XTvNext*)Source)->SelectionCount; i > 0  ; i--) {
			source = ((XTvPlayList*)Source)->Selections[i-1];
			if (source == NULL) return;
			Core->TvPlayList->Remove(source);
		}
		Core->TvPlayList->Items->EndUpdate();
	}
}

//---------------------------------------------------------------------------
//               DRAG DROP Search
//---------------------------------------------------------------------------
void __fastcall TMain::TbAzDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept) {
	XNode *n;
	if (Source->InheritsFrom(__classid(XTvNext))) {
		TTreeNode *source = ((XTvNext*)Source)->Selected;
		if (source != NULL) {
			if (Core->TvNext->GetNode(source)->NodeType == NODETYPE::SONG ) {
				Accept = true;
				return;
			}
		}
	}
	Accept = false;
}

void __fastcall TMain::TbAzDragDrop(TObject *Sender, TObject *Source, int X, int Y)
{
	TTreeNode *source = ((XTvNext*)Source)->Selected;
	if (source == NULL) return;
}

//---------------------------------------------------------------------------

void __fastcall TMain::Splitter2Moved(TObject *Sender)
{
	ShowScrollBar(Core->TvNext->Handle, SB_HORZ, false);
	ShowScrollBar(Core->TvPlayList->Handle, SB_HORZ, false);
}
//---------------------------------------------------------------------------

void __fastcall TMain::Splitter3Moved(TObject *Sender)
{
	ShowScrollBar(Core->TvPast->Handle, SB_HORZ, false);
	ShowScrollBar(Core->TvPlayList->Handle, SB_HORZ, false);
}
//---------------------------------------------------------------------------


void __fastcall TMain::Splitter1Moved(TObject *Sender)
{
	ShowScrollBar(Core->TvNext->Handle, SB_HORZ, false);
	ShowScrollBar(Core->TvPlayList->Handle, SB_HORZ, false);
	ShowScrollBar(Core->TvList->Handle, SB_HORZ, false);
	ShowScrollBar(Core->TvPast->Handle, SB_HORZ, false);
}
//---------------------------------------------------------------------------

void __fastcall TMain::Label1Click(TObject *Sender)
{
//    Core->Mix->ToggleNormalize();
}
//---------------------------------------------------------------------------

