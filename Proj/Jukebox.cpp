//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
USEFORM("..\Src\Boxon.cpp", Boxon);
USEFORM("..\Src\FicheBoxmin.cpp", Boxmin);
USEFORM("..\Src\FicheOptions.cpp", Options);
//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
		::SetLastError(NO_ERROR);
		//CreateMutex(NULL, false, L"Boxon");
		//if (GetLastError() != ERROR_ALREADY_EXISTS) {
			Application->Initialize();
			Application->MainFormOnTaskBar = true;
			//TStyleManager::TrySetStyle("..\\Style\\Cobalt XEMedia");
			Application->CreateForm(__classid(TBoxon), &Boxon);
		Application->CreateForm(__classid(TOptions), &Options);
		Application->CreateForm(__classid(TBoxmin), &Boxmin);
		Application->Run();
        //}
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
