#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Change this value to preview poses:
// 0=Stand, 1=Step Back, 2=Step Forward, 3=Sword Attack, 4=Kick
int frame = 2;
int energy = 100;

void drawSword(int hx,int hy,int mode){
  if(mode==0){
    display.drawLine(hx,hy,hx,hy+3,WHITE);
    display.drawLine(hx-2,hy-1,hx+2,hy-1,WHITE);
    display.drawLine(hx,hy-2,hx,hy-12,WHITE);
  }else{
    display.drawLine(hx+2,hy-1,hx+12,hy-5,WHITE);
    display.drawLine(hx+2,hy,hx+13,hy-4,WHITE);
  }
}

void drawCharacter(int x,int y,bool weak){
  int body=y+3;
  int leg=y+12;
  int ground=y+20;

  if(weak){
    display.drawCircle(x+2,y+5,3,WHITE);
    display.drawLine(x+2,y+8,x-2,leg+2,WHITE);
    display.drawLine(x-2,leg+2,x+3,ground,WHITE);
    display.drawLine(x+3,ground,x+8,ground,WHITE);
    display.drawLine(x-2,leg+2,x-5,ground,WHITE);
    int hx=x+8,hy=leg+1;
    display.drawLine(x+2,y+9,hx,hy,WHITE);
    drawSword(hx,hy,0);
    return;
  }

  int px=x;
  if(frame==1) px-=10;
  if(frame==2) px+=10;

  display.drawCircle(px,y,3,WHITE);
  display.drawLine(px,body,px,leg,WHITE);

  int hx,hy;

  switch(frame){
    case 0:
      display.drawLine(px,leg,px-3,ground,WHITE);
      display.drawLine(px,leg,px+3,ground,WHITE);
      display.drawLine(px,body+2,px-4,body+5,WHITE);
      hx=px+5; hy=body+4;
      display.drawLine(px,body+2,hx,hy,WHITE);
      drawSword(hx,hy,0);
      break;

    case 1:
      display.drawLine(px,leg,px-6,ground,WHITE);
      display.drawLine(px,leg,px+1,ground,WHITE);
      display.drawLine(px,body+2,px-4,body+5,WHITE);
      hx=px+5; hy=body+4;
      display.drawLine(px,body+2,hx,hy,WHITE);
      drawSword(hx,hy,0);
      break;

    case 2:
      display.drawLine(px,leg,px-1,ground,WHITE);
      display.drawLine(px,leg,px+6,ground,WHITE);
      display.drawLine(px,body+2,px-4,body+5,WHITE);
      hx=px+5; hy=body+4;
      display.drawLine(px,body+2,hx,hy,WHITE);
      drawSword(hx,hy,0);
      break;

    case 3:
      display.drawLine(px,leg,px-4,ground,WHITE);
      display.drawLine(px,leg,px+5,ground,WHITE);
      display.drawLine(px,body+2,px-4,body+5,WHITE);
      hx=px+8; hy=body+2;
      display.drawLine(px,body+2,hx,hy,WHITE);
      drawSword(hx,hy,1);
      break;

    case 4:
      display.drawLine(px,leg,px-2,ground,WHITE);
      display.drawLine(px,leg,px+9,leg-2,WHITE);
      display.drawLine(px,body+2,px-4,body+5,WHITE);
      hx=px+5; hy=body+4;
      display.drawLine(px,body+2,hx,hy,WHITE);
      drawSword(hx,hy,0);
      break;
  }
}

void setup(){
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
}

void loop(){
  display.clearDisplay();

  bool weak=(energy<=15);

  drawCharacter(64,18,weak);

  display.drawRect(29,48,70,8,WHITE);
  int fill=map(energy,0,100,0,68);
  if(fill>0) display.fillRect(30,49,fill,6,WHITE);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(43,58);
  display.print("ENERGY");

  display.display();
  delay(50);
}
