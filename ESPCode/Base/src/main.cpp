#include <Arduino.h>
#include <Base.h>

#define led 12

Base base;


void setup()
{
  base.begin();
}

void loop()
{
  base.loop(); //loop required by the class

}
