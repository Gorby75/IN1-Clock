#include "Wire.h" 
#include "RTClib.h"  
#define mn 60UL
#include <Eeprom24C32_64.h>
#define EEPROM_ADDRESS  0x50
#include <RCSwitch.h>
#include "PCF8574.h" // Required for PCF8574
RCSwitch mySwitch = RCSwitch();
static Eeprom24C32_64 eeprom(EEPROM_ADDRESS);
RTC_DS1307 RTC;  
PCF8574 expander; 
uint32_t TimeAdjustPeriod = 60*mn; //корректирровка времени на -1 сек за 38 мин
uint32_t TimeCorrection = -1;
int nh1,nh2,nm1,nm2,nh1s,nh2s,nm1s,nm2s,nh0,nm0;
byte izm=0;
#define BOARD_TYPE 0
#define KEY0 A0    // часы
#define KEY1 A1    // часы 
#define KEY2 A2    // минуты
#define KEY3 A3    // минуты
#define DECODER0 9
#define DECODER1 10
#define DECODER2 11
#define DECODER3 12
unsigned long timingsw,timer2,tt_al;
byte zi,trigsw=0,arm1,yar1,numel,arm2=0;
int timer1,ch_al,mn_al;
int god1,mou1,day1,mnh1,cmh1,god1a,mou1a,day1a,mnh1a,cmh1a,ch0,mn0,sec1;
byte digitMask[] = {8, 0, 5, 4, 6, 3, 2, 7, 1, 9}; // маска дешифратора платы in12_turned (цифры нормальные)
byte opts[] = {KEY0, KEY1, KEY2, KEY3};            // порядок индикаторов слева направо
#include "GyverHacks.h"
unsigned long alert433;
volatile byte indiDimm[4];        // величина диммирования (0-24)
volatile byte indiCounter[4];     // счётчик каждого индикатора (0-24)
volatile byte indiDigits[4];      // цифры, которые должны показать индикаторы (0-10)
volatile byte curIndi;            // текущий индикатор (0-3)
unsigned long tt=0;

void setDig(byte digit) {
  digit = digitMask[digit];
  setPin(DECODER3, bitRead(digit, 0));
  setPin(DECODER1, bitRead(digit, 1));
  setPin(DECODER0, bitRead(digit, 2));
  setPin(DECODER2, bitRead(digit, 3));
}

void setup() {
 eeprom.initialize();
  expander.begin(0x38);
  /* Setup some PCF8574 pins for demo */
  expander.pinMode(0, OUTPUT);
  expander.pinMode(1, OUTPUT);
  expander.pinMode(2, OUTPUT);
  expander.pinMode(3, OUTPUT);
  expander.pinMode(4, OUTPUT);
  expander.pinMode(5, OUTPUT);
  expander.pinMode(6, OUTPUT);
  expander.pinMode(7, OUTPUT);
  expander.digitalWrite(0, HIGH); 
expander.digitalWrite(1, HIGH);
expander.digitalWrite(2, HIGH);
expander.digitalWrite(3, HIGH);
expander.digitalWrite(4, HIGH);
expander.digitalWrite(5, HIGH);
expander.digitalWrite(6, HIGH);
expander.digitalWrite(7, HIGH);
  Serial.begin(9600);
  
  Wire.begin();
  RTC.begin();
 
ch_al=eeprom.readByte(1);
mn_al=eeprom.readByte(2);
arm1=eeprom.readByte(3);
yar1=eeprom.readByte(4);
numel=eeprom.readByte(5);



  // настройка пинов на выход
  pinMode(DECODER0, OUTPUT);
  pinMode(DECODER1, OUTPUT);
  pinMode(DECODER2, OUTPUT);
  pinMode(DECODER3, OUTPUT);
  pinMode(KEY0, OUTPUT);
  pinMode(KEY1, OUTPUT);
  pinMode(KEY2, OUTPUT);
  pinMode(KEY3, OUTPUT);
 // pinMode(8, INPUT);
 // pinMode(4, INPUT);
 // pinMode(5, INPUT);
 // pinMode(6, INPUT);
 // pinMode(7, INPUT);
 mySwitch.enableReceive(0);
  


  // перенастраиваем частоту ШИМ на пинах 3 и 11 на 7.8 кГц и разрешаем прерывания по совпадению
  TCCR2B = (TCCR2B & B11111000) | 2;    // делитель 8
  TCCR2A |= (1 << WGM21);   // включить CTC режим для COMPA
  TIMSK2 |= (1 << OCIE2A);  // включить прерывания по совпадению COMPA

  // яркость индикаторов (0 - 24)
  indiDimm[0] = yar1;
  indiDimm[1] = yar1;
  indiDimm[2] = yar1;
  indiDimm[3] = yar1;
}

void(* resetFunc) (void) = 0;

