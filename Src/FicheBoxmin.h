//---------------------------------------------------------------------------
#ifndef FicheBoxminH
#define FicheBoxminH

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>

class TMini;

#include "ClassCore.h"


//---------------------------------------------------------------------------
class TMini : public TForm
{
	__published:	// Composants gérés par l'EDI
		TLabel *Label1;
		void __fastcall Label1Click(TObject *Sender);
		void __fastcall FormResize(TObject *Sender);
		void __fastcall FormShow(TObject *Sender);
		void __fastcall FormClose(TObject *Sender, TCloseAction &Action);

	private:	// Déclarations utilisateur
		XCore *Core;

		void __fastcall SetTitle(UnicodeString title);

	public:		// Déclarations utilisateur
		__fastcall TMini(TComponent* Owner);

		__property UnicodeString Title = {write = SetTitle};
};
//---------------------------------------------------------------------------
extern PACKAGE TMini *Mini;
//---------------------------------------------------------------------------
#endif
