/**
 * Connections are fairly simple. As we are implementing software SPI, we have flexible pin options.
 * You can connect data transmission pins to any digital I/O pin. In our case the serial clock(CLK),
 * serial data(DIN), data/command(DC), chip enable(CE) and reset(RST) pins are connected from pin 7
 * all the down to pin 3 on Arduino.
 *
 * But unfortunately, the LCD has 3v communication levels, so we cannot directly connect these pins
 * to the Arduino. We need some protection. This can be done by shifting levels.
 *
 * One of the cheap and easiest way to shift levels is to add resistors inline with each data transmission
 * pin. Just add [10kΩ] resistors between the CLK, DIN, DC, and RST pins and a [1kΩ] resistor between CE.
 *
 * Finally, The backlight(BL) pin is connected to 3.3V via [330Ω] current limiting resistor. You can add
 * a potentiometer or connect this pin to any PWM-capable Arduino pin, if you wish to control its brightness.
 *
 */
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

//================================================================
// Declare LCD object for software SPI
// Define the pin output of the nokia LCD
//
// pin 13 - (CLK) Sserial clock pin for SPI interface.
// pin 12 - (DIN) Serial data pin for SPI interface.
// pin 11 - (DC) Data/Command, pin tells the display whether the data it’s receiving is a command or displayable data.
// pin 10 - (CS)  LCD Chip Enable
// pin  9 - (RST) LCD Reset.
//
// Adafruit_PCD8544( CLK, DIN, DC, CE, RST);
//
#define CLK         13
#define DIN         12
#define DC          11
#define CE          10
#define RST          9

Adafruit_PCD8544 display = Adafruit_PCD8544(CLK, DIN, DC, CE, RST);
#define MAX_WIDTH     84
#define MAX_HEIGHT    48

#define speakerPin  1

/**
 * The Funduino Joystick Shield V1.A
 *
 * int up_button        = 2;
 * int right_button     = 3;
 * int down_button      = 4;
 * int left_button      = 5;
 * int start_button     = 6;
 * int select_button    = 7;
 * int joystick_button  = 8;
 * int joystick_axis_x  = A0;
 * int joystick_axis_y  = A1;
 *
 * int buttons[] = {up_button, down_button, left_button, right_button, start_button, select_button, joystick_button};
 *
 */
//================================================================
// Define the push buttons pin output
//
#define UP          2
#define RIGHT       3
#define DOWN        4
#define LEFT        5
#define PAUSE       6
#define START_BUTTON     6
#define SELECT_BUTTON    7
#define JOYSTICK_BUTTON  8
#define JOYSTICK_AXIS_X  A0
#define JOYSTICK_AXIS_Y  A1


//================================================================
// Direction of the snake that is currenlt moving
//
int  direction;
byte  x[200], y[200];
int  i, slength;
int  tempx=10, tempy=10;
int  xx, yy;
int  xegg, yegg;
unsigned long time=280, beeptime=50;
int score=0;



void setup()   {
  Serial.begin(9600);

  pinMode(UP,INPUT);      digitalWrite(UP,HIGH);
  pinMode(RIGHT,INPUT);   digitalWrite(RIGHT,HIGH);
  pinMode(DOWN,INPUT);    digitalWrite(DOWN,HIGH);
  pinMode(LEFT,INPUT);    digitalWrite(LEFT,HIGH);
  pinMode(PAUSE,INPUT);   digitalWrite(PAUSE,HIGH);

  pinMode(speakerPin,OUTPUT);

  //Initialize Display
  display.begin();

  // you can change the contrast around to adapt the display for the best viewing!
  display.setContrast(57);
  //display.setContrast(70);

  // Clear the buffer.
  display.clearDisplay();

  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor(10,1);    display.print("SNAKE");
  display.setTextSize(1);
  display.setCursor(20,25);   display.print("utopian");
  display.display();
  delay(4000);
  display.clearDisplay();

  slength=8;
  xegg=(display.width())/2;
  yegg=(display.height())/2;
  for(i=0;i<=slength;i++)
  {
    x[i]=25-3*i;
    y[i]=10;
  }

  for(i=0;i<slength;i++)
  {
    display.drawCircle(x[i],y[i],1,BLACK);
  }
  display.display();
  direction = RIGHT;

}

void loop()
{
  movesnake();
}

void movesnake()
{
  boolean flag=false;

  flag = DirectionChanged();

  if(flag==true)
  {
    switch (direction) {
      case UP:    {tempy=y[0]-3;tempx=x[0];} break;
      case RIGHT: {tempx=x[0]+3;tempy=y[0];} break;
      case DOWN:  {tempy=y[0]+3;tempx=x[0];} break;
      case LEFT:  {tempx=x[0]-3;tempy=y[0];} break;
    }
  }

  if(millis()%time==0)
  {
    if(flag==false)
    {
      switch (direction) {
        case UP:    {tempy=y[0]-3;tempx=x[0];} break;
        case RIGHT: {tempx=x[0]+3;tempy=y[0];} break;
        case DOWN:  {tempy=y[0]+3;tempx=x[0];} break;
        case LEFT:  {tempx=x[0]-3;tempy=y[0];} break;
        // default : {
        //   display.clearDisplay();
        //   display.setTextColor(BLACK);
        //   display.setCursor(25,10);
        //   display.setTextSize(1);
        //   display.print("ERROR - 1");
        //   display.display();
        //   do {
        //     delay(200);
        //   } while (digitalRead(PAUSE));
        // } break;
      }
    }
    CheckGameOver();
    checkegg();

    if(tempx <= 0)          {tempx = MAX_WIDTH + tempx;}
    if(tempx >= MAX_WIDTH)  {tempx = tempx - MAX_WIDTH;}

    if(tempy <= 0)          {tempy = MAX_HEIGHT + tempy;}
    if(tempy >= MAX_HEIGHT) {tempy = tempy - MAX_HEIGHT;}

    drawsnake();
  }
}

