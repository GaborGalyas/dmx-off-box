#include <DMXSerial2.h>
#include <EEPROM.h>
const uint16_t my_pids[] = {E120_LAMP_HOURS};
const RDMPERSONALITY my_personalities[] = {
  {1, "Normally Open"},
  {1, "Normally Closed"},
  {2, "Set / Reset"}
};
/*
const RDMSENSOR my_sensors[] = {
  {E120_SENS_TEMPERATURE, E120_UNITS_CENTIGRADE, E120_PREFIX_NONE, 0, 80, 10, 40, true, false, "Temperature"}
};
*/
struct RDMINIT rdmInit = {
  "Gabor Galyas Lighting", // Manufacturer Label
  1, // Device Model ID
  "DMX Off Box", // Device Model Label
  1, // Default RDM personality (1 based)
  (sizeof(my_personalities)/sizeof(RDMPERSONALITY)), my_personalities,  
  (sizeof(my_pids)/sizeof(uint16_t)), my_pids,
 // (sizeof(my_sensors)/sizeof(RDMSENSOR)), my_sensors
};

unsigned long previousMillis = 0;
unsigned long prevMillis = 0;
unsigned long prevblinky = 0;
unsigned long blinkytime = 500;
float temperature = 0.0;
int16_t _lowestValue = 0;
int16_t _highestValue = 0;

int startaddress = 0;
//int dippins[] = {3, 4, 5, 6, 7, 8, 9, 10, 11};       // DIP Switch Pins
int relay  = 7;
int led  = 13;
int ledstate = LOW;
int TriggerPoint;


void setup () {
  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  digitalWrite(relay, LOW);
 
// for(int y = 0; y<=8; y++){
//   pinMode(dippins[y], INPUT_PULLUP);
//  }
 // DMXSerial2.init(&rdmInit, processCommand,getSensorValue);
  DMXSerial2.init(&rdmInit, processCommand);

} // setup()

void(* resetFunc) (void) = 0;


void loop() {
//startaddress = DMXSerial2.dipaddress();
TriggerPoint = EEPROM.read(512);
   
    switch (DMXSerial2.getPersonalityNumber()) {
      case 1:                       ////////////// Normally open
        if (startaddress == 0){
            if (DMXSerial2.readRelative(0) > TriggerPoint){
            digitalWrite(relay, HIGH);
            blinkytime = 100;
          } else {
            digitalWrite(relay, LOW);
            blinkytime = 500;
          }
        } else {
            if (DMXSerial2.read(startaddress) > TriggerPoint){
            digitalWrite(relay, HIGH);
            blinkytime = 100;
          } else {
            digitalWrite(relay, LOW);
            blinkytime = 500;          } 
        }  
        break;
        
      case 2:                       ////////////// Normally closed
          if (startaddress == 0){
            if (DMXSerial2.readRelative(0) < TriggerPoint){
            digitalWrite(relay, HIGH);
            blinkytime = 100;
          } else {
            digitalWrite(relay, LOW);
            blinkytime = 500;
          }
        } else {
            if (DMXSerial2.read(startaddress) < TriggerPoint){
            digitalWrite(relay, HIGH);
            blinkytime = 100;
          } else {
            digitalWrite(relay, LOW);
            blinkytime = 500;
          } 
        }
        break;

        case 3:                       ////////////// Set / reset
          if (startaddress == 0){
            if (DMXSerial2.readRelative(0) > TriggerPoint && DMXSerial2.readRelative(1) < TriggerPoint){
            digitalWrite(relay, HIGH);
            digitalWrite(led, HIGH);
            blinkytime = 100;
          } else if (DMXSerial2.readRelative(0) < TriggerPoint && DMXSerial2.readRelative(1) > TriggerPoint){
            digitalWrite(relay, LOW);
            digitalWrite(led, LOW);
            blinkytime = 500;
          }
        } else {
            if (DMXSerial2.read(startaddress) > TriggerPoint && DMXSerial2.read(startaddress + 1) < TriggerPoint){
            digitalWrite(relay, HIGH);
            digitalWrite(led, HIGH);
            blinkytime = 100;
          } else if (DMXSerial2.read(startaddress) < TriggerPoint && DMXSerial2.read(startaddress + 1) > TriggerPoint){
            digitalWrite(relay, LOW);
            digitalWrite(led, LOW);
            blinkytime = 500;
          }
        }
        break;
    }

      if (DMXSerial2.isIdentifyMode() == true) {
          if (millis() - prevMillis >= 50) {
              prevMillis = millis();
              digitalWrite(led, !ledstate);
              ledstate = !ledstate;
          }
          } else {
            if (DMXSerial2.noDataSince() > 250) {digitalWrite(led, LOW);}
         }
   if (DMXSerial2.noDataSince() <= 200){
    blinky();
   }
   else{
    if (DMXSerial2.isIdentifyMode() == false) {digitalWrite(led, LOW);}
   }
  
  /*
  if (millis() - previousMillis >= 10000) {
    previousMillis = millis();
  //   sensors.requestTemperatures(); 
    temperature = DMXSerial2.dipaddress(); 
  //  temperature = (sensors.getTempCByIndex(0));
    if (!isnan(temperature)) {
      if (temperature < _lowestValue) {
        _lowestValue = temperature;
      }
      if (temperature > _highestValue) {
        _highestValue = temperature;
      }
    }
  }
 */ 
  DMXSerial2.tick();

} // loop()

