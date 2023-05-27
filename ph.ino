float measure(int samples){
  int measurings=0;

    for (int i = 0; i < samples; i++)
    {
        measurings += analogRead(pHSense);
        delay(10);
    }
    return measurings/samples;
}
float calculatePh(float measurings){
  float adc_resolution = 1024.0;
  float voltage = 5 / adc_resolution * measurings;
  return 7 + ((2.5 - voltage) / 0.18);
}
