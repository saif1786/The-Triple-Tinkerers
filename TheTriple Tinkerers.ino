
#include <Adafruit_GFX.h> // Adafruit graphic library
#include <MCUFRIEND_kbv.h> //Library to use tft screen
MCUFRIEND_kbv tft;

#include <TouchScreen.h>
#define MINPRESSURE 200
#define MAXPRESSURE 1000
#include<SoftwareSerial.h> // Library to use GSM Module by modifying digital pins as Rx and Tx

//Defining Rx and Tx pin for SIM800 module
SoftwareSerial SIM800(52,53);
//Hardware Serial1 pin was used for wifi, hence no need to declare 19TX 18RX

// PIN wiring or tft touch screen and its calibrated values
const int XP=6,XM=A2,YP=A1,YM=7; //240x320 ID=0x1602
const int TS_LEFT=167,TS_RT=860,TS_TOP=912,TS_BOT=179;


TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//Adafruit Touch screen library class used for making buttons
Adafruit_GFX_Button Fan_on_btn, Fan_off_btn, LED_on_btn, LED_off_btn, Pump_on_btn, Pump_off_btn;

int pixel_x, pixel_y;     //Touch_getXY() updates global vars
bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); 
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }
    return pressed;
}

//Defining hexadecimal colour values
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//PIN for connecting relay for lED,Fan and Pump
const int LED = 22;
const int Fan = 24;
const int Pump = 26;
const int wifi_status = 28;
const int wifi_grnd = 29;


//creating variables to store LED state,Fan state,Pump state
String LED_state = "OFF";
String Fan_state = "OFF";
String Pump_state = "OFF";


//Variable to store text message
String textMessage;


//wifi ssid and password
String ssid = "Hotspot";
String pwd = "saifbest";

int L_S = 0;         //variable used for GSM controlling of buttons
int F_S = 0;
int P_S = 0;

String ledwifi ;
String fanwifi;
String pumpwifi;

String l_message;
String f_message;
String p_message;


//Datatypes for showing time
String timestr;
String hh;
String mm;
unsigned long  time;
unsigned long last;
int hhint;
int mmint;

unsigned long wifi_off_elapsed_time = 10000;
unsigned long lastweb;
unsigned long lastcheck;




void setup(void)
{
    //Starting Serial Communication
    Serial.begin(9600);
    SIM800.begin(9600);
    Serial1.begin(115200);
    SIM800.print("AT+CMGF=1\r");
    delay(1000);
    SIM800.print("AT+CNMI=2,2,0,0\r");
    delay(1000);
                   //Starting GSM Functionality
    delay(5000);//connecting dely
    uint16_t ID = tft.readID();
    tft.begin(0x1602);
    Serial.print("Ready");
    //connectwifi();                  //connecting wifi
    //Initialising TFT screen Display module
    tft.setRotation(0);            //PORTRAIT
    tft.fillScreen(BLACK);
    tft.setCursor(20,300);
    tft.setTextSize(2);tft.setTextColor(WHITE);
    tft.println("--:--");
    tft.setCursor(10,40);
    tft.setTextSize(2);
    tft.println("LED");
    LED_on_btn.initButton(&tft,  95, 50, 50, 50, WHITE, CYAN, BLACK, "ON", 2);      //initialising led button on screen
    LED_off_btn.initButton(&tft, 160, 50, 50, 50, WHITE, CYAN, BLACK, "OFF", 2);
    tft.setCursor(10,110);
    tft.println("FAN");
    Fan_on_btn.initButton(&tft, 95, 120, 50, 50, WHITE, CYAN, BLACK, "ON", 2);        // initialising fan button on screen       
    Fan_off_btn.initButton(&tft, 160, 120, 50, 50, WHITE, CYAN, BLACK, "OFF", 2);
    tft.setCursor(10,180);
    tft.println("PUMP");
    Pump_on_btn.initButton(&tft, 95, 190, 50, 50, WHITE, CYAN, BLACK, "ON", 2);         //initialising pump button on screen
    Pump_off_btn.initButton(&tft, 160, 190, 50, 50, WHITE, CYAN, BLACK, "OFF", 2);    
    LED_on_btn.drawButton(false);
    LED_off_btn.drawButton(false);
    Fan_on_btn.drawButton(false);
    Fan_off_btn.drawButton(false);
    Pump_on_btn.drawButton(false);
    Pump_off_btn.drawButton(false);    
    tft.fillCircle(215,50,10,RED);//LED status display
    tft.fillCircle(215,120,10,RED);//Fan status display
    tft.fillCircle(215,190,10,RED);//Pump status display
    tft.fillRect(10,240,220,40,WHITE);//Last User Input text Box
    pinMode(LED,OUTPUT);
    pinMode(Fan,OUTPUT);
    pinMode(Pump,OUTPUT);
    pinMode(wifi_status,OUTPUT);
    pinMode(wifi_grnd,OUTPUT);
    digitalWrite(wifi_grnd,LOW);
    digitalWrite(LED,HIGH);
    digitalWrite(Fan,HIGH);
    digitalWrite(Pump,HIGH);
}


