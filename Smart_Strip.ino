#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

#include <Adafruit_NeoPixel.h>

// ---------------------------------------------------------- IR initialisation
int RECV_PIN = 52;
IRrecv irrecv(RECV_PIN);
decode_results results;

// ---------------------------------------------------------- Strip initialisation
// --> Define strip properties and connectic
int LED_PIN = 51;
int LED_COUNT = 10;

// --> NeoPixel brightness, 0 (min) to 255 (max)
int BRIGHTNESS = 25;
int power = 0;

uint32_t state = 0xFF38C7;
int color_tracker = 255;

int default_step_size = 40;
int current_quadrant;
int goal_edge;
int step_size;
int turn;

// --> Setup remote code array
uint32_t controller_signals[17] = {0xFFE21D, // Off
                                   0xFFC23D, // On
                                   0xFFA25D, // Dim down
                                   0xFF629D, // Dim up
                                   0xFF22DD, // Color mash
                                   0xFF02FD, // Color cycle
                                   0xFFE01F, // Fav 1
                                   0xFFA857, // Fav 2
                                   0xFF906F, // Timer
                                   0xFF6897, // Red
                                   0xFF9867, // Green
                                   0xFFB04F, // Blue
                                   0xFF38C7, // White
                                   0xFF30CF, // Wheel red
                                   0xFF7A85, // Wheel yellow
                                   0xFF42BD, // Wheel blue
                                   0xFF52AD // Wheel green
                                   };

// --> Declare NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// ---------------------------------------------------------- Setup
void setup()
{
  // --> Initialise IR receiver
  Serial.begin(9600); // Initialise serial to monitor ir signals
  irrecv.enableIRIn(); // Start the receiver
  
  // --> Initialise NeoPixel strip object
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)  
}

// ---------------------------------------------------------- Run
void loop()
{
  if (irrecv.decode(&results)){
     Serial.println(results.value, HEX);

     // --> Check whether signal should be processed
     for (int i=0; i < 17; i++){
        if (controller_signals[i] == results.value){
          applySignal(results.value);
          break;
          }
        }
        
     Serial.print("\n"); 
     irrecv.resume(); // Receive the next value
     }
}

void applySignal(uint32_t hexSignal) 
  {
     // ----------------------------------------- Control on/off state, brightness
     // --> Turn off led 
     if (hexSignal == 0xFFE21D){
        strip.fill(strip.Color(0, 0, 0, 0));
        strip.show();
        power = 0;
        }

     // --> Turn on strip/apply state 
     else if (hexSignal == 0xFFC23D){
        power = 1;
        }
        
     // --> Set dim down
     else if (hexSignal == 0xFFA25D and power == 1){
       if (BRIGHTNESS > 0){
         BRIGHTNESS = BRIGHTNESS - 25;
         strip.setBrightness(BRIGHTNESS);
         }
       }

     // --> Set dim up
     else if (hexSignal == 0xFF629D and power == 1){
       if (BRIGHTNESS <= 230){
         BRIGHTNESS = BRIGHTNESS + 25;
         strip.setBrightness(BRIGHTNESS);
         }
       
       }

     // --> Set new state 
     else if (power == 1){
       state = hexSignal; 
       } 
        
     // ------------------------------------------------- Set to white
     if (state == 0xFF38C7 and power == 1){
       colorWipe(strip.Color(0, 0, 0, 255)  , 50);
//       color = 
       }
      
     // ------------------------------------------------- Set to Red
     else if (state == 0xFF6897 and power == 1){
       colorWipe(strip.Color(255, 0, 0)  , 50);
       color_tracker = 0;
       }

     // ------------------------------------------------- Set to Green
     else if (state == 0xFF9867 and power == 1){
       colorWipe(strip.Color(0,   255,   0)     , 50);
       color_tracker = 120;
       }

     // ------------------------------------------------- Set to Blue
     else if (state == 0xFFB04F and power == 1){
       colorWipe(strip.Color(0,   0,   255)     , 50);
       color_tracker = 240;
       }

     // ------------------------------------------------- Set to color cycle
     else if (state == 0xFF02FD and power == 1){
       rainbowCycle(20);
       }     

     // ------------------------------------------------- Set to color mash
     else if (state == 0xFF22DD and power == 1){
//       theaterChase(strip.Color(127, 127, 127), 50); // White
       theaterChaseRainbow(50);
       } 

     // ------------------------------------------------- Move to wheel red
     else if (state == 0xFF30CF and power == 1){
      color_tracker = color_tracker + step_towards_color(0); 
     
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();
      } 
     
     // ------------------------------------------------- Move to wheel yellow
     else if (state == 0xFF30CF and power == 1){
      color_tracker = color_tracker + step_towards_color(60); 
     
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();
      }  

     // ------------------------------------------------- Move to wheel green
     else if (state == 0xFF30CF and power == 1){
      color_tracker = color_tracker + step_towards_color(120); 
     
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();
      }  

     // ------------------------------------------------- Move to wheel blue
     else if (state == 0xFF30CF and power == 1){
      color_tracker = color_tracker + step_towards_color(240); 
     
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();
      } 

       
//     Serial.println(power);
     Serial.println(state, HEX);
     Serial.println(color_tracker);
   }



void step_towards_color(int goal_color){
  // --> Find current quadrant
  if (0 < color_tracker =< 60){int current_quadrant = 1;}
  else if (60 < color_tracker =< 120){int current_quadrant = 2;}
  else if (120 < color_tracker =< 240){int current_quadrant = 3;}
  else {int current_quadrant = 4;}

  // --> Find edge goal
  if (goal_color == 0){int goal_edge = 1;}
  else if (goal_color == 60){int goal_edge = 2;}
  else if (goal_color == 120){int goal_edge = 3;}
  else {int goal_edge = 4;} 

  // --> Find step size
  if (current_quadrant == 1 or current_quadrant == 2){int step_size = default_step_size/2;}
  else {int step_size = default_step_size;}
  
  // --> Find turn direction
  if (goal_edge == 1){
    if (current_quadrant == 1 or current_quadrant == 2){int turn = 0;}
    else {int turn = 1;}
    } 
  
  if (goal_edge == 2){
    if (current_quadrant == 2 or current_quadrant == 3){int turn = 0;}
    else {int turn = 1;}
    } 

  if (goal_edge == 3){
    if (current_quadrant == 3 or current_quadrant == 4){int turn = 0;}
    else {int turn = 1;}
    } 

  if (goal_edge == 4){
    if (current_quadrant == 4 or current_quadrant == 1){int turn = 0;}
    else {int turn = 1;}
    } 
  
  // --> Return (scaled) step size
  if (turn == 0){
    if (color_tracker + step_size > goal_color){return goal_color - color_tracker;}
    else {return step_size;} 
    }
  else{
    if (color_tracker - step_size < goal_color){return -(color_tracker - goal_tracker);}
    else {return - step_size;} 
    }
  }
