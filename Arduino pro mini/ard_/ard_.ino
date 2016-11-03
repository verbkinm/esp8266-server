void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    pinMode(13,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13,1);
  Serial.print("ard+");
  digitalWrite(13,0);
  delay(100);
  
  char tmp[4];
  for(int i = 0; i < strlen(tmp); i++)
    tmp[i]= '\0';
  sprintf(tmp, "%d", analogRead(7) );
  if(strlen(tmp) < 4){
    for(int i = strlen(tmp); i < 4; i++)
      tmp[i] = '_';
    tmp[4] = '\0';
  }
 
  Serial.print(tmp);
  delay(1000);
  
}
