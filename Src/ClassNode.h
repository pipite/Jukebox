//---------------------------------------------------------------------------

#ifndef ClassNodeH
#define ClassNodeH
//---------------------------------------------------------------------------

class XNode;

#include "ClassCore.h"

enum NODETYPE { ROOT, FOLDER, SONG, SONGLINK, ALPHA };


//---------------------------------------------------------------------------
class XNode {
	XNode            *PParent;
	XNode            *PLinkNode;
	int               PNodeType;
	unsigned 		  PCountFolder;
	unsigned          PCountSong;
	unsigned          PCountSongAll;
	unsigned          PLevel;
	// SongOnly
	bool              PWaveLoaded;
	bool              PWaveLoading;
	bool              PIsSort;
	unsigned          PCountPlay;
	unsigned          PCountPlayList;
	unsigned          PCountSession;
	UnicodeString     PPath;
	DWORD            *PWave;
	TBitmap          *PBitmap;
	unsigned __int64  PMsFadeIn;
	unsigned __int64  PMsFadeOut;
	unsigned __int64  PMsLength;

	void          __fastcall UpdateSongAll(int nbsong);
	void          __fastcall Init(void);

	bool 		  __fastcall GetHaveParent(void);
	bool 		  __fastcall GetIsLink(void);
	void          __fastcall SetParent(XNode *dest);
	UnicodeString __fastcall GetPath(void);
	UnicodeString __fastcall GetFileName(void);
	int           __fastcall GetNodeType(void);
	DWORD*        __fastcall GetWave(void);
	TBitmap*      __fastcall GetBitmap(void);
	bool          __fastcall GetWaveLoaded(void);
	void          __fastcall SetWaveLoaded(bool waveloaded);
	bool          __fastcall GetWaveLoading(void);
	void          __fastcall SetWaveLoading(bool waveloading);
	int           __fastcall GetMsFadeIn(void);
	void          __fastcall SetMsFadeIn(int msfadein);
	int           __fastcall GetMsFadeOut(void);
	void          __fastcall SetMsFadeOut(int msfadeout);
	int           __fastcall GetMsLength(void);
	void          __fastcall SetMsLength(int mslength);
	unsigned      __fastcall GetSongIndex(void);
	XNode*        __fastcall GetSongNode(void);
	XNode*        __fastcall GetRandomChild(void);
	bool          __fastcall GetHaveChild(void);
	bool          __fastcall GetHaveSong(void);
	unsigned      __fastcall GetCountChild(void);
	unsigned      __fastcall GetCountPlay(void);
	void          __fastcall SetCountPlay(unsigned value);
	unsigned      __fastcall GetCountPlayList(void);
	void          __fastcall SetCountPlayList(unsigned value);
	unsigned      __fastcall GetCountSession(void);
	void          __fastcall SetCountSession(unsigned value);
	unsigned      __fastcall GetIndex(void);
	bool          __fastcall GetIsRoot(void);
	bool          __fastcall GetIsFolder(void);
	bool          __fastcall GetIsSong(void);

public:
	bool   ShowWave;
	DWORD  Peak;
	TList *Child;
	bool   Populated;

				  __fastcall XNode(XNode *parent, UnicodeString path, int nodetype);
				  __fastcall XNode(XNode *parent, XNode *link);
				  __fastcall ~XNode(void);

	bool 		  __fastcall HasAsParent(XNode *node);
	void          __fastcall Sort(void);
	void          __fastcall ResetPopulated(void);

	XNode*        __fastcall ChildAt(unsigned i);
	XNode*        __fastcall Find(UnicodeString path);

	XNode*        __fastcall Add(UnicodeString path, int nodetype);
	XNode*        __fastcall Add(XNode *node);
	XNode*        __fastcall Link(XNode *songnode);
	XNode*        __fastcall CopyTo(XNode *nodedest);
	void          __fastcall MoveTo(XNode *dest);
	void          __fastcall Clear(void);

	void          __fastcall DrawWave(TImage *image);


	__property bool          HaveParent     = {read = GetHaveParent};
	__property XNode        *Parent 	    = {read = PParent,          write = SetParent};
	__property XNode        *RandomChild 	= {read = GetRandomChild};
	__property XNode        *SongNode    	= {read = GetSongNode};
	__property unsigned      Index 		 	= {read = GetIndex};
	__property unsigned      Level 		 	= {read = PLevel};
	__property unsigned      CountChild 	= {read = GetCountChild};
	__property unsigned      CountFolder 	= {read = PCountFolder};
	__property unsigned      CountSong 	 	= {read = PCountSong};
	__property unsigned      CountSongAll 	= {read = PCountSongAll};
	__property bool          HaveChild		= {read = GetHaveChild};
	__property bool          HaveSong 		= {read = GetHaveSong};
	__property bool          IsRoot 		= {read = GetIsRoot};
	__property bool          IsFolder 		= {read = GetIsFolder};
	__property bool          IsSong 	   	= {read = GetIsSong};
	__property bool          IsLink 	   	= {read = GetIsLink};

	__property int           NodeType		= {read = GetNodeType,      write = PNodeType};
	__property UnicodeString Path			= {read = GetPath,          write = PPath};
	__property UnicodeString FileName		= {read = GetFileName};
	__property int           MsFadeIn		= {read = GetMsFadeIn,      write = SetMsFadeIn};
	__property int           MsFadeOut		= {read = GetMsFadeOut,     write = SetMsFadeOut};
	__property int           MsLength		= {read = GetMsLength,      write = SetMsLength};
	__property unsigned      CountPlay		= {read = GetCountPlay,     write = SetCountPlay};
	__property unsigned      CountPlayList	= {read = GetCountPlayList, write = SetCountPlayList};
	__property unsigned      CountSession	= {read = GetCountSession,  write = SetCountSession};
	__property bool          WaveLoaded		= {read = GetWaveLoaded,    write = SetWaveLoaded};
	__property bool          WaveLoading	= {read = GetWaveLoading,   write = SetWaveLoading};
	__property DWORD        *Wave			= {read = GetWave};
	__property TBitmap      *Bitmap			= {read = GetBitmap};
};
#endif

