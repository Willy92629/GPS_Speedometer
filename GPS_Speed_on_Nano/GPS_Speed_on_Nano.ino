#include <TinyGPSPlus.h>
#include <Arduino.h>
#include <TinyGPS++.h>

void serialEvent();
void set7Seg(int number, int pos);
void start();
void lightPeriod();
void lightUp(int num, byte point);
void nextModeISR();
void brightnessISR();

byte displayMode=1; //(mode 1~mode 5)
int displayNum=51;
byte pointDigit;
byte brightness=1;//the lower, the more bright
double kmhr;

const int pinState[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

const int ABCD[7] ={15,19,6,8,9,16,5};
const int digit[4] = {4,18,17,14};

TinyGPSPlus gps;

void setup() {
    for(int i = 0; i < 4; i++) {
        pinMode(digit[i], OUTPUT);
    }
    for(int i = 0; i < 7; i++) {
        pinMode(ABCD[i], OUTPUT);
    }

    pinMode(7, OUTPUT);

    pinMode(2,INPUT_PULLUP);
    pinMode(3,INPUT_PULLUP);

    attachInterrupt(1, brightnessISR, RISING);
    attachInterrupt(0, nextModeISR, RISING);
    start();
    
    Serial.begin(9600);//GPS receiver's baud rate
}

void loop() {
    lightUp(displayNum,pointDigit);
}

void serialEvent(){
  while(Serial.available()){ 
    if(gps.encode(Serial.read())){      
      switch(displayMode){
        
        case 1://UTC time(+8 in Taiwan)
          displayNum=((gps.time.hour()+8)%24)*100+gps.time.minute();
          pointDigit=2;
          break;
          
        case 2:  //speed(kilometer per hour)
          kmhr=gps.speed.kmph();
          displayNum=(int)(kmhr*10);
          pointDigit=1;
          break;
          
        case 3: //heading course degree (0~360 degree)
          displayNum=gps.course.deg();
          pointDigit=0;
          break;
          
        case 4://GPS accuracy(HDOP:0.00~99.99)
          displayNum=gps.hdop.value();
          pointDigit=2;
          break;
       
        case 5://number of connected satellite(0~12)
          displayNum=gps.satellites.value();
          pointDigit=0;    
          break;
      }
    }
  }
}

void start(){
    for(int j=0;j<10;j++){
        for(int i = 0; i < 4; i++) {
            digitalWrite(digit[i], 1);
        }
        for(int i = 0;i < 7;i++) {
            digitalWrite(ABCD[i], pinState[j][i]);
        }
        digitalWrite(digit[0], 0);
        digitalWrite(digit[1], 0);
        digitalWrite(digit[2], 0);
        digitalWrite(digit[3], 0);
        delay(350);
    }
    
}

void set7Seg(int number, int pos, int point) {
    for(int i = 0; i < 4; i++) {
        digitalWrite(digit[i], 1);
    }
    if(point == pos-1){
        digitalWrite(7, 1);
    }else{
        digitalWrite(7, 0);
    }
    for(int i = 0;i < 7;i++) {
        digitalWrite(ABCD[i], pinState[number][i]);
    }
    digitalWrite(digit[pos-1], 0);
   
    //delay(5);
    lightPeriod();
}

void lightPeriod(){
  
  int t=2000;//digit period:2000us

  delayMicroseconds(t-brightness*7);
  for(int i = 0; i < 4; i++) {
        digitalWrite(digit[i], 1);
  }
   for(int i = 0;i < 7;i++) {
        digitalWrite(ABCD[i], 0);
  }
  delayMicroseconds(brightness*7);
}

void lightUp(int num, byte point){
    int arr[4];
    arr[0] = num%10;
    num/=10;
    arr[1] = num%10;
    num/=10;
    arr[2] = num%10;
    num/=10;
    arr[3] = num%10;
    num/=10;

    if(arr[3] == 0){
      set7Seg(arr[2], 3, point); 
      set7Seg(arr[1], 2, point);
      set7Seg(arr[0], 1, point);
    }else{
      set7Seg(arr[3], 4, point); 
      set7Seg(arr[2], 3, point); 
      set7Seg(arr[1], 2, point);
      set7Seg(arr[0], 1, point);
    }
}

void nextModeISR(){
  
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
 
  if (interrupt_time - last_interrupt_time > 150){
    displayMode++;
    if(displayMode==6)displayMode=1;
  }
  last_interrupt_time = interrupt_time;

}

void brightnessISR(){
  
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > 300) {
    brightness-=50;
    if(brightness<=0)brightness=201;
  }
  last_interrupt_time = interrupt_time;
}
  
