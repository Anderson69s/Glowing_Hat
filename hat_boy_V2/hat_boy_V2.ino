#include <Adafruit_NeoPixel.h>

// Pattern types supported:
enum  pattern {
  NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction {
  FORWARD, REVERSE };
int pin = A0;//declare potentiometer
float potard;// declare float to store potientiometer value
// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns :
public Adafruit_NeoPixel
{
public:

  // Member Variables: 
  pattern  ActivePattern;  // which pattern is running
  direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
  unsigned long lastUpdate; // last update of position

  uint32_t Color1, Color2;  // What colors are in use
  uint16_t TotalSteps;  // total number of steps in the pattern
  uint16_t Index;  // current step within the pattern

  void (*OnComplete)();  // Callback on completion of pattern

  // Constructor - calls base-class constructor to initialize strip
  NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
:
    Adafruit_NeoPixel(pixels, pin, type)
    {
      OnComplete = callback;
    }

  // Update the pattern
  void Update()
  {
    if((millis() - lastUpdate) > Interval) // time to update
    {
      lastUpdate = millis();
      switch(ActivePattern)
      {
      case RAINBOW_CYCLE:
        RainbowCycleUpdate();
        break;
      case THEATER_CHASE:
        TheaterChaseUpdate();
        break;
      case COLOR_WIPE:
        ColorWipeUpdate();
        break;
      case SCANNER:
        ScannerUpdate();
        break;
      case FADE:
        FadeUpdate();
        break;
      default:
        break;
      }
    }
  }

  // Increment the Index and reset at the end
  void Increment()
  {
    if (Direction == FORWARD)
    {
      Index++;
      if (Index >= TotalSteps)
      {
        Index = 0;
        if (OnComplete != NULL)
        {
          OnComplete(); // call the comlpetion callback
        }
      }
    }
    else // Direction == REVERSE
    {
      --Index;
      if (Index <= 0)
      {
        Index = TotalSteps-1;
        if (OnComplete != NULL)
        {
          OnComplete(); // call the comlpetion callback
        }
      }
    }
  }

  // Reverse pattern direction
  void Reverse()
  {
    if (Direction == FORWARD)
    {
      Direction = REVERSE;
      Index = TotalSteps-1;
    }
    else
    {
      Direction = FORWARD;
      Index = 0;
    }
  }

  // Initialize for a RainbowCycle
  void RainbowCycle(uint8_t interval, direction dir = FORWARD)
  {
    ActivePattern = RAINBOW_CYCLE;
    Interval = interval;
    TotalSteps = 255;
    Index = 0;
    Direction = dir;
  }

  // Update the Rainbow Cycle Pattern
  void RainbowCycleUpdate()
  {
    for(int i=0; i< numPixels(); i++)
        {
        uint32_t wheelColor = Wheel(((i * 256 / numPixels()) + Index) & 255);
          setPixelColor(i, (Red(wheelColor) * potard), (Green(wheelColor) * potard), (Blue(wheelColor) * potard));
        }
    show();
    Increment();
  }

  // Initialize for a Theater Chase
  void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
  {
    ActivePattern = THEATER_CHASE;
    Interval = interval;
    TotalSteps = numPixels();
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = dir;
  }

  // Update the Theater Chase Pattern
  void TheaterChaseUpdate()
  {
    for(int i=0; i< numPixels(); i++)
    {
      if ((i + Index) % 3 == 0)
      {
        setPixelColor(i, Color1);
      }
      else
      {
        setPixelColor(i, Color2);
      }
    }
    show();
    Increment();
  }

  // Initialize for a ColorWipe
  void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
  {
    ActivePattern = COLOR_WIPE;
    Interval = interval;
    TotalSteps = numPixels();
    Color1 = color;
    Index = 0;
    Direction = dir;
  }

  // Update the Color Wipe Pattern
  void ColorWipeUpdate()
  {
    setPixelColor(Index, Color1);
    show();
    Increment();
  }

  // Initialize for a SCANNNER
  void Scanner(uint32_t color1, uint8_t interval)
  {
    ActivePattern = SCANNER;
    Interval = interval;
    TotalSteps = (numPixels() - 1) * 2;
    Color1 = color1;
    Index = 0;
  }

  // Update the Scanner Pattern
  void ScannerUpdate()
  {
    for (int i = 0; i < numPixels(); i++)
    {
      if (i == Index)  // Scan Pixel to the right
      {
        setPixelColor(i, Color1);
      }
      else if (i == TotalSteps - Index) // Scan Pixel to the left
      {
        setPixelColor(i, Color1);
      }
      else // Fading tail
      {
        setPixelColor(i, DimColor(getPixelColor(i)));
      }
    }
    show();
    Increment();
  }

  // Initialize for a Fade
  void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
  {
    ActivePattern = FADE;
    Interval = interval;
    TotalSteps = steps;
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = dir;
  }

  // Update the Fade Pattern
  void FadeUpdate()
  {
    // Calculate linear interpolation between Color1 and Color2
    // Optimise order of operations to minimize truncation error
    uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
    uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
    uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

    ColorSet(Color(red, green, blue));
    show();
    Increment();
  }

  // Calculate 50% dimmed version of a color (used by ScannerUpdate)
  uint32_t DimColor(uint32_t color)
  {
    // Shift R, G and B components one bit to the right
    uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
    return dimColor;
  }

  // Set all pixels to a color (synchronously)
  void ColorSet(uint32_t color)
  {
    for (int i = 0; i < numPixels(); i++)
    {
      setPixelColor(i, color);
    }
    show();
  }

  // Returns the Red component of a 32-bit color
  uint8_t Red(uint32_t color)
  {
    return (color >> 16) & 0xFF;
  }

  // Returns the Green component of a 32-bit color
  uint8_t Green(uint32_t color)
  {
    return (color >> 8) & 0xFF;
  }

  // Returns the Blue component of a 32-bit color
  uint8_t Blue(uint32_t color)
  {
    return color & 0xFF;
  }

  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos)
  {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85)
    {
      return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if(WheelPos < 170)
    {
      WheelPos -= 85;
      return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else
    {
      WheelPos -= 170;
      return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
  }
};

// Define some NeoPatterns for the two rings 
//  as well as some completion routines
NeoPatterns Ring1(16, 5, NEO_GRB + NEO_KHZ800, &Ring1Complete);
NeoPatterns Ring2(16, 6, NEO_GRB + NEO_KHZ800, &Ring2Complete);

bool oldState = HIGH;
bool oldState2 = HIGH;
int showType = 0;

// Initialize everything and prepare to start
void setup()
{
  Serial.begin(115200);//start serial for fast debugging using Serial.print();

  pinMode(8, INPUT_PULLUP);//pull_up the first button
  pinMode(9, INPUT_PULLUP);//pull_up the second button

  // Initialize all the pixelStrips
  Ring1.begin();
  Ring2.begin();
}

// Main loop
void loop()
{
  // Update the rings.
  Ring1.Update();
  Ring2.Update();   
  potard = (analogRead(pin) / 1024.0);//read value from potentiometer (value is between 0 - 1)
  bool newState = digitalRead(8);//read digital 8 
  // Check if state changed from high to low (button press).
  if (newState == LOW && oldState == HIGH) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(8);//if digital 8 is low increment the counter
    if (newState == LOW) {
      showType++;
      if (showType > 11)//go back to first animation if showType > 11
        showType=0;
      startShow(showType);
    }
  }
   bool newState2 = digitalRead(9);//read digital 9
  
  // Check if state changed from high to low (button press).
  if (newState2 == LOW && oldState2 == HIGH) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState2 = digitalRead(9);
    if (newState2 == LOW) {//if digital 9 is low decrement the counter
      showType--;
      if (showType < 0)
        showType=11;
      startShow(showType);
    }
  }

