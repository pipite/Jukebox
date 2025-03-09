//---------------------------------------------------------------------------

#ifndef ClassSavFileH
#define ClassSavFileH

#include <fstream>

class XFile;

#include "ClassCore.h"

//---------------------------------------------------------------------------
class XFile {
	XCore   *Core;
   	ifstream Pf;
	TStringList *PFile;
	unsigned    PLevel;

	UnicodeString 	__fastcall GetPathArbo(void);
	UnicodeString 	__fastcall GetPathPlaylist(void);
	bool 			__fastcall GetHaveArbo(void);
	bool 			__fastcall GetHavePlaylist(void);
	UnicodeString   __fastcall GetExtendDate(void);
	void 			__fastcall MoveToArchive(UnicodeString src);

	void 			__fastcall SaveRecurseNode(XNode *node);
	void 			__fastcall ArchiveArbo(void);
	void 			__fastcall RemoveRootPath(bool bypass);

	public:
					__fastcall XFile(XCore *core);
					__fastcall ~XFile(void);

	//void 			__fastcall ConvertArbo(void);
	bool 			__fastcall ConvertPlaylist(void);
	void			__fastcall Save(void);
	void 			__fastcall SavePlaylist(UnicodeString filename);
	void 			__fastcall ArchivePlayList(void);
	void 			__fastcall Purge(UnicodeString filename);

	__property UnicodeString PathArbo	  = {read = GetPathArbo};
	__property UnicodeString PathPlaylist = {read = GetPathPlaylist};
	__property bool          HaveArbo	  = {read = GetHaveArbo};
	__property bool          HavePlaylist = {read = GetHavePlaylist};
	__property UnicodeString ExtendDate   = {read = GetExtendDate};

};



#endif
