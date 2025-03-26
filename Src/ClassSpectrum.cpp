//---------------------------------------------------------------------------
#include "JukeboxPCH1.h"
#pragma hdrstop

#include "ClassSpectrum.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
//   ClassSpectrum
// ---------------------------------------------------------------------------
__fastcall XSpectrum::XSpectrum(XMix *mix) {
    // Initialisation des membres
    Mix = mix;
    PBkg = new TBitmap();
    PImage = NULL;
    PMode = SpectrumMode::Bar;
    PNeedRedraw = true;
    PBarWidth = 7;    // Largeur par défaut des barres
    PBarSpacing = 2;  // Espacement par défaut entre les barres

    // Initialisation du tableau de pics
    memset(PPic, 0, sizeof(PPic));
}

__fastcall XSpectrum::~XSpectrum(void) {
    // Libération des ressources
    if (PBkg) {
        delete PBkg;
        PBkg = NULL;
    }
}

void __fastcall XSpectrum::SetImage(TImage *image) {
    // Vérification si l'image a changé
    if (PImage != image) {
        PImage = image;

        // Redimensionnement du bitmap de fond si l'image est valide
        if (PImage) {
            PBkg->Width = PImage->Width;
            PBkg->Height = PImage->Height;
            PNeedRedraw = true;
        }
    }
}

void __fastcall XSpectrum::SetMode(SpectrumMode mode) {
    // Changement du mode d'affichage
    if (PMode != mode) {
        PMode = mode;
        PNeedRedraw = true;

        // Réinitialisation du tableau de pics lors du changement de mode
        Reset();
    }
}

void __fastcall XSpectrum::Reset(void) {
    // Réinitialisation du tableau de pics
    memset(PPic, 0, sizeof(PPic));
    PNeedRedraw = true;
}

void __fastcall XSpectrum::SetBarWidth(int width) {
    // Validation et application de la nouvelle largeur
    if (width < 1) width = 1;
    if (width > 50) width = 50;  // Limite supérieure raisonnable

    if (PBarWidth != width) {
        PBarWidth = width;
        PNeedRedraw = true;
    }
}

void __fastcall XSpectrum::SetBarSpacing(int spacing) {
    // Validation et application du nouvel espacement
    if (spacing < 0) spacing = 0;
    if (spacing > 20) spacing = 20;  // Limite supérieure raisonnable

    if (PBarSpacing != spacing) {
        PBarSpacing = spacing;
        PNeedRedraw = true;
    }
}

void __fastcall XSpectrum::Draw(void) {
    // Vérification si l'image est valide
    if (!PImage) return;

    // Dessin du spectre selon le mode sélectionné
    DrawSpectrum();
}

// Méthodes de compatibilité pour maintenir l'interface existante
void __fastcall XSpectrum::DrawOscilo(void) {
    // Définir le mode Oscilloscope et dessiner
    SetMode(SpectrumMode::Oscilloscope);
    DrawSpectrum();
}

void __fastcall XSpectrum::DrawBar(void) {
    // Définir le mode Bar et dessiner
    SetMode(SpectrumMode::Bar);
    DrawSpectrum();
}

