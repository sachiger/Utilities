/*
 * Utilities.h library for utility methods for IoT applications
 * Created by Sachi Gerlitz
 * 08-IX-2024 ver 1.1 [<ESPmemUsage>]
 * 25-VIII-2024 ver 1 [<InfoStamp> parameters]
 * 22-VII-2024 ver 0.1
 */
#ifndef Utilities_h
  #define Utilities_h

  #include  "Arduino.h"

  class Utilities 
  {
    public:
      Utilities(TimePack T);									// constructor
      void  begin(uint8_t LEDPIN);
      void InfoStamp (TimePack C, const char* StartHeader, const char* MainHeader, bool AckDbg, bool Terminate );
      void  LedIndicator(uint8_t LEDPIN, uint8_t typeState);
      void  Print4FreeHeap(TimePack C);
      uint8_t  ProgMem_Len(const char in_from_flash[]);
      int UtCheckSum(uint8_t  storeBuf[], uint8_t len);
      int UtCheckSum16(uint16_t  storeBuf[], uint8_t len);
      char* TimestampToString(TimePack C, char* buf);
      char* FbufInt(uint16_t number, char* buf);
      void  PrintEmailBuf(char*  buf);
      char*  waitForUserInput(TimePack C, char* buffer=nullptr, const char* msg=nullptr);
      void  ESPmemUsage(TimePack C, const char* Mname, const char* msg);
    private:
      TimePack  _LT;

  };

#endif  //Utilities_h