/*
 * program to test Utilities lib
 *
 * V0 08-V-2024
 */

#include  <Clock.h>
TimePack  SysClock;                 // clock data
Clock     RunClock(SysClock);       // clock instance
#include  <Utilities.h>
Utilities RunUtil(SysClock);        // Utilities instance
const int LEDPIN  = D4;             // LED on GPIO2

#define   BAUDRATE        115200
char      daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void setup() {
  Serial.begin(BAUDRATE);     // Serial monitor setup
  delay(500);
  
  Serial.print("\n\n\nProgram started\n"); 

  SysClock.clockHour  = 23;
  SysClock.clockMin   = 30;
  SysClock.clockSec   = 0;
  SysClock.hourEdge   = false;
  SysClock.minEdge    = false;
  SysClock.secEdge    = false;
  SysClock.secFlip    = false;
  SysClock.previousMillis = 0;
  SysClock.clockYear      = 23;
  SysClock.clockMonth     = 2;
  SysClock.clockDay       = 28;
  SysClock.clockWeekDay   = 6;
  SysClock.speed          = 1;
  SysClock = RunClock.begin(SysClock);
  RunUtil.begin(LEDPIN);          
}

void loop() {
  static const char L0[] PROGMEM = "loop:";  
  static const char L1[] PROGMEM = "print message";
  char* EmailBuffer = "This is a message<br>this is the second line<br>";
  char  buffer[6];
  static uint16_t  num;

  SysClock = RunClock.MasterClock(SysClock);
  if ( SysClock.secEdge ) {
    // test Print4FreeHeap
    RunUtil.Print4FreeHeap(SysClock);
    // test ProgMem_Len
    Serial.print("length of <"); Serial.print(L1); Serial.print("> is ");
    Serial.print(RunUtil.ProgMem_Len(L1));
    Serial.print("\n");

    // test Clock
    Serial.print("\n time "); 
    Serial.print(SysClock.clockHour); Serial.print(":");
    Serial.print(SysClock.clockMin); Serial.print(":");
    Serial.print(SysClock.clockSec); Serial.print(" ");
    Serial.print(SysClock.clockDay); Serial.print("/");
    Serial.print(SysClock.clockMonth); Serial.print("/20");
    Serial.print(SysClock.clockYear); Serial.print(" day of week ");
    Serial.print(daysOfTheWeek[SysClock.clockWeekDay]); Serial.print("\n");

    // test InfoStamp, TimestampToString, FbufInt
    num++;
    if(num>999) num=0;
    RunUtil.InfoStamp(SysClock,L0,L1, 1, 1);
    RunUtil.InfoStamp(SysClock,L0,L1, 0, 0); Serial.print(RunUtil.FbufInt(num,buffer)); Serial.print(" -END\n");

    // test LedIndicator
    RunUtil.LedIndicator(LEDPIN,0);
    // test PrintEmailBuf
    RunUtil.PrintEmailBuf(EmailBuffer);
    SysClock.secEdge = false;
  }   // end of sec edge loop
}