  // Set the last button state to the old state.
  oldState = newState;
  oldState2 = newState2;
}

void startShow(int i) {
  switch(i){
    case 0:     
            //No animation
            Ring1.ActivePattern = NONE;
            Ring2.ActivePattern = NONE;
            Ring2.ColorSet(Ring2.Color(0, 0, 0));
            Ring1.ColorSet(Ring1.Color(0, 0, 0));
            break;
    case 1: 
            Ring1.ActivePattern = NONE;
            Ring2.ActivePattern = NONE;
            Ring2.ColorSet(Ring2.Color(255 * potard, 255 * potard, 255 * potard));
            Ring1.ColorSet(Ring1.Color(255 * potard, 255 * potard, 255 * potard));
            break;
    case 2: 
            Ring2.TheaterChase(Ring2.Color(255 * potard,0,0), Ring2.Color(0,0,0), 100);
            Ring1.TheaterChase(Ring1.Color(255 * potard,0,0), Ring1.Color(0,0,0), 100);
            break;
    case 3: 
            Ring2.TheaterChase(Ring2.Color(0,255 * potard,0), Ring2.Color(0,0,0), 100);
            Ring1.TheaterChase(Ring1.Color(0,255 * potard,0), Ring1.Color(0,0,0), 100);
            break;
    case 4: 
            Ring2.TheaterChase(Ring2.Color(0,0,255 * potard), Ring2.Color(0,0,0), 100);
            Ring1.TheaterChase(Ring1.Color(0,0,255 * potard), Ring1.Color(0,0,0), 100);
            break;
    case 5:             
            Ring2.TheaterChase(Ring2.Color(255 * potard,255 * potard,255 * potard), Ring2.Color(0,0,0), 100);
            Ring1.TheaterChase(Ring1.Color(255 * potard,255 * potard,255 * potard), Ring1.Color(0,0,0), 100);
            break;
    case 6: 
            Ring2.Scanner(Ring1.Color(255 * potard,0,0), 55);
            Ring1.Scanner(Ring1.Color(255 * potard,0,0), 55);
            break;
    case 7: 
            Ring2.Scanner(Ring1.Color(0,255 * potard,0), 55);
            Ring1.Scanner(Ring1.Color(0,255 * potard,0), 55);
            break;
    case 8: 
            Ring2.Scanner(Ring1.Color(0,0,255 * potard), 55);
            Ring1.Scanner(Ring1.Color(0,0,255 * potard), 55);
            break;
    case 9: 
            Ring2.Scanner(Ring1.Color(255 * potard,255 * potard,255 * potard), 55);
            Ring1.Scanner(Ring1.Color(255 * potard,255 * potard,255 * potard), 55);
            break;
    case 10: 
            Ring2.ActivePattern = FADE;
            Ring2.Interval = 50;
            Ring1.ActivePattern = FADE;
            Ring1.Interval = 50;
            Ring1.Color1 = (Ring1.Color(255 * potard,0,0));
            Ring2.Color1 = (Ring2.Color(255 * potard,0,0));
            Ring1.Color2 = (Ring1.Color(0,0,255 * potard));
            Ring2.Color2 = (Ring2.Color(0,255 * potard,0));
            break;
    case 11: 
            Ring2.RainbowCycle(10);
            Ring1.RainbowCycle(10);
            break;
}
}
//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Ring1 Completion Callback
void Ring1Complete()
{
   
}


// Ring 2 Completion Callback
void Ring2Complete()
{
    
}


