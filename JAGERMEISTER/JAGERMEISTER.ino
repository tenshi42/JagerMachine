int sensorPin = A0;
int pumpPin = 11;
int fanPin = 12;
int liquidPin = 10;
int temps = 0;  // variable to store the value coming from the sensor
int tempsHistory[10];
long historyPos = 0;
bool historyOk = false;

bool isFirstCooling = true;
bool isCooling = false; // is fan and pump on
bool isCooled = false; // is temperature between -15°c and -14°C

int f = 2;
int e = 3;
int d = 4;
int a = 5;
int g = 6;

int coolingPos = 0;

void setAllLow(){
  digitalWrite(a, LOW);
  digitalWrite(d, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
}

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(pumpPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(liquidPin, OUTPUT);
  Serial.begin(9600);

  pinMode(a, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(f, OUTPUT);
  pinMode(g, OUTPUT);
  setAllLow();
}

void setL(){
  digitalWrite(a, LOW);
  digitalWrite(g, LOW);
  digitalWrite(d, HIGH);
  digitalWrite(e, HIGH);
  digitalWrite(f, HIGH);
}

void setF(){
  digitalWrite(d, LOW);
  digitalWrite(a, HIGH);
  digitalWrite(e, HIGH);
  digitalWrite(f, HIGH);
  digitalWrite(g, HIGH);
}

void setMinusUp(){
  digitalWrite(d, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
  digitalWrite(a, HIGH);
}

void setMinusMid(){
  digitalWrite(a, LOW);
  digitalWrite(d, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, HIGH);
}

void setMinusDown(){
  digitalWrite(a, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
  digitalWrite(d, HIGH);
}

void displayCooling(){
  if(coolingPos == 0){
    setMinusUp();
  }
  else if(coolingPos == 1){
    setMinusMid();
  }
  else if(coolingPos == 2){
    setMinusDown();
  }
  coolingPos++;
  if(coolingPos >= 3)
    coolingPos = 0;
}

void recvControl(){
  if (Serial.available() > 0) {
    char c = (char)Serial.read();

    if(c == '1'){
      digitalWrite(pumpPin, HIGH);
    }
    else if(c == '2'){
      digitalWrite(fanPin, HIGH);
    }
    else{
      digitalWrite(pumpPin, LOW);
      digitalWrite(fanPin, LOW);
    }
  }
}

void control(int temps, int isLiquidOk){
  // real temps to sensor value is : 7.0169x+276.84 where x = real temps
  if(temps > 170 && !isCooling && !isCooled && isLiquidOk == 1){ // -15 °C
    digitalWrite(fanPin, HIGH);
    if(isFirstCooling){ // only if temps >= 0
      // display init
      Serial.println("init...");
      delay(180000); // 180000ms == 180s == 3min : need to cool a little bit before start pump
      isFirstCooling = false;
    }
    else if(!isFirstCooling){
      delay(60000);// 60000ms == 60s == 1min : need to cool a little bit before start pump
    }
    digitalWrite(pumpPin, HIGH);
    isCooling = true;
    isCooled = false;
  }
  else if((temps <= 170 || isLiquidOk == 0) && isCooling && !isCooled){
    digitalWrite(fanPin, LOW);
    digitalWrite(pumpPin, LOW);
    isCooled = true;
  }
  else if((temps >= 177 || isLiquidOk == 0) && isCooled){ // -14°C approximatly
    isCooling = false;
    isCooled = false;
  }
}

int getSmoothValue(int curVal){
  tempsHistory[historyPos] = curVal;
  historyPos++;
  if(historyPos >= 10){
    historyPos = 0;
    historyOk = true;
  }

  if(!historyOk)
    return curVal;
  int meanTemps = 0;
  for(int i = 0 ; i < 10 ; i++)
    meanTemps += tempsHistory[i];
  meanTemps /= 10;
  return meanTemps;
}

void loop() {
  delay(500);
  recvControl();

  int isLiquidOk = digitalRead(liquidPin);
  temps = analogRead(sensorPin);
  int meanTemps = getSmoothValue(temps);

  if(isCooling)
    displayCooling();

  if(isLiquidOk == 0)
    setL();

  if(isLiquidOk == 1 && ((!isCooled && !isCooling) || (isCooling && isCooled)))
    setF();
  
  Serial.print("Raw temps : ");
  Serial.print(temps);
  Serial.print(", Smooth temps : ");
  Serial.print(meanTemps);
  Serial.print(", isLiquidOk : ");
  Serial.print(isLiquidOk);
  Serial.print(", isCooling : ");
  Serial.print(isCooling);
  Serial.print(", isCooled : ");
  Serial.println(isCooled);

  if(historyOk)
    control(meanTemps, isLiquidOk);
}