void loop() {
  DateTime now = RTC.now();

  if (mySwitch.available()) {
  
alert433= mySwitch.getReceivedValue();
Serial.println(alert433);
mySwitch.resetAvailable();
if ((alert433>5510000)&& (alert433 < 5513000)) {
god1=int(now.year());
mou1=int(now.month());
day1=int(now.day());
mnh1a=int(now.minute());
cmh1a=int(now.hour());

mnh1=(alert433-5510000)-(((int(alert433-5510000)/100))*100);
cmh1=int((alert433-5510000)/100);
if ((mnh1>60)||(mnh1<0)||(cmh1<0)||(cmh1>24)) goto sss1;
if ((mnh1!=mnh1a)||(cmh1!=cmh1a)) {
  RTC.adjust(DateTime(god1, mou1, day1, cmh1, mnh1, 0));
RTC.begin();
resetFunc();
}
sss1:;
}    
}      


    
  nh1=int(now.hour()/10);
  nh2=now.hour()-(nh1*10);
  nm1=int(now.minute()/10);
  nm2=now.minute()-(nm1*10);
  sec1=now.second();

if (arm1==1)
{
  expander.digitalWrite(0, LOW);
  expander.digitalWrite(1, LOW);
}
if (arm1==0)
{ 
   expander.digitalWrite(0, HIGH);
  expander.digitalWrite(1, HIGH); 
  }
 indiDigits[0] = nh1; 
 indiDigits[1] = nh2;
indiDigits[2] = nm1;
indiDigits[3] = nm2;


if ((arm1==1)&&(int(now.hour())==ch_al)&&(int(now.minute())==mn_al)&&(int(now.second())==0)){

  ////alarm/////
  tt_al=millis();
  expander.digitalWrite(3, LOW);
  arm2=1;
  if (numel>1){
  for (zi = 0; zi <= numel; zi++){
  expander.digitalWrite(4, LOW);
  expander.digitalWrite(4, HIGH); 
  
  }

} 
}  


if (digitalRead(4)==1){
tt=millis();
delay(1000);
yar1=eeprom.readByte(4);
set4:;
nm1=int(yar1/10);
nm2=yar1-(nm1*10);
  indiDimm[0] = yar1;
  indiDimm[1] = yar1;
  indiDimm[2] = yar1;
  indiDimm[3] = yar1;
indiDigits[0] = 0; 
 indiDigits[1] = 0;
indiDigits[2] = nm1;
indiDigits[3] = nm2;

if (digitalRead(3)==1){
delay(500);
tt=millis();
yar1=yar1+1;
if (yar1>24) yar1=24;
if (yar1<2) yar1=2;
tt=millis();
}  
if (digitalRead(4)==1){
delay(500);
tt=millis();
yar1=yar1-1;
if (yar1>24) yar1=24;
if (yar1<2) yar1=2;
tt=millis();
}  
if (digitalRead(6)==1){  
migg();
if (yar1!=eeprom.readByte(4))eeprom.writeByte(4,yar1);
ch_al=eeprom.readByte(1);
mn_al=eeprom.readByte(2);
arm1=eeprom.readByte(3);
goto end3; 
}
if (millis()-tt<5000)goto set4;  

}

if (digitalRead(3)==1){
delay(1000);
tt=millis();
numel=eeprom.readByte(5);
set9:;
indiDigits[0] = 0; 
 indiDigits[1] = 0;
indiDigits[2] = 0;
indiDigits[3] = numel;
if (digitalRead(3)==1){
delay(500);
tt=millis();
numel=numel+1;
if (numel>9) numel=9;
tt=millis();
}  
if (digitalRead(4)==1){
delay(500);
tt=millis();
numel=numel-1;
if (numel<1) numel=1;
tt=millis();
}
if (digitalRead(6)==1){  
migg();
if (numel!=eeprom.readByte(5))eeprom.writeByte(5,numel);
ch_al=eeprom.readByte(1);
mn_al=eeprom.readByte(2);
arm1=eeprom.readByte(3);
goto end3; 
}
if (millis()-tt<5000)goto set9; 
}

end3:;




if (digitalRead(7)==1){
arm2=0;  
expander.digitalWrite(3, HIGH); 
}  
if ((millis()-tt_al>300000)&&(arm2==1)){
tt_al=millis();
  expander.digitalWrite(3, LOW);
  arm2=2;
  if (numel>1){
  for (zi = 0; zi <= numel; zi++){
  expander.digitalWrite(4, LOW);
  expander.digitalWrite(4, HIGH); 
}
  }
}
if ((millis()-tt_al>300000)&&(arm2==2)){
arm2=0;  
expander.digitalWrite(3, HIGH);  
}

//alarmoff  
  

if (digitalRead(5)==1){
izm=0;
expander.digitalWrite(1, LOW);
tt=millis();
set1:;
if (digitalRead(3)==1){
delay(250);
izm=1;
tt=millis();
mn_al=mn_al+5;
if (mn_al>59)
{mn_al=0;
ch_al=ch_al+1;}

if (ch_al>23) ch_al=0;  
  }
if (digitalRead(4)==1){
izm=1;
delay(250);
tt=millis();
mn_al=mn_al-5;
if (mn_al<1)
{
mn_al=50;  
ch_al=ch_al-1;
if (ch_al<0)
ch_al=23;
}
}
nh1=int(ch_al/10);
  nh2=ch_al-(nh1*10);
  nm1=int(mn_al/10);
  nm2=mn_al-(nm1*10);
  
 indiDigits[0] = nh1; 
 indiDigits[1] = nh2;
indiDigits[2] = nm1;
indiDigits[3] = nm2;
if (digitalRead(6)==1){
migg();
if (izm==1)
{
eeprom.writeByte(1,ch_al);
eeprom.writeByte(2,mn_al);
izm=0; 
eeprom.writeByte(3,1);
ch_al=eeprom.readByte(1);
mn_al=eeprom.readByte(2);
arm1=eeprom.readByte(3);
goto end1;
}
if (izm==0){
if (arm1==1) {
  arm1=0;
  eeprom.writeByte(3,arm1);
  ch_al=eeprom.readByte(1);
mn_al=eeprom.readByte(2);
arm1=eeprom.readByte(3);
goto end1;
}
if (arm1==0) {
  arm1=1;
  eeprom.writeByte(3,arm1);
  ch_al=eeprom.readByte(1);
mn_al=eeprom.readByte(2);
arm1=eeprom.readByte(3);
goto end1;
}
}
}
if (millis()-tt<5000)goto set1;  
}

///clock set
if (digitalRead(6)==1){
expander.digitalWrite(0, LOW);
expander.digitalWrite(1, HIGH);
tt=millis();
ch0=int(now.hour());
mn0=int(now.minute());
delay(1000);
set2:;
if (digitalRead(3)==1){
delay(250);
tt=millis();
mn0=mn0+5;
if (mn0>59){
mn0=0;
ch0=ch0+1;
if (ch0>23)
ch0=0;  
  }
}
if (digitalRead(4)==1){
delay(250);
tt=millis();
mn0=mn0-5;
if (mn0<1)
{
mn0=50;  
ch0=ch0-1;
if (ch0<0)
ch0=23;
}
}
nh1=int(ch0/10);
  nh2=ch0-(nh1*10);
  nm1=int(mn0/10);
  nm2=mn0-(nm1*10);
  
 indiDigits[0] = nh1; 
 indiDigits[1] = nh2;
indiDigits[2] = nm1;
indiDigits[3] = nm2;
if (digitalRead(6)==1){
  RTC.adjust(DateTime(2014, 1, 21, ch0, mn0, 0));
  expander.digitalWrite(0, LOW);
  migg();
  goto end1;
}
if (millis()-tt<5000)goto set2;  
}  
end1:;  
}

