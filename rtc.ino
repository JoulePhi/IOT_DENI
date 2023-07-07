void getNowDate(){
  sprintf(date, "%i/%i/%i",rtc.day(),rtc.month(),rtc.year());
}
void getNowTime(){
  sprintf(time, "%i:%i:%i",rtc.hour(),rtc.minute(),rtc.second());
}