void loop(void)
{  
    
  
    time = millis();          //time updating facility
    if(time-last>=60000){
      timeupdate();
    }

    if(SIM800.available()>0){
      textMessage = SIM800.readString();
      Serial.print(textMessage);// for debugging
      delay(10);
    }

    if(textMessage.indexOf("LED ON")>=0){
      L_S = 1;
      String returnMessage = "LED is turned ON";
      delay(10);
      sendSMS(returnMessage);
      delay(100);
      textMessage = ""; 
    }


    if(textMessage.indexOf("LED OFF")>=0){
      L_S = 2;     
      String returnMessage = "LED is turned OFF";
      delay(10);
      sendSMS(returnMessage);
      delay(100);
      textMessage = ""; 
    }

    if(textMessage.indexOf("FAN ON")>=0){
      F_S = 1;
      String returnMessage = "Fan is turned ON";
      delay(10);
      sendSMS(returnMessage);
      delay(100);
      textMessage = ""; 
    }

    if(textMessage.indexOf("FAN OFF")>=0){
      F_S = 2;
      String returnMessage = "Fan is turned OFF";
      delay(10);
      sendSMS(returnMessage);
      delay(100);
      textMessage = ""; 
    }

    if(textMessage.indexOf("PUMP ON")>=0){
      P_S = 1;
      String returnMessage = "Pump is turned ON";
      delay(10);
      sendSMS(returnMessage);
      delay(100);
      textMessage = ""; 
    }

    if(textMessage.indexOf("PUMP OFF")>=0){      
      P_S = 2;
      String returnMessage = "Pump is turned OFF";
      delay(10);
      sendSMS(returnMessage);
      delay(100);
      textMessage = ""; 
    }                

    
    bool down = Touch_getXY();
    LED_on_btn.press((down && LED_on_btn.contains(pixel_x, pixel_y)) || L_S == 1 || ledwifi == "1");       //led ON conditions
    LED_off_btn.press((down && LED_off_btn.contains(pixel_x, pixel_y)) || L_S == 2 || ledwifi =="0");      //led off conditions
    if (LED_on_btn.justReleased())
        LED_on_btn.drawButton();
    if (LED_off_btn.justReleased())
        LED_off_btn.drawButton();
    if (LED_on_btn.justPressed()) {
        LED_on_btn.drawButton(true);
        delay(100);
        LED_on_btn.drawButton();
        tft.fillCircle(215,50,10, GREEN);
        digitalWrite(LED,LOW);
        refresh();
        tft.setCursor(60,250);
        tft.setTextColor(BLACK);
        tft.println("LED ON");
        L_S = 0;
    }
    if (LED_off_btn.justPressed()) {
        LED_off_btn.drawButton(true);
        delay(100);
        LED_off_btn.drawButton();
        tft.fillCircle(215,50,10, RED);
        digitalWrite(LED,HIGH);
        refresh();
        tft.setCursor(60,250);
        tft.setTextColor(BLACK);
        tft.println("LED OFF");        
        L_S = 0;
    }


    bool down1 = Touch_getXY();
    Fan_on_btn.press((down1 && Fan_on_btn.contains(pixel_x, pixel_y)) || F_S == 1 || fanwifi == "1");       //fan on conditions
    Fan_off_btn.press((down1 && Fan_off_btn.contains(pixel_x, pixel_y)) || F_S == 2 || fanwifi == "0");     //fan off conditions
    if (Fan_on_btn.justReleased())
        Fan_on_btn.drawButton();
    if (Fan_off_btn.justReleased())
        Fan_off_btn.drawButton();
    if (Fan_on_btn.justPressed()) {
        Fan_on_btn.drawButton(true);
        delay(100);
        Fan_on_btn.drawButton();
        tft.fillCircle(215,120,10, GREEN);
        digitalWrite(Fan,LOW);
        refresh();
        tft.setCursor(60,250);
        tft.setTextColor(BLACK);
        tft.println("FAN ON");        
        F_S = 0;
    }
    if (Fan_off_btn.justPressed()) {
        Fan_off_btn.drawButton(true);
        tft.fillCircle(215,120,10, RED);
        delay(100);
        Fan_off_btn.drawButton();
        digitalWrite(Fan,HIGH);
        refresh();
        tft.setCursor(60,250);
        tft.setTextColor(BLACK);
        tft.println("FAN OFF");
        F_S = 0;
    }

 bool down2 = Touch_getXY();
    Pump_on_btn.press((down2 && Pump_on_btn.contains(pixel_x, pixel_y)) || P_S == 1 || pumpwifi == "1");         //pump on conditions
    Pump_off_btn.press((down2 && Pump_off_btn.contains(pixel_x, pixel_y)) || P_S == 2 || pumpwifi == "0");       //pump off conditions
    if (Pump_on_btn.justReleased())
        Pump_on_btn.drawButton();
    if (Pump_off_btn.justReleased())
        Pump_off_btn.drawButton();
    if (Pump_on_btn.justPressed()) {
        Pump_on_btn.drawButton(true);
        delay(100);
        Pump_on_btn.drawButton();
        tft.fillCircle(215,190,10, GREEN);
        digitalWrite(Pump,LOW);
        refresh();
        tft.setCursor(60,250);
        tft.setTextColor(BLACK);
        tft.println("PUMP ON");        
        P_S = 0;
    }
    if (Pump_off_btn.justPressed()) {
        Pump_off_btn.drawButton(true);
        delay(100);
        Pump_off_btn.drawButton();     
        tft.fillCircle(215,190,10, RED);
        digitalWrite(Pump,HIGH);
        refresh();
        tft.setCursor(60,250);
        tft.setTextColor(BLACK);
        tft.println("PUMP OFF");        
        P_S = 0;
    }

    

    if(time-lastcheck>=wifi_off_elapsed_time){                   //Feature of checking wifi connectivity at a fixed intervel 
      lastcheck=time;
      check_wifi();
    }

}