void CheckGameOver()
{
  uint8_t bh, bl;
  unsigned int high;

  for(i=1;i<slength;i++)
  {
    if(x[i]==x[0] && y[i]==y[0])
    {
      bh=EEPROM.read(1);
      bl=EEPROM.read(0);
      high=(((0xff00+bh)<<8) + bl);
      if(score>high)
      {
        high=score;
        bh=(high >> 8);
        bl=high & 0xff;
        EEPROM.write(1,bh);
        EEPROM.write(0,bl);
      }
      display.clearDisplay();
      display.setTextColor(BLACK);
      display.setTextSize(1);
      display.setCursor(20,12);   display.print("Game Over");
      display.setCursor(15,30);   display.print("Score: ");     display.print(score);
      display.setCursor(15,40);   display.print("High: ");      display.print(high);
      display.display();
      beep(20,5000);
      display.clearDisplay();

      slength=8;
      score=0;
      time=280;
      redraw();
    }
  }
}

void checkegg()
{
  if(x[0]==xegg or x[0]==(xegg+1) or x[0]==(xegg+2) or x[0]==(xegg-1))
  {
    if(y[0]==yegg or y[0]==(yegg+1) or y[0]==(yegg+2) or y[0]==(yegg-1))
    {
      score+=1;
      slength+=1;
      if(time>=90)  {time-=20;}
      display.fillRect(xegg,yegg,3,3,WHITE);
      display.display();
      beep(35,beeptime);
      xegg=random(1,MAX_WIDTH -1);
      yegg=random(1,MAX_HEIGHT-1);
    }
  }
}

boolean DirectionChanged()
{
  int u, r, d, l, p;
  u = r = d = l = p = HIGH;

  for(i = UP; i <= PAUSE; i++) {
    if (digitalRead(i)==LOW) {
      switch(i) {
        case UP: {
          u=LOW;
          if(u==LOW and direction!=DOWN)
          {
            direction = UP;
            //tempx=x[0];
            //tempy=y[0]-3;
            return true;
          }
        } break;

        case RIGHT: {
          r=LOW;
          if(r==LOW and direction!=LEFT)
          {
            direction = RIGHT;
            //tempx=x[0]+3;
            //tempy=y[0];
            return true;
          }
        } break;

        case DOWN: {
          d=LOW;
          if(d==LOW and direction!=UP)
          {
            direction = DOWN;
            //tempx=x[0];
            //tempy=y[0]+3;
            return true;
          }
        } break;

        case LEFT: {
          l=LOW;
          if(l==LOW and direction!=RIGHT)
          {
            direction = LEFT;
            //tempx=x[0]-3;
            //tempy=y[0];
            return true;
          }
        } break;

        case PAUSE: {
          p=LOW;
          if(p==LOW)
          {
            display.clearDisplay();
            display.setTextColor(BLACK);
            display.setCursor(25,10);
            display.setTextSize(1);
            display.print("PAUSED");
            display.display();
            delay(400);
            do {
              delay(100);
            } while (digitalRead(PAUSE));

            display.clearDisplay();
            for(i=5;i>0;i--)
            {
              display.setCursor(25,10);
              display.setTextSize(1);
              display.print("READY!");
              display.setCursor(40,30);
              display.print(i);
              display.display();
              delay(1000);
              display.clearDisplay();
            }
            redraw();
          }
        } break;

      }
      break;
    }
  }

  return false;
}

void drawsnake()
{
  for(i=0;i<=slength;i++)
  {
    xx=x[i];
    yy=y[i];
    x[i]=tempx;
    y[i]=tempy;
    tempx=xx;
    tempy=yy;
  }
  display.fillRect(xegg,yegg,3,3,BLACK);
  display.drawCircle(x[0],y[0],1,BLACK);
  display.drawCircle(x[slength],y[slength],1,WHITE);
  display.display();
}

void redraw()
{
  display.fillRect(xegg,yegg,3,3,BLACK);
  for(i=0;i<slength;i++)
     {
     display.drawCircle(x[i],y[i],1,BLACK);
     }
     display.display();
}

void beep (int freq,long tb)
{
    int x;
    long delayAmount = (long)(500/freq);
    long loopTime = (long)(tb/(delayAmount*2));
    for (x=0;x<loopTime;x++)     // One iteration produces sound for one cycle
    {
        digitalWrite(speakerPin,HIGH);
        delay(delayAmount);
        digitalWrite(speakerPin,LOW);
        delay(delayAmount);
    }
    delay(2);
}

