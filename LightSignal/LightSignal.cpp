#include "LightSignal.h"

//output PWM pin
const byte PIN=6;

//LightSignal constructor which is used to initialize the number of pins
// Arduino board pin number and the type of LED strip(i.e., RGB, RGBW etc.,)
LightSignal::LightSignal(uint16_t pixels, uint8_t pin, uint8_t type):Adafruit_NeoPixel(pixels, pin, type){

}

// Updates the pattern if the difference between current time and lastupdate is grater than the interval time
void LightSignal::Update(LightParameter *a)
{

        if((millis() - a->lastupdate) > a->interval || a->stopTime == 100) // time to update
        {

            switch(a->pattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate(a);
                    a->stopTime = a->startTime + (a->cycles)*(a->interval);
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate(a);
                    a->stopTime = a->startTime + (a->cycles)*(a->interval);
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate(a);
                    a->stopTime = a->startTime + ((a->interval)*(a->grouplength));
                    break;
                case SCANNER:
                    ScannerUpdate(a);
                    a->totalsteps = (a->grouplength-1)*2;
                    a->stopTime = a->startTime + (a->cycles)*((a->interval)*a->totalsteps);
                    break;
                case FADE:
                    FadeUpdate(a);
                    a->stopTime = a->startTime + (a->cycles)*(a->interval);
                    break;
                case BLINK:
                    BlinkUpdate(a);
                    a->stopTime = a->startTime + (a->cycles)*2*(a->interval);
                    break;
                case ON_AND_OFF:
                    OnOffUpdate(a);
                    a->stopTime = a->startTime + (a->cycles)*(a->onTime  + a->offTime);
                    break;
                case PULSATING:
                    PulsatingUpdate(a);
                    a->stopTime = a->startTime + (a->cycles)*((a->interval)*2*max(max(Red(a->Color1),
                                                                                      Blue(a->Color1)),
                                                                                        Green(a->Color1)));
                    break;
                case LOADING:
                    LoadingUpdate(a);
                    a->stopTime = a->startTime + (a->cycles)*((a->interval)*(a->grouplength)) + (a->cycles*a->interval);
                    break;
                case STEP:
                    StepUpdate(a);
                    a->stopTime = a->startTime + (a->cycles)*((a->interval)*(a->grouplength));
                    break;
                default:
                    break;
            }
            a->lastupdate = millis();
        }
    }

//change the status of the  pattern and make all the modules of that group blank
void LightSignal::OnComplete(LightParameter *a)
{
    for( int i = 0; i < a->grouplength; i++)
    {
        setPixelColor(a->group[i], 0, 0, 0, 0);
    }
    show();
}

//updates the function depending on pattern priority provided by user
void LightSignal::mainLoop(LightParameter *a)
{
    if( a->startTime <= millis() && a->complete == -1)                //checks if the pattern is suppose to start or not
   {
     a->complete = 1;
   }
   else if( a->stopTime <= millis() && a->complete == 1)              //checks if the pattern run time is over or not
   {
     a->complete = 0;
     OnComplete(a);
   }
   if( a->complete == 1)                                               //updates the pattern
   {
     Update(a);
   }
}

// Increment the index and reset at the end
void LightSignal::Increment(LightParameter *p)
    {
        if (p->direction == FORWARD)
        {
           p->index++;
           // p->totalsteps++;
         if (p->index >= p->totalsteps)
            {
                p->index = 0;
           /*     if (p->totalsteps>=(p->grouplength*p->cycles))
                {
                    p->complete=OnComplete(p->complete); // call the comlpetion callback
                }*/
            }
        }
        else // Direction == REVERSE
        {
            --p->index;
            if (p->index <= 0)
            {
                p->index = p->grouplength-1;
             /*   if (p->totalsteps>=(p->grouplength*p->cycles))
                {
                    p->complete=OnComplete(p->complete); // call the comlpetion callback
                }*/
            }
        }
    }

// Update the Rainbow Cycle Pattern
void LightSignal::RainbowCycleUpdate(LightParameter *b)
    {
        for(int i=0; i< (b->grouplength); i++)
        {
            setPixelColor(b->group[i], Wheel(((i * 256 / b->grouplength) + b->index) & 255));
        }
        show();
        Increment(b);
    }

// Update the Theater Chase Pattern
void LightSignal::TheaterChaseUpdate(LightParameter *b)
    {
        for(int i=0; i< b->grouplength; i++)
        {
            if ((i + b->index) % 3 == 0)
            {
                setPixelColor(b->group[i], b->Color1);
            }
            else
            {
                setPixelColor(b->group[i], b->Color2);
            }
        }
        show();
        Increment(b);
}

// Update the Color Wipe Pattern
void LightSignal::ColorWipeUpdate(LightParameter *b)
{
        setPixelColor(b->group[b->index], b->Color1);
        show();
        Increment(b);
}

// Update the Scanner Pattern
void LightSignal::ScannerUpdate(LightParameter *b)
{
        for (int i = 0; i < b->grouplength; i++)
        {
            if (i == b->index)  // Scan Pixel to the right
            {
                 setPixelColor(b->group[i], b->Color1);
            }
            else if (i == b->totalsteps - b->index) // Scan Pixel to the left
            {
                 setPixelColor(b->group[i], b->Color1);
            }
            else // Fading tail
            {
                 setPixelColor(b->group[i], DimColor(getPixelColor(b->group[i])));
            }
        }
        show();
        Increment(b);
}