//SMS Sending Function
void sendSMS(String returnMessage){
  // AT command to set SIM800 to SMS mode
  SIM800.print("AT+CMGF=1\r"); 
  delay(1000);
 
  SIM800.println("AT+CMGS=\"8948712143\""); 
  delay(1000);
  // Send the SMS
  SIM800.println(returnMessage); 
  delay(1000);
 
  // End AT command with a ^Z, ASCII code 26
  SIM800.println((char)26); 
  delay(100);
  SIM800.println();
  // Give module time to send SMS
  delay(8000);  
}

//Last User Input Text Box Refresh Function
void refresh(){
    tft.fillRect(10,240,220,40,WHITE);
    delay(100);
}
//function for refreshisng time space
void refreshtime(){
    tft.fillRect(5,290,120,40,BLACK);
    delay(100);
}

void refresh_wifi_tb(){
  tft.fillRect(150,290,110,40,BLACK);
  delay(100);
}
//function for updating time
void timeupdate(){
  last = time;
  String ts = SIM800.readString();          //used to empty previous data on SIM800 serial buffer
  Serial.print(ts);
  
      SIM800.print("AT+CCLK?\r");
      timestr = SIM800.readString();
      Serial.println(timestr);
      if(timestr.indexOf("CCLK:")>0){
        hh = timestr.substring(28,30);
        mm = timestr.substring(31,33);        
        hhint = hh.toInt();
        mmint = mm.toInt();       
   
        if(hhint<12){
          refreshtime();

          tft.setCursor(20,290);
          tft.setTextSize(2);tft.setTextColor(WHITE);
          tft.println(hhint);
          tft.setCursor(50,290);
          tft.setTextColor(WHITE);tft.setTextSize(2);
          tft.println(": "+ mm);
          tft.setCursor(110,290);
          tft.setTextSize(2);tft.setTextColor(WHITE);
          tft.println("am");
        }
        if(hhint>=12){
          refreshtime();
          tft.setCursor(50,290);
          tft.setTextColor(WHITE);tft.setTextSize(2);
          tft.println(": "+ mm);
          tft.setCursor(20,290);
          tft.setTextSize(2);tft.setTextColor(WHITE);
          tft.println(hhint-12);
          tft.setCursor(110,290);
          tft.setTextSize(2);tft.setTextColor(WHITE);
          tft.println("pm");
        }
        if(hhint==12){
          refreshtime();
          tft.setCursor(50,290);
          tft.setTextColor(WHITE);tft.setTextSize(2);
          tft.println(": "+ mm);
          tft.setCursor(20,290);
          tft.setTextSize(2);tft.setTextColor(WHITE);
          tft.println(hhint);
          tft.setCursor(110,290);
          tft.setTextSize(2);tft.setTextColor(WHITE);
          tft.println("pm");
        }
        if(hhint==00){
          refreshtime();

          tft.setCursor(20,290);
          tft.setTextSize(2);tft.setTextColor(WHITE);
          tft.println(hhint+12);
          tft.setCursor(50,290);
          tft.setTextColor(WHITE);tft.setTextSize(2);
          tft.println(": "+ mm);          
          tft.setCursor(110,290);
          tft.setTextSize(2);tft.setTextColor(WHITE);
          tft.println("am");          
        }                
        
      }
}
       
        
//Starting Wifi function
void connectwifi(){
  retry:
  Serial1.print("AT+CWMODE=1\r");
  delay(1000);
  Serial1.print("AT+CWJAP="+ssid+","+pwd+"\r");
  delay(1000);
    
  if(Serial.readString().indexOf("OK")==-1){
    goto retry;
  }
    
  }


