
// - V3.0 ZX_RASPBERRY_SCANNER -

/*  Copyright 2015 PJ Evans (http://mrpjevans.com/)

    Based on code Copyright 2013 James E Thompson (http://altoidsdrone.com/)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
This is the keyboard scanner for a ZX Spectrum 16/48K for the Pro Micro controller
   Wiring: Use with pullup resistor to VCC on all Data lines
   This Software: Cycle Addresses to 0V and scan Data lines for drop caused by a
   button press. Then output the appropriate USB command.
*/

//
// Keyboard Matrix
//

// The columns and rows of the ZX's keyboard matrix
const int dataNo = 5;
const int addressNo = 8;

// Map the microcontroller's pins to array dimensions
int dataPin[dataNo] = {A0, A1, A2, A3, 15}; // Pro Micros don't have an A4
int data[dataNo] = {0, 1, 2, 3, 4};
int address[addressNo] = {2, 3, 4, 5, 6, 7, 8, 9};

// The keyboard map
char keyMap[dataNo][addressNo] =
{// 2   3   4   5   6   7   8   9   << address lines

  {'n','j','c','u','7','f','r','4'},       // A0
  {'m','k','x','i','8','d','e','3'},       // A1
  {' ','l','z','o','9','s','w','2'},       // A2
  {' ',' ',' ','p','0','a','q','1'},       // A3
  {'b','h','v','y','6','g','t','5'},       // 15
  
};

int specialKeyMap[dataNo][addressNo] =
{// 2   3   4   5   6   7   8   9   << address lines

  {0,0,0,0,KEY_F7,0,0,KEY_F4},                             // A0
  {0,0,0,0,KEY_F8,0,0,KEY_F3},                             // A1
  {0,0,0,KEY_F11,KEY_F9,0,0,KEY_F2},                       // A2
  {KEY_ESC,KEY_RETURN,0,KEY_F12,KEY_F10,0,0,KEY_F1},       // A3
  {0,0,0,0,KEY_F6,0,0,KEY_F5},                             // 15
  
};

int specialKeyMapShift[dataNo][addressNo] =
{// 2   3   4   5   6   7   8   9   << address lines

  {0,0,0,0,KEY_UP_ARROW,0,0,0},                           // A0
  {0,0,0,0,KEY_RIGHT_ARROW,0,0,0},                        // A1
  {0,0,0,0,0,0,0,0},                                      // A2
  {0,0,0,0,KEY_BACKSPACE,0,0,0},                          // A3
  {0,0,0,0,KEY_DOWN_ARROW,0,0,KEY_LEFT_ARROW},            // 15
  
};

// Need to track keypresses so we dont over-send 'Release' commands, these overwhelm the Raspberry Pi
bool keyPressed[dataNo][addressNo] =
{// 2   3   4   5   6   7   8   9   << address lines

  {false,false,false,false,false,false,false,false},       // A0
  {false,false,false,false,false,false,false,false},       // A1
  {false,false,false,false,false,false,false,false},       // A2
  {false,false,false,false,false,false,false,false},       // A3
  {false,false,false,false,false,false,false,false},       // 15
  
};

// Some other globals
boolean debug = false;                    // Output debug information to the serial monitor
boolean capsShift = false;                // Has CAPS SHIFT been pressed? (Used by Special Keys mode)
char out;                                 // Keypress
boolean specialKeyModeSwitch = false;     // Monitors the state of the special key mode switch
boolean specialKeyMode = false;           // Toggle state for special key mode
int specialKeyVal;                        // In special key mode, what is the key value pressed?

//
// Keyboard Interface
//

void setup()
{

  // Do we output debug information? If so, open the port
  if (debug) Serial.begin(9600);

  // Set the address lines to outputmode and prep (high)
  for (int a = 0; a < addressNo; a++)
  {
    pinMode(address[a], OUTPUT);
    digitalWrite(address[a], HIGH);
  }

  // Set the data lines to input
  for (int d = 0; d < dataNo; d++)
  {
    pinMode(dataPin[d], INPUT);
  }

  // Keyboard mode switch
  pinMode(14, INPUT);

  // intialise keyboard connection
  Keyboard.begin();

}// end of setup



