//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

class TBoxmin;

#include "FicheBoxmin.h"
#include "Boxon.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TBoxmin *Boxmin;
//---------------------------------------------------------------------------
__fastcall TBoxmin::TBoxmin(TComponent* Owner) : TForm(Owner)
{
	Core = Boxon->Core;

	Top  = Core->Intf->MinTop;
	Left = Core->Intf->MinLeft;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void __fastcall TBoxmin::SetTitle(UnicodeString title)
{
	if (Label1->Caption != title) {
		Label1->Caption = title;
		Width = Label1->Width + 25;
	}
}

void __fastcall TBoxmin::Label1Click(TObject *Sender)
{
	Core->Mix->NextNow();
}
//---------------------------------------------------------------------------

void __fastcall TBoxmin::FormResize(TObject *Sender)
{
	if ( WindowState == wsMaximized ) {
		Boxmin->Hide();
		Boxon->Show();
		return;
	}
}


void __fastcall TBoxmin::FormShow(TObject *Sender)
{
	WindowState = wsNormal;
	if (Boxon != NULL) Boxon->Hide();
}
//---------------------------------------------------------------------------


void __fastcall TBoxmin::FormClose(TObject *Sender, TCloseAction &Action)
{
	Boxon->Show();
}
//---------------------------------------------------------------------------

