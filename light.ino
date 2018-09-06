



void onRadiationPulse() {

  makeColor();
  drawDots();
  publishRandomColor();
  publishCpm();
}



void drawDots() {

  for (int i = 0; i < SPOTS; i++) {
    for (int m = 0; m < LEDS_SPOT; m++) {
      leds[i * LEDS_SPOT + m] = CHSV(spots[i].h, spots[i].s, spots[i].v);
    }
  }
  FastLED.show();
}


CHSV makeColor() {
  unsigned long turns = loops / MAX_NUM;
  unsigned long r = loops % MAX_NUM;

  unsigned int p = r / MAX_CCC % SPOTS; // number of the spot
  unsigned int c = r % MAX_CCC;

  byte  h = c  % MAX_H ; // hue
  byte  s = c / MAX_H  % MAX_S; //saturation
  byte  v = c / MAX_H  / MAX_S % MAX_V; //value

  spots[p].h  = map(h, 0, MAX_H, 0, 255);
  spots[p].s = map(s, 0, MAX_S, 128, 255);
  spots[p].v = map(v, 0, MAX_V, 128, 255);


  Serial.print("Seconds=");  Serial.print(millis() / 1000);
  Serial.print(" Count=");   Serial.print(radiationWatch.radiationCount());
  Serial.print(" Loops=");   Serial.print(loops);
  Serial.print(" Turns=");   Serial.print(turns);
  Serial.print(" Random=");  Serial.print(r);
  Serial.println();

  Serial.print("P=");   Serial.print(p);
  Serial.print(" C=");  Serial.print(c);
  Serial.print(" H=");  Serial.print(spots[p].h);
  Serial.print(" S=");  Serial.print(spots[p].s);
  Serial.print(" V=");  Serial.print(spots[p].v);
  Serial.println();


  last_random = r;
  last_spot = p;
}

void showAllLeds(int r, int g, int b ) {
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(r, g, b);
  }
  FastLED.show();
}


