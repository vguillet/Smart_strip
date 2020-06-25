#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

#include <Adafruit_NeoPixel.h>

// =========================================================================================== Intialisation
// ---------------------------------------------------------- IR initialisation
int RECV_PIN = 21;
IRrecv irrecv(RECV_PIN);
decode_results results;

// ---------------------------------------------------------- Strip initialisation
// --> Define strip properties and connectic
int LED_PIN = 51;
int LED_COUNT = 60;

// --> NeoPixel brightness, 0 (min) to 255 (max)
int BRIGHTNESS = 255;

uint32_t  hexSignal = 0xFFE21D;
uint32_t state = 0xFF38C7;
int color_tracker = 60;
int default_step_size = 20;

// --> Initialte remote code array
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


// ---------------------------------------------------------- Interrupt tracker initialisation
boolean run_loop = true;
boolean hold_lock = false;

// =========================================================================================== Setup
void setup()
{
  // --> Initialise IR receiver
  Serial.begin(9600); // Initialise serial to monitor ir signals
  irrecv.enableIRIn(); // Start the receiver
  
  // --> Initialise NeoPixel strip object
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)  

  // --> Setup interrupt
  attachInterrupt(digitalPinToInterrupt(21), adjustState, CHANGE);
}

// =========================================================================================== Run
void loop()
{
 applySignal();
 
//  if (irrecv.decode(&results)){
//     Serial.println(results.value, HEX);
//
//     // --> Check whether signal should be processed
//     for (int i=0; i < 17; i++){
//        if (controller_signals[i] == results.value){
//          applySignal();
//          break;
//          }
//        }
//
//       //     Serial.println(power);
//     Serial.print("Internal state: "); 
//     Serial.println(state, HEX);
//     
//     Serial.print("Color tracker: ");
//     Serial.println(color_tracker);
//     
//     Serial.print("Brightness: ");
//     Serial.println(BRIGHTNESS);   
//     Serial.print("\n"); 
//     irrecv.resume(); // Receive the next value
//     }
     
  run_loop = true; // Reset run_loop  
}

// =========================================================================================== Define interrupts
void adjustState(){
  if (irrecv.decode(&results)){
    Serial.println(results.value, HEX);
  
    for (int i=0; i < 17; i++){
          if (controller_signals[i] == results.value or hold_lock == true){
            hexSignal = results.value;
            run_loop = false;
            break;
            }
          }
    Serial.println(hexSignal, HEX);
    Serial.print("\n");
    irrecv.resume(); // Receive the next value
    }
  } 

// =========================================================================================== Define of functions
void applySignal() 
  {
     // ----------------------------------------- Brightness      
     // --> Set dim down
     if (hexSignal == 0xFFA25D){
       if (BRIGHTNESS > 0){
         BRIGHTNESS = BRIGHTNESS - 25;
         strip.setBrightness(BRIGHTNESS);
         }
       }

     // --> Set dim up
     else if (hexSignal == 0xFF629D){
       if (BRIGHTNESS <= 230){
         BRIGHTNESS = BRIGHTNESS + 25;
         strip.setBrightness(BRIGHTNESS);
         }
       }

     // --> Set state
     setState(hexSignal); 
     
     // ------------------------------------------------- Turn off led 
     if (hexSignal == 0xFFE21D){
      strip.fill(strip.Color(0, 0, 0, 0));
      strip.show();

      hold("Off");
      } 

     // ------------------------------------------------- Set to white
     else if (state == 0xFF38C7){
      colorWipe(strip.Color(0, 0, 0, 255)  , 40);

      hold("White");
      }
      
     // ------------------------------------------------- Set to Red
     else if (state == 0xFF6897){
      colorWipe(strip.Color(255, 0, 0)  , 40);
      color_tracker = 0;

      hold("Red");
      }

     // ------------------------------------------------- Set to Green
     else if (state == 0xFF9867){
      colorWipe(strip.Color(0,   255,   0)     , 40);
      color_tracker = 20;

      hold("Green");
      }

     // ------------------------------------------------- Set to Blue
     else if (state == 0xFFB04F){
      colorWipe(strip.Color(0,   0,   255)     , 40);
      color_tracker = 240;

      hold("Blue");
      }

     // ------------------------------------------------- Set to color cycle
     else if (state == 0xFF02FD){
      rainbowCycle(20);
      run_loop = 1;
      }     

     // ------------------------------------------------- Set to color mash
     else if (state == 0xFF22DD){
//    theaterChase(strip.Color(127, 127, 127), 50); // White
      theaterChaseRainbow(50);
      run_loop = 1;
      } 
 
     // ------------------------------------------------- Move to fav 1
     else if (state == 0xFFE01F){
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();

      hold("Fav 1");
      } 

     // ------------------------------------------------- Move to fav 2
     else if (state == 0xFFA857){   
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();

      hold("Fav 2");
      } 

     // ------------------------------------------------- Move to clock
     else if (state == 0xFF906F){
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();

      hold("Timer");
      } 
         
     // ------------------------------------------------- Move to wheel red
     else if (state == 0xFF30CF){
      step_towards_color(0); 
     
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();

      hold("Wheel red");
      } 
     
     // ------------------------------------------------- Move to wheel yellow
     else if (state == 0xFF7A85){
      step_towards_color(20); 
     
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();

      hold("Wheel yellow");
      }  

     // ------------------------------------------------- Move to wheel green
     else if (state == 0xFF52AD){
      step_towards_color(80); 
     
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();

      hold("Wheel green");
      }  

     // ------------------------------------------------- Move to wheel blue
     else if (state == 0xFF42BD){
      step_towards_color(170);
     
      // -- > Apply new color
      strip.fill(Wheel(color_tracker));
      strip.show();

      hold("Wheel blue");
      }

     // ------------------------------------------------- Hold to catch all unwanted signal
     else {
      hold_lock = true;
      hold("Hold lock");
      hold_lock = false;
      }
   }  

