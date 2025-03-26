//---------------------------------------------------------------------------
#ifndef ClassAudioDeviceH
#define ClassAudioDeviceH

class ClassAudioDevice;

#include "ClassCore.h"

//---------------------------------------------------------------------------
// Structure pour stocker les informations d'un périphérique audio
struct TAudioDevice {
	int ID;                  // Identifiant du périphérique
	UnicodeString Name;      // Nom du périphérique
	int Channels;            // Nombre de canaux supportés
	DWORD SupportedFormats;  // Formats audio supportés
	WORD DriverVersion;      // Version du pilote
};

//---------------------------------------------------------------------------
// Classe pour gérer les périphériques audio du système
class ClassAudioDevice {
private:
	XCore *PCore;                      // Pointeur vers le core de l'application
	std::vector<TAudioDevice> OutputDevices;  // Liste des périphériques de sortie
	std::vector<TAudioDevice> InputDevices;   // Liste des périphériques d'entrée
	int CurrentOutputDevice;           // Périphérique de sortie actuellement sélectionné
	int CurrentInputDevice;            // Périphérique d'entrée actuellement sélectionné

public:
	// Constructeur et destructeur
	ClassAudioDevice(XCore *core);
	~ClassAudioDevice(void);

	// Méthodes pour lister les périphériques
	int __fastcall ListOutputDevices(void);
	int __fastcall ListInputDevices(void);
	void __fastcall List(void);  // Pour compatibilité avec le code existant

	// Méthodes pour sélectionner les périphériques
	bool __fastcall SelectOutputDevice(int deviceID);
	bool __fastcall SelectInputDevice(int deviceID);

	// Méthodes pour obtenir des informations sur les périphériques
	bool __fastcall GetCurrentOutputDeviceInfo(TAudioDevice &deviceInfo);
	bool __fastcall GetCurrentInputDeviceInfo(TAudioDevice &deviceInfo);

	// Propriétés
	__property int OutputDeviceCount = { read = GetOutputDeviceCount };
	__property int InputDeviceCount = { read = GetInputDeviceCount };
	__property int CurrentOutput = { read = CurrentOutputDevice, write = SelectOutputDevice };
	__property int CurrentInput = { read = CurrentInputDevice, write = SelectInputDevice };

private:
	// Méthodes privées pour les propriétés
	int __fastcall GetOutputDeviceCount(void) { return OutputDevices.size(); }
	int __fastcall GetInputDeviceCount(void) { return InputDevices.size(); }
};
#endif
