//---------------------------------------------------------------------------
#ifndef ClassTreeViewPastH
#define ClassTreeViewPastH

class XTvPast;

#include "ClassCore.h"
#include "ClassTreeNodeTool.h"

// -----------------------------------------------------
// ClassTreeViewPast
// -----------------------------------------------------
class XTvPast : public TTreeView {
	// Membres de données
	XCore       *Core;      // Référence au noyau de l'application
	TTreeNode   *PTNR;      // Nœud racine visuel
	XTNodeTool  *PTNTool;   // Outil de manipulation des nœuds
	XNode       *PNR;       // Nœud racine de données

	// Gestionnaires d'événements
	void __fastcall ODblClick(TObject *Sender);
	void __fastcall OCustomDrawItem(TCustomTreeView *Sender, TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw);
	void __fastcall OMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall OResize(TObject *Sender);
	
	// Gestionnaires d'événements pour le glisser-déposer
	void __fastcall ODragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept);
	void __fastcall ODragDrop(TObject *Sender, TObject *Source, int X, int Y);
	void __fastcall OEndDrag(TObject *Sender, TObject *Target, int X, int Y);
	
	// Méthodes utilitaires
	void __fastcall SetIcon(TTreeNode *tn, int a, int b, int c);
	
	// Méthode utilitaire pour obtenir un nœud
	XNode* __fastcall GetNode(TTreeNode *tn);
public:
	TImageList *ILList;

	// Constructeur et destructeur
	XTvPast(TComponent *owner, XCore *core);
	~XTvPast(void);

	// Méthodes publiques
	void __fastcall Reset(void);
	void __fastcall Remove(TTreeNode *treenode);

	// Propriétés
	__property TTreeNode *TNR = {read = PTNR};
	__property XNode     *NR  = {read = PNR};

};
#endif