void migg()
{
expander.digitalWrite(0, LOW);
expander.digitalWrite(1, LOW);
delay(250);
expander.digitalWrite(0, HIGH);
expander.digitalWrite(1, HIGH);
delay(250);
expander.digitalWrite(0, LOW);
expander.digitalWrite(1, LOW);
delay(250);
expander.digitalWrite(0, HIGH);
expander.digitalWrite(1, HIGH);
delay(250);
expander.digitalWrite(0, LOW);
expander.digitalWrite(1, LOW);
delay(250);
expander.digitalWrite(0, HIGH);
expander.digitalWrite(1, HIGH);
delay(250);
}  


// динамическая индикация в прерывании таймера 2
ISR(TIMER2_COMPA_vect) {
  indiCounter[curIndi]++;             // счётчик индикатора
  if (indiCounter[curIndi] == indiDimm[curIndi])  // если достигли порога диммирования
    setPin(opts[curIndi], 0);         // выключить текущий индикатор

  if (indiCounter[curIndi] > 25) {    // достигли порога в 25 единиц
    indiCounter[curIndi] = 0;         // сброс счетчика лампы
    if (++curIndi >= 4) curIndi = 0;  // смена лампы закольцованная

    // отправить цифру из массива indiDigits согласно типу лампы
    byte thisDig = digitMask[indiDigits[curIndi]];
    setPin(DECODER3, bitRead(thisDig, 0));
    setPin(DECODER1, bitRead(thisDig, 1));
    setPin(DECODER0, bitRead(thisDig, 2));
    setPin(DECODER2, bitRead(thisDig, 3));
    setPin(opts[curIndi], 1);         // включить анод на текущую лампу
  }
}
