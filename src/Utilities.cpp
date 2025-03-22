/*
 * Utilities.cpp library for utility methods for IoT applications
 * Created by Sachi Gerlitz
 * 
 * constructor:   Utilities
 * methods:       begin; InfoStamp; LedIndicator; Print4FreeHeap; ProgMem_Len;
 *                UtCheckSum; UtCheckSum16; TimestampToString; (FbufTime); FbufInt; PrintEmailBuf; waitForUserInput; 
 *                ESPmemUsage; PrintIamAlive; uint16ToChars; String2Bool; getVersion;
 * 
 * 16-II-2025   ver 2.7   [String2Bool]
 * 31-I-2025    ver 2.6   [parse '&emsp;']
 * 14-I-2025    ver 2.4   [correct <uint16ToChars>] 
 * 02-I-2025    ver 1.3   [add <uint16ToChars>]
 * 25-XII-2024  ver 1.2   [add <PrintIamAlive>]
 * 08-IX-2024   ver 1.1   [<ESPmemUsage>]
 * 25-VIII-2024 ver 1     [<InfoStamp> parameters]
 * 12-VI-2024 ver 0
 *
 */
#include  "Arduino.h"
//#include  "avr/pgmspace.h"
#include  "Clock.h"
#include  "Utilities.h"
#define MaxMemBuffer  80  // maximum length of temp buffer to handle Flash Memory in RAM

//****************************************************************************************/
Utilities::Utilities(TimePack T) {
    _LT = T;
}     // end of Utilities 

//****************************************************************************************/
void Utilities::begin(uint8_t LEDPIN){
  pinMode(LEDPIN, OUTPUT);                // Set LED as output
  //
}     // end of begin

//****************************************************************************************/
void Utilities::InfoStamp (TimePack C, const char* StartHeader, const char* MainHeader, bool AckDbg, bool Terminate ) {
  /* 
  *  method to print time stamp and messages:
  *  {"ACK","DBG"}," 08:59 ",StartHeader," ",MainHeader,{" -END\n"," "}
  */
  if (AckDbg)   Serial.print(F("ACK "));
  else          Serial.print(F("DBG "));
  
  char buf[10];                           // Temp buffer to format time
  Serial.print(TimestampToString(C,buf));

  Serial.print(StartHeader);
  if (MainHeader!=nullptr) {
    Serial.print(F(" "));
    Serial.print(MainHeader);
  }
  
  if ( Terminate )  Serial.print(F(" -END\n"));
  else              Serial.print(F(" "));
}     // end of InfoStamp

//****************************************************************************************/
void  Utilities::LedIndicator(uint8_t LEDPIN, uint8_t typeState){
  /*
   * Procedure to drive the internal LED indicator according to: 
   *    1. <typeState> for no error condition
   *    2. for error condition
   * it uses a cycle of MAXCYCLE periods (the timers are setting the length of each period)
   * 
   * notes: for the LED 0 means OFF 1 means ON
   *        the rate of change is by the rate of calling the method!
   */
  const   uint8_t MAXCYCLE=16;
  const   uint8_t MAXSTATE=8;
  const   bool  indicationCycle[MAXSTATE][MAXCYCLE]= {
                            //  0 1 2 3 4 5 6 7 8 9 A B C D E F
                              { 1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0 },      // LedWifiSearch 0 search for WiFi to connect as station
                              { 1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0 },      // LedAPSearch   1 search for client to connect as WiFi Access Point
                              { 1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0 },      // LedWifiLost   2 WiFi Network connection lost
                              { 1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },      // LedNTPfailure 3 Network time NOT set (NTP failure)
                              { 1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0 },      // LedShadesOK   4 Shades controller normal operation
                              { 1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0 },      // LedShadePause 5 Shades controller script paused operation
                              { 1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0 },      // LedSDfailure  6 Error opening SD file
                              { 0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1 }       // LedTBD7       7 not in use
                              };
  static  uint8_t counter=0;

  if (indicationCycle[typeState][counter])  digitalWrite(LEDPIN, LOW);
  else                                      digitalWrite(LEDPIN, HIGH);
  counter++;
  if ( counter==MAXCYCLE ) counter=0;
}   // end of LedIndicator

//****************************************************************************************/
void  Utilities::Print4FreeHeap(TimePack C){
  /*
   * method to print the freeheap status 4 times every minute. The ticks are seconds: 4,19,34,49
   */
  static const char Mname[] PROGMEM = "Print4FreeHeap:";
  static const char L0[] PROGMEM = "Free Heap (15 sec tick) ";
  if ( C.secEdge&& ( C.clockSec==4 || C.clockSec==19 || C.clockSec==34 || C.clockSec==49 )) {
    InfoStamp(C,Mname,L0,0,0);  Serial.print(ESP.getFreeHeap(),DEC); Serial.print(F(" -END\n"));
  }
}     // end of Print4FreeHeap

