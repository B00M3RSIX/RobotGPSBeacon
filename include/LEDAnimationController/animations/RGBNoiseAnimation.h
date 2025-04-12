#ifndef RGB_NOISE_ANIMATION_H
#define RGB_NOISE_ANIMATION_H

#include "../AnimationBase.h"
#include <math.h>

/**
 * @brief RGB-Rausch-Animation
 * 
 * Diese Animation erzeugt kontinuierlich Rauscheffekte mit wechselnden Farben.
 * Sie verwendet Vorberechnungstechniken für bessere Performance.
 * Sie ist vom Typ CONTINUOUS und läuft endlos.
 */
class RGBNoiseAnimation : public AnimationBase {
private:
    // Struktur für vorberechnete Rauschwerte
    struct NoisePoint {
        uint8_t hue;
        uint8_t sat;
        uint8_t val;
    };
    
    NoisePoint* noisePoints;  // Array für vorberechnete Rauschwerte
    float* precalcCos;        // Vorberechnete Cosinus-Werte
    float* precalcSin;        // Vorberechnete Sinus-Werte
    uint32_t runTime;         // Laufzeit der Animation
    
public:
    /**
     * @brief Konstruktor
     * 
     * @param leds Zeiger auf das LED-Array
     * @param context current Animation context
     */
    RGBNoiseAnimation(CRGB* leds, AnimationContext* context) 
        : AnimationBase(leds, context) 
        , runTime(0) {
        
        // Speicher für vorberechnete Werte allozieren
        noisePoints = new NoisePoint[context->numLeds];
        precalcCos = new float[context->numLeds];
        precalcSin = new float[context->numLeds];
        
        // Trigonometrische Werte vorberechnen
        for (int i = 0; i < context->numLeds; i++) {
            float angle = i * 2 * M_PI / context->numLeds;
            precalcCos[i] = cos(angle);
            precalcSin[i] = sin(angle);
        }
    }
    
    /**
     * @brief Destruktor
     * 
     * Gibt den allozierten Speicher frei.
     */
    ~RGBNoiseAnimation() {
        delete[] noisePoints;
        delete[] precalcCos;
        delete[] precalcSin;
    }
    
    /**
     * @brief Gibt den Typ der Animation zurück
     * 
     * @return ANIMATION_CONTINUOUS
     */
    AnimationType getType() const override {
        return ANIMATION_CONTINUOUS;
    }
    
    /**
     * @brief Prüft, ob die Animation einen bestimmten Befehl unterstützt
     * 
     * @param cmd Der zu prüfende Befehl
     * @return true wenn cmd == CMD_RGB_NOISE
     */
    bool supportsCommand(AnimationCommand cmd) const override {
        return cmd == CMD_RGB_NOISE;
    }
    
    /**
     * @brief Startet die Animation
     */
    void onStart() override {
        runTime = 0;
    }
    
    /**
     * @brief Führt einen Animationsschritt aus
     * 
     * @param currentTime Aktuelle Zeit in Millisekunden
     * @return true wenn der Schritt erfolgreich ausgeführt wurde
     */
    bool run(uint32_t currentTime) override {
        // Prüfe, ob die Animation abgebrochen wurde
        if (cancelRequested) {
            return true;
        }
        
        // Prüfe, ob es Zeit für den nächsten Schritt ist
        if (currentTime - lastUpdateTime < stepDuration) {
            return true; // Noch nicht Zeit für Update, aber erfolgreich
        }
        
        // Berechne die Rauschwerte für den aktuellen Frame vor
        precalculateNoiseFrame(currentTime);
        
        // Wende die vorberechneten Werte auf die LEDs an
        for (int i = 0; i < context->numLeds; i++) {
            leds[i] = CHSV(noisePoints[i].hue, noisePoints[i].sat, noisePoints[i].val);
        }
        
        // Zeige die Änderung an
        FastLED.show();
        
        // Aktualisiere die Laufzeit
        runTime += currentTime - lastUpdateTime;
        
        // Bei kontinuierlichen Animationen bleibt der Fortschritt immer bei 0
        // oder kann zyklisch sein, je nach Implementierung
        progress = 0.0f;
        
        // Aktualisiere die Zeit für den nächsten Schritt
        lastUpdateTime = currentTime;
        
        return true;
    }
    
    /**
     * @brief Gibt den Fortschritt der Animation zurück
     * 
     * Bei kontinuierlichen Animationen gibt es keinen echten Fortschritt,
     * daher wird immer 0.0 zurückgegeben.
     * 
     * @return 0.0f
     */
    float getProgress() const override {
        return 0.0f;
    }
    
private:
    /**
     * @brief Berechnet die Rauschwerte für den aktuellen Frame vor
     * 
     * @param now Aktuelle Zeit in Millisekunden
     */
    void precalculateNoiseFrame(uint32_t now) {
        double angle_offset = double(now) / 32000.0 * 2 * M_PI;
        now = (now << 5) * 1;
        
        for (int i = 0; i < context->numLeds; i++) {
            // Verwende vorberechnete Werte für bessere Performance
            float x = precalcCos[i] * cos(angle_offset) - precalcSin[i] * sin(angle_offset);
            float y = precalcCos[i] * sin(angle_offset) + precalcSin[i] * cos(angle_offset);
            
            x *= 0xffff * 4;
            y *= 0xffff * 4;
            
            // Berechne verschiedene Rauschwerte für Farbton, Sättigung und Helligkeit
            uint16_t noise = inoise16(x, y, now);
            uint16_t noise2 = inoise16(x, y, 0xfff + now);
            uint16_t noise3 = inoise16(x, y, 0xffff + now);
            noise3 = noise3 >> 8;
            
            int16_t noise4 = map(noise3, 0, 255, -64, 255);
            if (noise4 < 0) {
                noise4 = 0;
            }
            
            // Speichere die vorberechneten Werte
            noisePoints[i].hue = noise >> 8;
            noisePoints[i].sat = MAX(128, noise2 >> 8);
            noisePoints[i].val = noise4;
        }
    }
};

#endif // RGB_NOISE_ANIMATION_H