void loop()
{

  // Read the keyboard mode switch
  if(digitalRead(14) == LOW){

    // Flag the switch as pressed
    if(!specialKeyModeSwitch) specialKeyModeSwitch = true;

    if(debug) Serial.println("Special Key Mode Switch Pressed");
    
  } else if(specialKeyModeSwitch){ // If the key is flagged as pressed

    // Mark it as released
    specialKeyModeSwitch = false;

    // Now toggle mode
    specialKeyMode = !specialKeyMode;

    // Reset
    Keyboard.releaseAll();
    
    if(debug){
      Serial.print("Special key mode ");
      Serial.println(specialKeyMode);
    }
    
  }

  // cycle the address lines to low for scanning, then reset
  for (int aCyc = 0; aCyc < addressNo; aCyc++)
  {

    // Set the address line to low, so it's the only one that triggers
    digitalWrite(address[aCyc], LOW);

    // Scan each of the data lines for drop (keypress)
    for (int tData = 0; tData < dataNo; tData++)
    {

      // Read the data line
      int pressed = digitalRead(dataPin[tData]);

      // If 0, the line has gone low, so a key has been pressed
      if (pressed == LOW)
      {

        // Are we in special key mode?
        if(specialKeyMode){

            // Check on Caps Shift
            if(tData == 3 && aCyc == 2) {
              
              capsShift = true;
              
            } else {

              if(capsShift){

                specialKeyVal = specialKeyMapShift[tData][aCyc];
                
              } else {

                specialKeyVal = specialKeyMap[tData][aCyc];
                
              }

              if(specialKeyVal != 0){

                if (debug) {
                Serial.print(tData);
                Serial.print(" ");
                Serial.print(aCyc);
                Serial.print(" ");
                Serial.println(specialKeyVal);
              }

              Keyboard.press(specialKeyVal);
              
              keyPressed[tData][aCyc] = true;
                
              }
              
            }

        } else {

            // Deal with modifier keys first
            if(tData == 3 && aCyc == 2  ) {       // Caps
  
              Keyboard.press(KEY_LEFT_SHIFT);
              
              // Debug
              if(debug) Serial.println("CAPS");            
  
            } else if(tData == 3 && aCyc == 1) {   // Enter
                
              Keyboard.press(KEY_RETURN);

              // Debug
              if(debug) Serial.println("ENTER");
  
            } else if(tData == 2 && aCyc == 0) {   // Symbol Shift
  
              Keyboard.press(KEY_LEFT_ALT);
  
              // Debug
              if(debug) Serial.println("SYM SHIFT");
            
            } else {

              // Find key
              out = keyMap[tData][aCyc];

              // Write out the row and column coordinates to the serial monitor if required
              if (debug) {
                Serial.print(tData);
                Serial.print(" ");
                Serial.print(aCyc);
                Serial.print(" ");
                Serial.println(out);
              }
              
              // Send the keypress
              Keyboard.press(out);
                
            }
            
            // Remember the keypress
            keyPressed[tData][aCyc] = true;
              
          }

        } else {

          // This matrix point has not been pressed, if it has been previously, release it
          if (keyPressed[tData][aCyc] == true) {
            if(specialKeyMode){
              if(capsShift){
                Keyboard.release(specialKeyMapShift[tData][aCyc]);
              } else {
                Keyboard.release(specialKeyMap[tData][aCyc]);
              }
            } else if(tData == 3 && aCyc == 2) {
              Keyboard.release(KEY_LEFT_SHIFT);
           } else if(tData == 3 && aCyc == 1) {
              Keyboard.release(KEY_RETURN);
           } else if(tData == 2 && aCyc == 0) {
              Keyboard.release(KEY_LEFT_ALT);
           } else {
              Keyboard.release(keyMap[tData][aCyc]);
            }
          }
          keyPressed[tData][aCyc] == false;
          
          // Cancel Caps Shift for special key mode if no longer pressed
          if(tData == 3 && aCyc == 2) capsShift = false;
          
        }

    }

    // Set the line high again
    digitalWrite(address[aCyc], HIGH);

  }

}


