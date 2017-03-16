//weather station
//based on http://timeinventorskabinet.org/wiki/doku.php/windclocks/anemometer

//output to lcd

#include <LiquidCrystal.h>
#include <Dns.h>
#include <Ethernet.h>
#include <SPI.h>

//delay between two posts (ms)
#define POST_DELAY 600000
//pin on which anemo is connected, matches interrupt 0 on arduino duo
#define PIN_ANEMO 2
//on vortex inspeed anemo, 1Hz is 2.95451922kts. ! pulse counted in var anemo_cnt
#define SPEED_PER_HZ 2.95451922
//delay for instant speed calculation
#define SPEED_INSTANT_DELAY 5
//define wind vane pin
#define PIN_VANE A0
//define maximum and minimum for measurement on inspeed vane
#define DIR_VANE_MIN 0.05
#define DIR_VANE_MAX 0.95

//define http destination
#define SERVER_URL "sirjeannot.free.fr"
#define SERVER_PORT 80

unsigned long last;
//number of roll per 10 minutes. shoud cap around 177000 for 99.9 constant kts
volatile unsigned int anemo_cnt;
//diff for anemo roll instant over 5sec
unsigned int anemo_cnt_last;

/* LCD RS pin to digital pin 9
 * LCD Enable pin to digital pin 8
 * LCD D4 pin to digital pin 7
 * LCD D5 pin to digital pin 6
 * LCD D6 pin to digital pin 5
 * LCD D7 pin to digital pin 3
 * LCD R/W pin to ground
*/
LiquidCrystal lcd(9, 8, 7, 6, 5, 3);

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 };
byte ip[] = { 192, 168, 20, 5 };
byte subnet[] = { 255, 255, 255, 0 };
byte gateway[] = { 192, 168, 20, 1 };
//sirjeannot.free.fr
byte server[] = { 212, 27, 63, 153 };

EthernetClient client;

//string for posting http data
String posthttp;
//anemo, 0 avg, 1 gust, 2 instant
float anemo[3] = {0,0,0};
//vane, 0 is current direction, 1 is last min gust
float vane[2];
char vanedir[2]="  ";

void setup() {
  Serial.begin(9600);      
  //set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  //init ethernet, first gateway argument is dns
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  // disable SD SPI
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  //set wind and vane pins
  pinMode(PIN_ANEMO, INPUT);
  digitalWrite(PIN_ANEMO, HIGH);
  pinMode(PIN_VANE, INPUT);
  //interrupt 0 in pin 2 for arduino duemilanove
  attachInterrupt(0, roll, FALLING);
  //set one var to 0 
  delay(1000);
}

void loop() {
  last=millis();
  anemo_cnt=0;
  anemo_cnt_last=0;
  anemo[0]=0;
  anemo[1]=0;
  vane[1]=0;
  while(millis()-last<POST_DELAY) {
    getwind();
    getdir();
    updatelcd();
  }
  strbuild();
  postmethod();
}

void roll() {
  anemo_cnt++;
}

void updatelcd() {
  lcd.clear();
  lcd.home();
  lcd.print("A");
  lcd.print(anemo[0],1);
  lcd.print("kts");
  lcd.setCursor(11,0);
  lcd.print(vane[0], 0);
  lcd.setCursor(14,0);
  lcd.print(vanedir);
  lcd.setCursor(0,1);
  lcd.print("I");
  lcd.print(anemo[2],1);
  lcd.setCursor(7,1);
  lcd.print("G");
  lcd.print(anemo[1],0);
  lcd.setCursor(13,1);
  lcd.print("*");

  lcd.setCursor(12,1);
  lcd.print("Safe");
  if (anemo[1]>35)
  {
    lcd.setCursor(12,1);
    lcd.print("Warn");
  }
  if (anemo[1]>50)
  {
    lcd.setCursor(12,1);
    lcd.print("Alrt");
  }
}

void getwind() {
  //instant, for the last 5 sec
  anemo[2]=(anemo_cnt-anemo_cnt_last)*SPEED_PER_HZ/SPEED_INSTANT_DELAY;
  anemo_cnt_last=anemo_cnt;
  //average, since beginning of last 5 minutes
  anemo[0]=anemo_cnt*SPEED_PER_HZ/(millis()-last)*1000;
  //gust
  anemo[1]=max(anemo[1],anemo[2]);   
  delay(SPEED_INSTANT_DELAY*1000); 
}

void getdir() {
  //measure and compute only if gust is as low as possible, i.e. if last gust is less than the previous stored
  //maybe computing an average would be better to get rid of the noise...
  if ( anemo[1] <= vane[1] ) {
    //store the new minimum
    vane[1]=anemo[1];
    //get direction, compensante for the 90/100 pente (range 5% 95%), and remove the 5% shift on the analog scale of 1024
    float dir = (float)analogRead(PIN_VANE) / 0.9 - DIR_VANE_MIN * 1023;
    //move to 360° world
    dir = dir / 1023 * 360;
    //remove artifacts
    if ((dir>360) || (dir<0))
    { dir=0; }
    vane[0]=(int)dir;
    if ((dir>=270)||(dir<90))
    { vanedir[0]='N'; }
    else
    { vanedir[0]='S'; }
    if (dir>180)
    { vanedir[1]='W'; }
    else
    { vanedir[1]='E'; }
  }  
}

void strbuild() {
  posthttp = String("avg=");
  posthttp += anemo[0];
  posthttp += "&gust=";
  posthttp += anemo[1];
  posthttp += "&vane=";
  posthttp += vane[0];
}

void postmethod() {
  if (client.connect(SERVER_URL, SERVER_PORT)) {
    client.println("POST /wind/post.php HTTP/1.1");   
    client.println("Host: <PUT_YOUR_BASE_URL_HERE>");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Length: 18");
    client.println();
    client.println(posthttp);
    client.println();
   } 
   else {
     lcd.setCursor(0,1);
     lcd.print("connection failed");
   }
   if (client.connected()) {
      client.stop();
   }
}