//updates the Fade pattern
void LightSignal::FadeUpdate(LightParameter *b)
{
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(b->Color1) * (b->totalsteps - b->index)) + (Red(b->Color2) * b->index)) / b->totalsteps;
        uint8_t green = ((Green(b->Color1) * (b->totalsteps - b->index)) + (Green(b->Color2) * b->index)) / b->totalsteps;
        uint8_t blue = ((Blue(b->Color1) * (b->totalsteps - b->index)) + (Blue(b->Color2) * b->index)) / b->totalsteps;

        ColorSet(b,Color(red, green, blue));
        show();
        Increment(b);
}

//Updates the Color set pattern
void LightSignal::ColorSet(LightParameter *p, uint32_t color)
{
        for (int i = 0; i < p->grouplength; i++)
        {
            setPixelColor(p->group[i], color);
        }
        show();
}

// Calculate 50% dimmed version of a color (used by ScannerUpdate)
uint32_t LightSignal::DimColor(uint32_t color)
{
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 2, Green(color) >> 2, Blue(color) >> 2);
        return dimColor;
}

// Returns the Red component of a 32-bit color
uint8_t LightSignal::Red(uint32_t color)
{
        return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t LightSignal::Green(uint32_t color)
{
        return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t LightSignal::Blue(uint32_t color)
{
        return color & 0xFF;
}

uint32_t LightSignal::Wheel(byte WheelPos)
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

//Updates the blink pattern
void LightSignal::BlinkUpdate(LightParameter *b)
{
    for(int i=0; i < b->grouplength;i++)
    {
        if(getPixelColor(b->group[i]) == 0 )                    //setting LED's to state HIGH
      {
        setPixelColor(b->group[i], b->Color1);
      }
      else if(getPixelColor(b->group[i]) > 0 )                  //setting LED's to LOW state
      {
          setPixelColor(b->group[i],0,0,0,0);
      }
    }
    show();
}

//Updates the On and Off pattern
void LightSignal::OnOffUpdate(LightParameter *b)
{
    for(int i=0; i < b->grouplength;i++)
    {
      if(getPixelColor(b->group[i]) == 0)                 //setting LED's to state HIGH
      {b->interval = b->onTime;
        setPixelColor(b->group[i], b->Color1);
      }
      else if(getPixelColor(b->group[i]) > 0 )            //setting LED's to LOW state
      {b->interval = b->offTime;
          setPixelColor(b->group[i],0,0,0,0);
      }
    }
    show();
}

//sets the brightness of the LED's
uint32_t LightSignal::Brightness(uint32_t color1 ,uint32_t intensity)
{
    uint32_t cul = max(max(Red(color1),Blue(color1)),Green(color1));

    uint32_t color = Color((Green(color1)/cul)*intensity,(Red(color1)/cul)*intensity,(Blue(color1)/cul)*intensity);
    return color;
}

//Updates the Pulsating pattern
void LightSignal::PulsatingUpdate(LightParameter *b)
{
    setPixelColor(8,b->Color1);
   // uint32_t color = getPixelColor(10);

    uint32_t min1 = min(min(Red(b->Color1),Blue(b->Color1)),Green(b->Color1));
    uint32_t max1 = max(max(Red(b->Color1),Blue(b->Color1)),Green(b->Color1));
    //setPixelColor(10, Brightness(color, b->Color1, 255));
    if( b->ledstate == false)
    {
        (b->totalsteps)++;
        for(int i = 0; i < b->grouplength; i++)
        {
            setPixelColor(b->group[i], Brightness(b->Color1, b->totalsteps));
        }
        if( b->totalsteps >= max1)
        {
            b->ledstate = true;
        }
    }
    else if( b->ledstate == true)
    {
        --b->totalsteps;
        for(int i = 0; i < b->grouplength; i++)
        {
            setPixelColor(b->group[i], Brightness(b->Color1, b->totalsteps));
        }
        if( b->totalsteps <= min1)
        {
            b->ledstate = false;
        }
    }
    show();
}

//Updates the Loading bar pattern
void LightSignal::LoadingUpdate(LightParameter *b)                   //function for Loading bar
{
    if( b->index < b->grouplength)
    {
    setPixelColor(b->group[b->index], b->Color1);
    show();
    Increment(b);
    }
    else if(b->index >= b->grouplength)
    {
        OnComplete(b);
        b->index = 0;

        //Increment(b);
    }

}


//Updates the step pattern
void LightSignal::StepUpdate(LightParameter *b)                          //function to turn one LED ON at a time in a group
{
    if(b->index >= b->grouplength)
     {
         b->index = 0;
     }
     for( int i = 0; i < b->grouplength; i++)
     {
            if( b->index != i)
            {
                setPixelColor(b->group[i], 0, 0, 0, 0);
            }
            else
            {
                setPixelColor(b->group[i], b->Color1);
            }
     }

     show();
    Increment(b);
}

