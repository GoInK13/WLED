#pragma once

#include "wled.h"

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This usermod can be used to drive a wordclock with a 11x10 pixel matrix with WLED. There are also 4 additional dots for the minutes. 
 * The visualisation is desribed in 4 mask with LED numbers (single dots for minutes, minutes, hours and "clock/Uhr").
 * There are 2 parameters to chnage the behaviour:
 * 
 * active: enable/disable usermod
 * diplayItIs: enable/disable display of "Il est" on the clock.
 */

class WordClockFrUsermod : public Usermod 
{
  private:
    unsigned long lastTime = 0;
    int lastTimeMinutes = -1;

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    bool usermodActive = false;
    bool displayItIs = false;
    bool displayPAM = false;
    bool animationActive = false;
    
    // defines for mask sizes
    #define maskSizeLeds        110
    #define maskSizeMinutes     15
    #define maskSizeHours       12
    #define maskSizeItIs        5
    #define maskSizeMinuteDots  2
    #define maskSizeAntePostMeridiem  2

/*
    01234567890 0→10
    ILMESTIDEUX
    10987654321 21→11
    QUATRELUNES
    23456789012 22→32
    HUITROISEPT
    32109876543 43→33
    NEUFONZESIX
    45678901234 44→54
    MIDIXMINUIT
    54321098765 65→55
    CINQYHEURES
    67890123456 66→76
    JMOINSKLETB
    76543210987 87→77
    DEMIELQUART
    89012345678 88→98
    VINGT-CINQW
    98765432109 109→99
    DIX+1234PAM
*/
    // "minute" masks
    const int maskMinutes[12][maskSizeMinutes] = 
    {
      { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}, // :00
      { 94,  95,  96,  97,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}, // :05  CINQ 
      {109, 108, 107,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}, // :10  DIX
      { 74,  75,  81,  80,  79,  78,  77,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}, // :15  ET QUART
      { 88,  89,  90,  91,  92,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}, // :20  VINGT
      { 88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  -1,  -1,  -1,  -1,  -1}, // :25  VINGT-CINQ
      { 74,  75,  87,  86,  85,  84,  83,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1}, // :30  ET DEMIE
      { 67,  68,  69,  70,  71,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97}, // :35  MOINS VINGT-CINQ
      { 67,  68,  69,  70,  71,  88,  89,  90,  91,  92,  -1,  -1,  -1,  -1,  -1}, // :40  MOINS VINGT
      { 67,  68,  69,  70,  71,  73,  74,  81,  80,  79,  78,  77,  -1,  -1,  -1}, // :45  MOINS LE QUART
      { 67,  68,  69,  70,  71, 109, 108, 107,  -1,  -1,  -1,  -1,  -1,  -1,  -1}, // :50  MOINS DIX
      { 67,  68,  69,  70,  71,  94,  95,  96,  97,  -1,  -1,  -1,  -1,  -1,  -1}  // :55  MOINS CINQ
    };

    // hour masks
    const int maskHours[13][maskSizeHours] = 
    {
      { 49,  50,  51,  52,  53,  54,  -1,  -1,  -1,  -1,  -1,  -1}, // 00: MINUIT
      { 14,  13,  12,  60,  59,  58,  57,  56,  -1,  -1,  -1,  -1}, // 01: UNE HEURE
      {  7,   8,   9,  10,  60,  59,  58,  57,  56,  55,  -1,  -1}, // 02: DEUX HEURES
      { 25,  26,  27,  28,  29,  60,  59,  58,  57,  56,  55,  -1}, // 03: TROIS HEURES
      { 21,  20,  19,  18,  17,  16,  60,  61,  62,  63,  64,  65}, // 04: QUATRE HEURES
      { 65,  64,  63,  62,  60,  59,  58,  57,  56,  55,  -1,  -1}, // 05: CINQ HEURES
      { 35,  34,  33,  60,  59,  58,  57,  56,  55,  -1,  -1,  -1}, // 06: SIX HEURES
      { 29,  30,  31,  32,  60,  59,  58,  57,  56,  55,  -1,  -1}, // 07: SEPT HEURES
      { 22,  23,  24,  25,  60,  59,  58,  57,  56,  55,  -1,  -1}, // 08: HUIT HEURES
      { 43,  42,  41,  40,  60,  59,  58,  57,  56,  55,  -1,  -1}, // 09: NEUF HEURES
      { 46,  47,  48,  60,  59,  58,  57,  56,  55,  -1,  -1,  -1}, // 10: DIX HEURES
      { 39,  38,  37,  36,  60,  59,  58,  57,  56,  55,  -1,  -1}, // 11: ONZE HEURES
      { 44,  45,  46,  47,  60,  59,  58,  57,  56,  55,  -1,  -1}  // 12: MIDI
    };

    // mask "Il est"
    const int maskItIs[maskSizeItIs] = {0, 1, 3, 4, 5};

    // mask minute dots
    const int maskMinuteDots[4][maskSizeMinuteDots] = {
      { 106, 105}, //+1
      { 106, 104}, //+2
      { 106, 103}, //+3
      { 106, 102}  //+4
    };
    
    // mask afternoon or morning
    const int maskAntePostMeridiem[2][maskSizeAntePostMeridiem] = {
      { 100,  99}, //AM
      { 101,  99}  //PM
    };

    // overall mask to define which LEDs are on
    int maskLedsOn[maskSizeLeds] = 
    {
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0
    };