//*********************************************************************************/
uint8_t  Utilities::ProgMem_Len( const char in_from_flash[] ){
  /*
   * method to figure out string length from Flash memory 
   * returns string length or 0
   * usage:
   * const static char abc[] PROGMEM = "string to be measured";
   * uint8_t abc_length=ProgMem_Length( abc );
   */
  // http://arduino.land/FAQ/content/4/26/en/how-to-use-dynamic-memory.html
  char* buf = new char[MaxMemBuffer];
  if (buf) { memcpy_P(buf, in_from_flash, MaxMemBuffer); }
  for ( uint8_t i=0; i<MaxMemBuffer ; i++ ) 
    if ( buf[i] == 0x00 ) { delete [] buf; return  i; }
  delete [] buf;
  return  0;  // no NULL found
}     // end of ProgMem_Len

/*********************************************************************************/
int Utilities::UtCheckSum(uint8_t  storeBuf[], uint8_t len){
  /*
   * method to calculate check sum of an int8_t array length <len> bytes
   */
  uint8_t CheckSum=0;
  for (uint8_t iii=0; iii<len; iii++) CheckSum ^= storeBuf[iii];
  return  CheckSum;  
}   // end of UtCheckSum

/*********************************************************************************/
int Utilities::UtCheckSum16(uint16_t  storeBuf[], uint8_t len){
  /*
   * method to calculate check sum of an int16_t array length <len> words
   */
  uint8_t CheckSum=0;
  for (uint8_t iii=0; iii<len; iii++) {
    CheckSum ^= highByte(storeBuf[iii]);
    CheckSum ^= lowByte (storeBuf[iii]);
  }
  return  CheckSum;  
}   // end of UtCheckSum16

/****************************************************************************************/
char*  Utilities::TimestampToString(TimePack C, char*  buf){
  /*
   * Method to convert system's time stamp to a string buffer <buf>, must be of 10 chars length
   *  hh:mm:ss 0
   *  0123456789
   */
  char* TimeStamp;
  TimeStamp=buf;
  *TimeStamp++ = char ( ( C.clockHour / 10 ) + 0x30 ) ;
  *TimeStamp++ = char ( ( C.clockHour % 10 ) + 0x30 ) ;
  *TimeStamp++ = ':';
  *TimeStamp++ = char ( ( C.clockMin / 10 ) + 0x30 ) ;
  *TimeStamp++ = char ( ( C.clockMin % 10 ) + 0x30 ) ;
  *TimeStamp++ = ':';
  *TimeStamp++ = char ( ( C.clockSec / 10 ) + 0x30 ) ;
  *TimeStamp++ = char ( ( C.clockSec % 10 ) + 0x30 ) ;
  *TimeStamp++ = ' ';
  *TimeStamp++ = '\0';
  return buf;
}   // end of TimestampToString

//*********************************************************************************/
char*  Utilities::FbufInt(uint16_t number, char* buf){ 
  /*
   * method to format uint16_t <Number> into <buf> as chars (buf must be 6 chars long)
   * limited to 3 chars, with leading and trailing blanks
   *   999 0
   *  012345
   *   ERR 0
   */
  #define MaxAllowedNumber  999
  char* pntr=buf;
  *pntr++ = ' '; 
  if (number > MaxAllowedNumber) {    // error on number
    *pntr++ = 'E';
    *pntr++ = 'R';
    *pntr++ = 'R';
    *pntr++ = ' '; 
    *pntr++ = 0x00; 
  } else {                            // valid number
    *pntr++ = number/100   +0x30;
    *pntr++ = number%100/10+0x30;
    *pntr++ = number%100%10+0x30;
    *pntr++ = ' '; 
    *pntr++ = 0x00; 
  }
  return  buf;
}   // end of FbufInt

//****************************************************************************************/
void  Utilities::PrintEmailBuf(char*  buf){
  /*
   * method to parse a buffer <buf> and print. 
   *  - Replace <BR> with NL
   *  - Replace &emsp; with TAB
   */
  char* ptr;
  ptr=buf;
  Serial.print(F("\n\t"));
  while ( *ptr!=0x00 ) {                                      // end of buffer check
    if                                                        // <br> check
      ( *ptr=='<' && ( *(ptr+1)=='b' || *(ptr+1)=='B') && (*(ptr+2)=='r' || *(ptr+2)=='R') && *(ptr+3)=='>' ) {
      Serial.print(F("\n\t"));
      for (uint8_t ii=0;ii<3;ii++) ptr++;                     // repoint while enabling the while() (replace 4 chars by 2)
    } else if                                                 // &emsp; check
      ( *ptr=='&' && *(ptr+1)=='e' && *(ptr+2)=='m' && *(ptr+3)=='s'  && *(ptr+4)=='p'  && *(ptr+5)==';' ) {
      Serial.print(F("\t"));
      for (uint8_t ii=0;ii<5;ii++) ptr++;                     // repoint while enabling the while() (replace 5 chars by 1)
    } else Serial.print(*ptr);
    ptr++;
  }
}     // end of PrintEmailBuf
  