void __fastcall XSpectrum::DrawSpectrum(void) {
    // Vérifications préliminaires
    if (!PImage || !Mix) return;

    // Variables locales
    float *fftcur = NULL;
    float *fftlast = NULL;
    float volcur = 0.0f;
    float vollast = 0.0f;
    float s = 0.0f;
    float o = 0.0f;
    int h = 0;
    TCanvas *c = NULL;
    const float chute = 1.5f;
    bool ismix = false;

    // Préparation du bitmap de fond
    PBkg->Canvas->Pen->Color = clGray;
    PBkg->Canvas->Brush->Color = clBlack;
    PBkg->Canvas->Rectangle(0, 0, PBkg->Width, PBkg->Height);
    PBkg->Canvas->Pen->Color = clSkyBlue;

    try {
        // Vérification si la lecture est en cours
        if (Mix->Current && Mix->Current->IsPlaying) {
            // Récupération des données
            ismix = Mix->IsMixing;
            fftcur = Mix->Current->FFT;
            fftlast = Mix->Last->FFT;
            c = PBkg->Canvas;
            h = PBkg->Height;
            vollast = Mix->Last->Volume;
            volcur = Mix->Current->Volume;

            if (PMode == SpectrumMode::Oscilloscope) {
                // Mode Oscilloscope: traitement des données FFT point par point
                for (int i = 1; i <= 126; i++) {
                    // Calcul de l'amplitude pour la fréquence i
                    s = fabs(fftcur[i + 5]) * volcur;

                    // Mixage avec le canal précédent si nécessaire
                    if (ismix) {
                        s += fabs(fftlast[i + 5]) * vollast;
                        s /= 2.0f;
                    }

                    // Transformation non linéaire pour améliorer la visualisation
                    if (s > 0.0f) {
                        s = sqrt(sqrt(s)) * h * 1.5f;
                    }

                    // Limitation de l'amplitude à la hauteur de l'image
                    s = std::min(s, static_cast<float>(h));

                    // Application de l'effet de chute pour une animation plus fluide
                    if (s >= PPic[i] - chute) {
                        PPic[i] = s;
                    } else {
                        PPic[i] -= chute;
                    }

                    // Dessin des lignes uniquement si nous avons traité au moins deux points
                    if (i > 1) {
                        // Dessin de la ligne principale (jaune)
                        PBkg->Canvas->Pen->Color = clYellow;
                        c->MoveTo(i * 2 - 2, h - o - 1);
                        c->LineTo(i * 2, h - s - 1);

                        // Dessin de la ligne de pic (bleu ciel)
                        PBkg->Canvas->Pen->Color = clSkyBlue;
                        c->MoveTo(i * 2 - 2, h - PPic[i - 1] - 1);
                        c->LineTo(i * 2, h - PPic[i] - 1);
                    }

                    // Mémorisation de la valeur actuelle pour le prochain point
                    o = s;
                }
            } else { // Mode Bar
                // Mode Bar: traitement des données FFT par groupes pour des barres plus larges
                // Calcul des paramètres optimaux des barres
                int barWidth = PBarWidth;
                int barSpacing = PBarSpacing;

                // Calcul du nombre de barres qui peuvent tenir dans la largeur disponible
                int totalBarWidth = barWidth + barSpacing;
                int maxBars = PBkg->Width / totalBarWidth;

                // Ajustement de la largeur et de l'espacement si nécessaire
                if (maxBars < 10) {  // Nombre minimum de barres souhaité
                    // Réduction de la largeur et de l'espacement pour avoir au moins 10 barres
                    float ratio = static_cast<float>(PBkg->Width) / (10 * (barWidth + barSpacing));
                    barWidth = static_cast<int>(barWidth * ratio);
                    if (barWidth < 1) barWidth = 1;
                    barSpacing = static_cast<int>(barSpacing * ratio);
                    if (barSpacing < 1) barSpacing = 1;
                    totalBarWidth = barWidth + barSpacing;
                    maxBars = PBkg->Width / totalBarWidth;
                }

                // Limitation du nombre de barres à 126 (nombre maximum de points FFT)
                int numBars = (maxBars > 126) ? 126 : maxBars;

                // Calcul de la position de départ pour centrer les barres
                const int startX = (PBkg->Width - numBars * totalBarWidth) / 2; // Centrage

                // Calcul du nombre de points FFT par barre
                const int pointsPerBar = 126 / numBars;

                // Traitement pour chaque barre
                for (int barIndex = 0; barIndex < numBars; barIndex++) {
                    // Calcul de la position X de la barre
                    int barX = startX + barIndex * totalBarWidth;

                    // Calcul de l'amplitude moyenne pour cette barre
                    float sumAmplitude = 0.0f;
                    int startPoint = 1 + barIndex * pointsPerBar;
                    int endPoint = startPoint + pointsPerBar - 1;

                    // Limitation des points pour éviter les dépassements
                    if (endPoint > 126) endPoint = 126;

                    // Calcul de la moyenne des amplitudes pour les points de cette barre
                    for (int i = startPoint; i <= endPoint; i++) {
                        float pointAmplitude = fabs(fftcur[i + 5]) * volcur;

                        // Mixage avec le canal précédent si nécessaire
                        if (ismix) {
                            pointAmplitude += fabs(fftlast[i + 5]) * vollast;
                            pointAmplitude /= 2.0f;
                        }

                        sumAmplitude += pointAmplitude;
                    }

                    // Calcul de l'amplitude moyenne
                    float avgAmplitude = sumAmplitude / (endPoint - startPoint + 1);

                    // Transformation non linéaire pour améliorer la visualisation
                    if (avgAmplitude > 0.0f) {
                        avgAmplitude = sqrt(sqrt(avgAmplitude)) * h * 1.5f;
                    }

                    // Limitation de l'amplitude à la hauteur de l'image
                    avgAmplitude = std::min(avgAmplitude, static_cast<float>(h));

                    // Application de l'effet de chute pour une animation plus fluide
                    if (avgAmplitude >= PPic[barIndex] - chute) {
                        PPic[barIndex] = avgAmplitude;
                    } else {
                        PPic[barIndex] -= chute;
                    }

                    // Dessin de la barre principale (bleu)
                    PBkg->Canvas->Pen->Color = clBlue;
                    PBkg->Canvas->Brush->Color = clNavy;
                    c->Rectangle(barX, h - 1, barX + barWidth, h - avgAmplitude - 1);

                    // Dessin de la ligne de pic (bleu ciel)
                    PBkg->Canvas->Pen->Color = clYellow;
                    c->MoveTo(barX, h - PPic[barIndex] - 1);
                    c->LineTo(barX + barWidth, h - PPic[barIndex] - 1);

                    // Restauration de la couleur de fond
                    PBkg->Canvas->Brush->Color = clBlack;
                }
            }
        }

        // Copie du bitmap de fond vers l'image cible
        BitBlt(PImage->Canvas->Handle, 0, 0, PBkg->Width, PBkg->Height,
               PBkg->Canvas->Handle, 0, 0, SRCCOPY);

        // Rafraîchissement de l'image
        PImage->Refresh();

        // Réinitialisation du flag de redessinage
        PNeedRedraw = false;
    }
    catch (Exception &e) {
        // Journalisation de l'erreur (à implémenter si nécessaire)
    }
    catch (...) {
        // Capture des exceptions inconnues
    }
}