    // update led mask
    void updateLedMask(const int wordMask[], int arraySize)
    {
      // loop over array
      for (int x=0; x < arraySize; x++) 
      {
        // check if mask has a valid LED number
        if (wordMask[x] >= 0 && wordMask[x] < maskSizeLeds)
        {
          // turn LED on
          maskLedsOn[wordMask[x]] = 1;
        }
      }
    }

    // set hours
    void setHours(int hours)
    {
      int index = hours;

      // check if we get an overrun of 12 o´clock
      if (hours >= 13)
      {
        index -= 12;
      }

      // update led mask
      updateLedMask(maskHours[index], maskSizeHours);
    }

    // set minutes
    void setMinutes(int index)
    {
      // update led mask
      updateLedMask(maskMinutes[index], maskSizeMinutes);
    }

    // set minutes dot
    void setSingleMinuteDots(int minutes)
    {
      // modulo to get minute dots
      int minutesDotCount = minutes % 5;

      // check if minute dots are active
      if (minutesDotCount > 0)
      {
        updateLedMask(maskMinuteDots[minutesDotCount-1], maskSizeMinuteDots);
      }
    }

    
    // set AM or PM
    void setAntePostMeridiem(bool amPM)
    {
      updateLedMask(maskAntePostMeridiem[amPM], maskSizeAntePostMeridiem);
    }

    // update the display
    // localtime from internet
    void updateDisplay(time_t f_localTime) 
    {
      int minutes = minute(f_localTime);
      int hours = hour(f_localTime);

      // disable complete matrix at the bigging
      for (int x = 0; x < maskSizeLeds; x++)
      {
        maskLedsOn[x] = 0;
      } 
      
      // display it is/Il est if activated
      if (displayItIs)
      {
        updateLedMask(maskItIs, maskSizeItIs);
      }

      // set single minute dots
      setSingleMinuteDots(minutes);

      // switch minutes
      switch (minutes / 5) 
      {
        case 0:
            // full hour
            setMinutes(0);
            setHours(hours);
            break;
        case 1:
            // 5 nach
            setMinutes(1);
            setHours(hours);
            break;
        case 2:
            // 10 nach
            setMinutes(2);
            setHours(hours);
            break;
        case 3:
            // 15
            setMinutes(3);
            setHours(hours);
            break;
        case 4:
            // 20 nach
            setMinutes(4);
            setHours(hours);
            break;
        case 5:
            // 25
            setMinutes(5);
            setHours(hours);
            break;
        case 6:
            // 30
            setMinutes(6);
            setHours(hours);
            break;
        case 7:
            // -25
            setMinutes(7);
            setHours(hours + 1);
            break;
        case 8:
            // -20
            setMinutes(8);
            setHours(hours + 1);
            break;
        case 9:
            // -15
            setMinutes(9);
            setHours(hours + 1);
            break;
        case 10:
            // -10
            setMinutes(10);
            setHours(hours + 1);
            break;
        case 11:
            // -5
            setMinutes(11);
            setHours(hours + 1);
            break;
        }

        //Set AM or PM if asked
        if (displayPAM) {
          setAntePostMeridiem(isPM(f_localTime));
        }
    }

  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() 
    {
    }

    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() 
    {
    }

    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() {
      static bool l_usermodActive_old = false;
      static bool l_displayItIs_old = false;
      static bool l_displayPAM_old = false;

      // do it every 5 seconds
      if (millis() - lastTime > 5000 || 
          l_usermodActive_old != usermodActive ||
          l_displayItIs_old   != displayItIs ||
          l_displayPAM_old    != displayPAM) 
      {
        // check the time
        int minutes = minute(localTime);

        // check if we already updated this minute
        if (lastTimeMinutes != minutes ||
          l_usermodActive_old != usermodActive ||
          l_displayItIs_old   != displayItIs ||
          l_displayPAM_old    != displayPAM) 
        {
          // update the display with new time
          updateDisplay(localTime);

          // remember last update time
          lastTimeMinutes = minutes;
        }

        // remember last update
        lastTime = millis();
        l_usermodActive_old = usermodActive;
        l_displayItIs_old   = displayItIs;
        l_displayPAM_old    = displayPAM;
      }
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    /*
    void addToJsonInfo(JsonObject& root)
    {
    }
    */

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
    }

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
    }

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     * 
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.  
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("Word Clock Fr");
      top["Activer"] = usermodActive;
      top["Allumer Il est"] = displayItIs;
      top["Allumer PAM"] = displayPAM;
      top["Activer animation"] = animationActive;
    }

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     * 
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     * 
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root)
    {
      // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
      // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

      JsonObject top = root["Word Clock Fr"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["Activer"], usermodActive);
      configComplete &= getJsonValue(top["Allumer Il est"], displayItIs);
      configComplete &= getJsonValue(top["Allumer PAM"], displayPAM);
      configComplete &= getJsonValue(top["Activer animation"], animationActive);

      return configComplete;
    }

    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw()
    {
      // check if usermod is active
      if (usermodActive == true)
      {
        // loop over all leds
        for (int x = 0; x < maskSizeLeds; x++)
        {
          // check mask
          if (maskLedsOn[x] == 0)
          {
            // set pixel off
            strip.setPixelColor(x, RGBW32(0,0,0,0));
          }
        }
      }
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_WORDCLOCK_FR;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};