// =========================================================================================== Def state modifiers
void hold(String tracker){
  while(run_loop){
    Serial.println(tracker);   // Add tracker to see what holds
    delay(0);
    }
  run_loop = 1;
  }
  
void setState(uint32_t hexSignal){
   if (hexSignal != 0xFFE21D   // Off
  and hexSignal != 0xFFC23D    // On
  and hexSignal != 0xFFA25D    // Dim down
  and hexSignal != 0xFF629D    // Dim up
  and hexSignal != 0
  ){state = hexSignal;}
  }

void step_towards_color(int goal_color){
  // --> Declare variables
  int current_quadrant;
  int goal_edge;
  int step_size;
  int turn;
 
  // --> Find current quadrant
  if (0 <= color_tracker && color_tracker <= 20){current_quadrant = 4;}
  else if (20 < color_tracker && color_tracker <= 80){current_quadrant = 3;}
  else if (80 < color_tracker && color_tracker < 170){current_quadrant = 2;}
  else {current_quadrant = 1;}
  
  // --> Find edge goal
  if (goal_color == 0){goal_edge = 1;}
  else if (goal_color == 20){goal_edge = 4;}
  else if (goal_color == 80){goal_edge = 3;}
  else {goal_edge = 2;} 
 
  // --> Find step size
  if (current_quadrant == 4){step_size = default_step_size/4;}
  else if (current_quadrant == 3){step_size = default_step_size/2;}
  else if (current_quadrant == 2){step_size = default_step_size;}
  else {step_size = default_step_size;}
    
  // --> Find turn direction
  if (goal_edge == 1){
    if (current_quadrant == 1 or current_quadrant == 2){turn = 0;}
    else {turn = 1;}
    } 
  
  if (goal_edge == 2){
    if (current_quadrant == 2 or current_quadrant == 3){turn = 0;}
    else {turn = 1;}
    } 

  if (goal_edge == 3){
    if (current_quadrant == 3 or current_quadrant == 4){turn = 0;}
    else {turn = 1;}
    } 

  if (goal_edge == 4){
    if (current_quadrant == 4 or current_quadrant == 1){turn = 0;}
    else {turn = 1;}
    } 
  
  // --> Return (scaled) step size
  if (turn == 0){
    if (goal_color > color_tracker){
      if (goal_color - color_tracker < step_size){step_size = goal_color - color_tracker;}
      }
      
    else{
      if (goal_color + (255 - color_tracker) < step_size){step_size = goal_color + (255 - color_tracker);}
      }
    }
    
  else{
    if (goal_color < color_tracker){
      if (color_tracker - goal_color < step_size){step_size = -(color_tracker - goal_color);}
      else {step_size = -step_size;}
      }

    else {
      if ((color_tracker + 255) - goal_color < step_size){step_size = -(color_tracker + 255) - goal_color;}
      else {step_size = -step_size;}
      }
    }
    
  // --> Apply step if not at goal
  if (color_tracker != goal_color){color_tracker = color_tracker + step_size;}

  // --> Loop if past 255 range
  if (color_tracker > 255){color_tracker = color_tracker - 255;}
  else if (color_tracker < 0){color_tracker = color_tracker + 255;}
  
  }

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  
  while(run_loop){
    Serial.println("Color cycle");   // Add tracker to see what holds
    for(j=0; j<256; j++) { // Cycles of all colors on wheel
      
      if (run_loop == 0){break;} // Break out if new signal
      
      else{
        for(i=0; i< strip.numPixels(); i++) {
          strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
          }
        }
        
    strip.show();
    delay(wait);
    }
  }
}