//function to check wifi connection status
void check_wifi(){
  Serial1.println("AT+CWJAP?");
  String wifi_rd = Serial1.readString();
  if(wifi_rd.indexOf(ssid)>0){
    digitalWrite(wifi_status,HIGH);
    refresh_wifi_tb();
    tft.setCursor(150,290);tft.setTextSize(1);tft.setTextColor(WHITE);
    tft.println("Connected");
    tft.setCursor(150,305);tft.setTextSize(1);tft.setTextColor(WHITE);
    tft.println("Touch Disabled");
    Serial.println("Connected");
    wifi_off_elapsed_time = 5000;
    web_update();
  }
  else{
    
    digitalWrite(wifi_status,LOW);
    refresh_wifi_tb();
    tft.setCursor(150,290);tft.setTextSize(1);tft.setTextColor(WHITE);
    tft.println("Not Connected");
    tft.setCursor(150,305);tft.setTextSize(1);tft.setTextColor(WHITE);
    tft.println("Touch Enabled");
    Serial.println("Not Connected");
    wifi_off_elapsed_time = 70000;
  }
}



//function to start TCP protocol connection
void start_tcp(){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // ip address of  api.thingspeak.com
  cmd += "\",80";
  Serial1.println(cmd);
}

//function for updating web connected devices status
void web_update(){
  start_tcp();
  delay(2000);
  String tt2=(Serial1.readString()); //this is to remove the excess string from readString buffer
  Serial1.println("AT+CIPSEND=36");
  delay(1000);
  Serial1.println("GET /channels/706943/fields/1/last");
  delay(2000);
  l_message = Serial1.readString();
  ledwifi = l_message.substring(61,62);

  start_tcp();
  delay(2000);
  String tt=(Serial1.readString());
  Serial1.println("AT+CIPSEND=36");
  delay(1000);
  Serial1.println("GET /channels/706945/fields/1/last");
  delay(2000);
  f_message = Serial1.readString();
  fanwifi = f_message.substring(61,62);

  start_tcp();
  delay(2000);
  String tt1=(Serial1.readString());
  Serial1.println("AT+CIPSEND=36");
  delay(1000);
  Serial1.println("GET /channels/706946/fields/1/last");
  delay(2000);
  p_message = Serial1.readString();
  pumpwifi = p_message.substring(61,62);

}
