//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassAudioDevice.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   ClassAudioDevice
//   Classe pour gérer les périphériques audio du système
// ---------------------------------------------------------------------------
__fastcall ClassAudioDevice::ClassAudioDevice(XCore *core) {
	PCore = core;
	CurrentOutputDevice = 0;  // Périphérique par défaut
	CurrentInputDevice = 0;   // Périphérique par défaut
}

__fastcall ClassAudioDevice::~ClassAudioDevice(void) {
	// Rien à libérer pour l'instant
}

// ---------------------------------------------------------------------------
// Liste tous les périphériques de sortie audio disponibles
// Retourne le nombre de périphériques trouvés
// ---------------------------------------------------------------------------
int __fastcall ClassAudioDevice::ListOutputDevices(void) {
	unsigned int i;
	WAVEOUTCAPS outcaps;
	MMRESULT result;
	int deviceCount = waveOutGetNumDevs();
	
	// Vider la liste des périphériques
	OutputDevices.clear();
	
	// Parcourir tous les périphériques de sortie
	for (i = 0; i < deviceCount; i++) {
		result = waveOutGetDevCaps(i, &outcaps, sizeof(outcaps));
		if (result == MMSYSERR_NOERROR) {
			// Ajouter le périphérique à la liste
			TAudioDevice device;
			device.ID = i;
			device.Name = UnicodeString(outcaps.szPname);
			device.Channels = outcaps.wChannels;
			device.SupportedFormats = outcaps.dwFormats;
			device.DriverVersion = outcaps.vDriverVersion;
			OutputDevices.push_back(device);
			
			// Afficher dans le Memo si disponible
//			if (PCore && PCore->Form && PCore->Form->Memo1) {
//				PCore->Form->Memo1->Lines->Add(UnicodeString(i) + L" = " + device.Name);
//			}
		}
		else {
			// Gérer l'erreur
			UnicodeString errorMsg = L"Erreur lors de l'obtention des informations du périphérique " + UnicodeString(i);
//			if (PCore && PCore->Form && PCore->Form->Memo1) {
//				PCore->Form->Memo1->Lines->Add(errorMsg);
//			}
		}
	}
	
	return OutputDevices.size();
}

// ---------------------------------------------------------------------------
// Liste tous les périphériques d'entrée audio disponibles
// Retourne le nombre de périphériques trouvés
// ---------------------------------------------------------------------------
int __fastcall ClassAudioDevice::ListInputDevices(void) {
	unsigned int i;
	WAVEINCAPS incaps;
	MMRESULT result;
	int deviceCount = waveInGetNumDevs();
	
	// Vider la liste des périphériques
	InputDevices.clear();
	
	// Parcourir tous les périphériques d'entrée
	for (i = 0; i < deviceCount; i++) {
		result = waveInGetDevCaps(i, &incaps, sizeof(incaps));
		if (result == MMSYSERR_NOERROR) {
			// Ajouter le périphérique à la liste
			TAudioDevice device;
			device.ID = i;
			device.Name = UnicodeString(incaps.szPname);
			device.Channels = incaps.wChannels;
			device.SupportedFormats = incaps.dwFormats;
			device.DriverVersion = incaps.vDriverVersion;
			InputDevices.push_back(device);
			
			// Afficher dans le Memo si disponible
//			if (PCore && PCore->Form && PCore->Form->Memo1) {
//				PCore->Form->Memo1->Lines->Add(L"Input " + UnicodeString(i) + L" = " + device.Name);
//			}
		}
		else {
			// Gérer l'erreur
			UnicodeString errorMsg = L"Erreur lors de l'obtention des informations du périphérique d'entrée " + UnicodeString(i);
//			if (PCore && PCore->Form && PCore->Form->Memo1) {
//				PCore->Form->Memo1->Lines->Add(errorMsg);
//			}
		}
	}
	
	return InputDevices.size();
}

// ---------------------------------------------------------------------------
// Sélectionne un périphérique de sortie audio
// Retourne true si la sélection a réussi, false sinon
// ---------------------------------------------------------------------------
bool __fastcall ClassAudioDevice::SelectOutputDevice(int deviceID) {
	// Vérifier que l'ID est valide
	if (deviceID >= 0 && deviceID < (int)OutputDevices.size()) {
		CurrentOutputDevice = deviceID;
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Sélectionne un périphérique d'entrée audio
// Retourne true si la sélection a réussi, false sinon
// ---------------------------------------------------------------------------
bool __fastcall ClassAudioDevice::SelectInputDevice(int deviceID) {
	// Vérifier que l'ID est valide
	if (deviceID >= 0 && deviceID < (int)InputDevices.size()) {
		CurrentInputDevice = deviceID;
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Obtient les informations du périphérique de sortie actuellement sélectionné
// Retourne true si les informations sont disponibles, false sinon
// ---------------------------------------------------------------------------
bool __fastcall ClassAudioDevice::GetCurrentOutputDeviceInfo(TAudioDevice &deviceInfo) {
	if (!OutputDevices.empty() && CurrentOutputDevice < OutputDevices.size()) {
		deviceInfo = OutputDevices[CurrentOutputDevice];
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Obtient les informations du périphérique d'entrée actuellement sélectionné
// Retourne true si les informations sont disponibles, false sinon
// ---------------------------------------------------------------------------
bool __fastcall ClassAudioDevice::GetCurrentInputDeviceInfo(TAudioDevice &deviceInfo) {
	if (!InputDevices.empty() && CurrentInputDevice < InputDevices.size()) {
		deviceInfo = InputDevices[CurrentInputDevice];
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Pour la compatibilité avec le code existant
// ---------------------------------------------------------------------------
void __fastcall ClassAudioDevice::List(void) {
	ListOutputDevices();
}

