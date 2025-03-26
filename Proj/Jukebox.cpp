//---------------------------------------------------------------------------
#include <vcl.h>
#include <tchar.h>
#pragma hdrstop

//---------------------------------------------------------------------------
#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>

USEFORM("..\Src\FicheBoxmin.cpp", Mini);
USEFORM("..\Src\FicheMain.cpp", Main);
USEFORM("..\Src\FicheOptions.cpp", Options);
//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
		::SetLastError(NO_ERROR);
		CreateMutex(NULL, false, L"Jukebox");
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			Application->Initialize();
			Application->MainFormOnTaskBar = true;
			TStyleManager::TrySetStyle("Cobalt XEMedia");
			Application->CreateForm(__classid(TMain), &Main);
		Application->CreateForm(__classid(TOptions), &Options);
		Application->CreateForm(__classid(TMini), &Mini);
		Application->Run();
		}
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
