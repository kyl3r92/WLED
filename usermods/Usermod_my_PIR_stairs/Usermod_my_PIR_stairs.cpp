// This is a minimal version of the original Usermod_Staircase_Controller
// It provides the lockout logic necessary to prevent double-triggering.
// **FINAL VERSION: Corrected for Active HIGH Trigger**

#include "wled.h"

class Usermod_my_PIR_stairs : public Usermod {
  private:
    // Configuration
    int8_t pirPinUp = -1;      // GPIO for upward movement
    uint8_t presetIdUp = 0;    // Preset ID for upward movement
    int8_t pirPinDown = -1;    // GPIO for downward movement
    uint8_t presetIdDown = 0;  // Preset ID for downward movement
    uint16_t lockoutTime = 15; // Lockout time in seconds

    // Runtime
    unsigned long lastTriggerTime = 0;
    bool initDone = false;

    // Strings to reduce flash memory usage
    static const char _name[];
    static const char _pinUp[];
    static const char _presetUp[];
    static const char _pinDown[];
    static const char _presetDown[];
    static const char _lockout[];

  public:
    void setup() {
      // FIX 1: Wechsel auf INPUT, da der Sensor den Pin aktiv treibt
      if (pirPinUp >= 0) pinMode(pirPinUp, INPUT); 
      if (pirPinDown >= 0) pinMode(pirPinDown, INPUT);
      initDone = true;
    }

    void loop() {
      if (!initDone) return;
      if (strip.isUpdating()) return; 

      // Check if we are currently in lockout period
      if (millis() - lastTriggerTime < (unsigned long)lockoutTime * 1000) {
        return; // Ignore all sensor input during lockout
      }

      bool trigger = false;
      uint8_t presetToStart = 0;

      // Check Up Sensor
      // FIX 2: Trigger auf HIGH (Bewegung erkannt)
      if (pirPinUp >= 0 && digitalRead(pirPinUp) == HIGH) { 
        trigger = true;
        presetToStart = presetIdUp;
      }
      
      // Check Down Sensor (only if Up Sensor didn't trigger)
      // FIX 2: Trigger auf HIGH (Bewegung erkannt)
      if (!trigger && pirPinDown >= 0 && digitalRead(pirPinDown) == HIGH) { 
        trigger = true;
        presetToStart = presetIdDown;
      }

      if (trigger && presetToStart > 0) {
        // Start the preset
        applyPreset(presetToStart, CALL_MODE_BUTTON);
        
        // Start lockout timer
        lastTriggerTime = millis();
        DEBUG_PRINT(F("Staircase triggered preset ID "));
        DEBUG_PRINT(presetToStart);
        DEBUG_PRINT(F(". Lockout for "));
        DEBUG_PRINT(lockoutTime);
        DEBUG_PRINTLN(F("s."));
      }
    }

    uint16_t getId() { return USERMOD_ID_PIR_STAIRCASE; } // Use a unique ID

    // --- Configuration Handling ---

    void addToConfig(JsonObject& root) {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_pinUp)]    = pirPinUp;
      top[FPSTR(_presetUp)]  = presetIdUp;
      top[FPSTR(_pinDown)]  = pirPinDown;
      top[FPSTR(_presetDown)] = presetIdDown;
      top[FPSTR(_lockout)]  = lockoutTime;
    }

    bool readFromConfig(JsonObject& root) {
      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) return false;

      pirPinUp    = top[FPSTR(_pinUp)] | pirPinUp;
      presetIdUp  = top[FPSTR(_presetUp)] | presetIdUp;
      pirPinDown  = top[FPSTR(_pinDown)] | pirPinDown;
      presetIdDown = top[FPSTR(_presetDown)] | presetIdDown;
      lockoutTime = top[FPSTR(_lockout)] | lockoutTime;

      if (initDone) setup();
      return true;
    }
};

const char Usermod_my_PIR_stairs::_name[]      PROGMEM = "PIR Stairs";
const char Usermod_my_PIR_stairs::_pinUp[]     PROGMEM = "pinUp";
const char Usermod_my_PIR_stairs::_presetUp[]  PROGMEM = "presetUp";
const char Usermod_my_PIR_stairs::_pinDown[]   PROGMEM = "pinDown";
const char Usermod_my_PIR_stairs::_presetDown[] PROGMEM = "presetDown";
const char Usermod_my_PIR_stairs::_lockout[]   PROGMEM = "lockoutSec";

static Usermod_my_PIR_stairs myPIRStairsInstance; 
REGISTER_USERMOD(myPIRStairsInstance);