//****************************************************************************************/
char*  Utilities::waitForUserInput(TimePack C, char* buffer, const char* msg){
  /*
   * method to wait until the user press something
   * <C> system clock structure
   * if <buffer> provided, it returns the user input
   * if <msg> provided, it is used to prompt the use
   */
  static const char Mname[] PROGMEM = "Utilities::waitForUserInput:";
  static const char Lwait[] PROGMEM = "Enter text to proceed ";
  static const char LIn[] PROGMEM = "Input stream: ";
  String input;
  if ( msg==nullptr ) InfoStamp(C,Mname,Lwait,1,1);
  else                        InfoStamp(C,Mname,msg,1,1);
  while (1==1 ){                              // wait for user input
    if (Serial.available() > 0) {             // user entered something
          input = Serial.readStringUntil('\n');
          InfoStamp(C,Mname,LIn,1,0); Serial.print(input); Serial.print(" - END\n");
          break;
    } else { delay (100); }
  }   // end wait for user input
  if ( buffer!=nullptr ) {
    input.toCharArray(buffer,input.length()+1);
  }   // end of argument provided
  return buffer;
} // end of waitForUserInput

//****************************************************************************************/
void  Utilities::ESPmemUsage(TimePack C, const char* Mname, const char* msg){
  /*
   * method to print the usage of ESP memory
   */
  static const char L0[] PROGMEM = "Free heap: ";
  static const char L1[] PROGMEM = ". Heap fregmentation: ";
  static const char L2[] PROGMEM = "%. Max Free Block Size: ";
  InfoStamp(C,Mname,L0,1,0); Serial.print(ESP.getFreeHeap()); Serial.print(L1); Serial.print(ESP.getHeapFragmentation()); 
  Serial.print(L2); Serial.print(ESP.getMaxFreeBlockSize()); Serial.print(F(" ")); Serial.print(msg); Serial.print(F(" -END\n"));
} // end of ESPmemUsage

//*********************************************************************************/
void  Utilities::PrintIamAlive(TimePack C,bool activate){
    /*
     * method to ptint I am alive indication
     */
    if ( !activate ) return;                    // function inactive
    static const char Mname[] PROGMEM = "Utilities::PrintIamAlive:";
    static const char L0[] PROGMEM = "One minute edge watchdog.";
    static const char L2[] PROGMEM = "One hour edge watchdog.";
    const char DOT[6] = {'-','+','.','*','#','@'};
    static uint8_t dotter=0;
    dotter++; if (dotter>5) dotter=0;
    Serial.print(DOT[dotter]); Serial.print(F("\r"));
    if (C.clockSec==0) { 
        Serial.println(); InfoStamp(C,Mname,L0,0,1);
        ESPmemUsage(C,Mname,nullptr);
        if ( C.hourEdge ) InfoStamp(C,Mname,L2,0,1);
    }   // end of minute edge
}   // end of PrintIamAlive

// **************************************************************************************** //
char* Utilities::uint16ToChars(uint16_t value, char *buffer, size_t bufferSize) {
  /*
    * method to convert int number into char buffer
    * alternative:
    * #include <cstdio> // For snprintf
    * snprintf(buffer, bufferSize, "%u", value); // Converts the number to a null-terminated string
    */
    uint16_t  _value = value;
    size_t i = 0;
    buffer[0] = 0x00;
    if (bufferSize < 2) return  buffer;                   // Not enough space
    do {
        if (i < bufferSize - 1) {                         // Ensure there's space for null terminator
            buffer[i++] = (_value % 10) + '0';            // Convert digit to char
        }
        _value /= 10;
    } while (_value > 0);
    buffer[i] = 0x00;                                     // Null-terminate
    for (size_t j = 0; j < i / 2; ++j) {                  // Reverse the string
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
    return  buffer;
}   // end of uint16ToChars

//****************************************************************************************/
bool Utilities::String2Bool(char* str){
  /*
   * method to convert string to boolean
   */
  if (strncmp(str, "\"true\"", 4) == 0) return true;
  else return false;
}   // end of String2Bool

// ****************************************************************************************/
const   char* Utilities::getVersion() {
    /*
     * method to return the lib's version
     */
    return  UtilitiesVersion;
}   // end of getVersion

//****************************************************************************************/
