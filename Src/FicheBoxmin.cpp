//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#pragma package(smart_init)
#pragma resource "*.dfm"

class TMini;

#include "FicheBoxmin.h"

//---------------------------------------------------------------------------

TMini *Mini;

//---------------------------------------------------------------------------
__fastcall TMini::TMini(TComponent* Owner) : TForm(Owner)
{
	Core = Main->Core;

	Top  = Core->Intf->MinTop;
	Left = Core->Intf->MinLeft;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void __fastcall TMini::SetTitle(UnicodeString title)
{
	if (Label1->Caption != title) {
		Label1->Caption = title;
		Width = Label1->Width + 25;
	}
}

void __fastcall TMini::Label1Click(TObject *Sender)
{
	Core->Mix->NextNow();
}
//---------------------------------------------------------------------------

void __fastcall TMini::FormResize(TObject *Sender)
{
	if ( WindowState == wsMaximized ) {
		Hide();
		Main->Show();
		return;
	}
}


void __fastcall TMini::FormShow(TObject *Sender)
{
	WindowState = wsNormal;
	if (Main != NULL) Main->Hide();
}
//---------------------------------------------------------------------------


void __fastcall TMini::FormClose(TObject *Sender, TCloseAction &Action)
{
	Main->Show();
}
//---------------------------------------------------------------------------