void blinky(){
 if (millis() - prevblinky >= blinkytime) {
              prevblinky = millis();
              digitalWrite(led, !ledstate);
              ledstate = !ledstate;
          }
}


bool8 processCommand(struct RDMDATA *rdm, uint16_t *nackReason)
{
  byte CmdClass       = rdm->CmdClass;     // command class
  uint16_t Parameter  = rdm->Parameter;     // parameter ID
  bool8 handled = false;


// This is a sample of how to return some device specific data
  if (Parameter == SWAPINT(E120_LAMP_HOURS)) { // 0x0400
    if (CmdClass == E120_GET_COMMAND) {
      if (rdm->DataLength > 0) {
        // Unexpected data
        *nackReason = E120_NR_FORMAT_ERROR;
      } else if (rdm->SubDev != 0) {
        // No sub-devices supported
        *nackReason = E120_NR_SUB_DEVICE_OUT_OF_RANGE;
      } else {
        rdm->DataLength = 4;
        rdm->Data[0] = 0;
        rdm->Data[1] = 0;
        rdm->Data[2] = 0;
        rdm->Data[3] = EEPROM.read(512);
        handled = true;
      }
    } else if (CmdClass == E120_SET_COMMAND) {
      
      if (rdm->DataLength < 1) {
          // Oversized data
          *nackReason = E120_NR_FORMAT_ERROR;
        } else if (rdm->SubDev != 0) {
          // No sub-devices supported
          *nackReason = E120_NR_SUB_DEVICE_OUT_OF_RANGE;
        }   
    else {
    
          uint16_t newtrigpoint = rdm->Data[3];
          
          if ((newtrigpoint <= 0) || (newtrigpoint > 255)) {
            // Out of range start address
            *nackReason = E120_NR_DATA_OUT_OF_RANGE;
          } else {
         
            int trigpt = newtrigpoint;
            rdm->DataLength = 0;
            // persist in EEPROM
            EEPROM.update(512, trigpt);
            handled = true;
            digitalWrite(led, HIGH);
          delay(300);
          digitalWrite(led, LOW);
          delay(300);
          digitalWrite(led, HIGH);
          delay(300);
          digitalWrite(led, LOW);

          }
        }
    
    
    }

  }  else if (Parameter == SWAPINT(E120_RESET_DEVICE)) { ////////////RESET
    if (CmdClass == E120_GET_COMMAND) {
      *nackReason = E120_NR_UNSUPPORTED_COMMAND_CLASS;
    } else if (CmdClass == E120_SET_COMMAND) {
      resetFunc(); //call reset
      handled = true;
    }
  }
  /*
  else if (Parameter == SWAPINT(E120_LAMP_HOURS)) { // 0x0400           ////////////// SET TRIGPOINT
    if (CmdClass == E120_GET_COMMAND) {
      if (rdm->DataLength > 0) {
        // Unexpected data
        *nackReason = E120_NR_FORMAT_ERROR;
      } else if (rdm->SubDev != 0) {
        // No sub-devices supported
        *nackReason = E120_NR_SUB_DEVICE_OUT_OF_RANGE;
      } else {
        rdm->DataLength = 4;
        rdm->Data[3] = EEPROM.read(512);
        handled = true;
      }
    } else if (CmdClass == E120_SET_COMMAND) {
      
      if (rdm->DataLength != 4) {
          // Oversized data
          *nackReason = E120_NR_FORMAT_ERROR;
        } else if (rdm->SubDev != 0) {
          // No sub-devices supported
          *nackReason = E120_NR_SUB_DEVICE_OUT_OF_RANGE;
        }   
    else {
    
          uint16_t newtrigpoint = rdm->Data;
          
          if ((newtrigpoint <= 0) || (newtrigpoint > 255)) {
            // Out of range start address
            *nackReason = E120_NR_DATA_OUT_OF_RANGE;
          digitalWrite(led, HIGH);
          delay(100);
          digitalWrite(led, LOW);
          delay(100);
          digitalWrite(led, HIGH);
          delay(100);
          digitalWrite(led, LOW);
      
          } else {
         
            int trigpt = newtrigpoint;
            rdm->DataLength = 0;
            // persist in EEPROM
            EEPROM.update(512, trigpt);
            handled = true;
            digitalWrite(led, HIGH);
          delay(300);
          digitalWrite(led, LOW);
          delay(300);
          digitalWrite(led, HIGH);
          delay(300);
          digitalWrite(led, LOW);

          }
        }
    
    }

  }
 */ 
  return handled;
} // processCommand
/*
bool8 getSensorValue(uint8_t sensorNr, int16_t *value, int16_t *lowestValue, int16_t *highestValue, int16_t *recordedValue) {
  if (sensorNr == 0) {
    *value = temperature;
    *lowestValue = _lowestValue;
    *highestValue = _highestValue;
    return true;
  }
  return false;
} // getSensorValue

*/
