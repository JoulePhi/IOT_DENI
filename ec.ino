float readEc(float temp){
  float voltage,ecValue;
  voltage = analogRead(EC_PIN)/1024.0*5000;   
  ecValue =  ec.readEC(voltage,temp);  
  return ecValue;
}
