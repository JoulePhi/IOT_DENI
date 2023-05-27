String getNowDate(){
  String date = "";
  date += String(rtc.year());
  date += "/";
  date += String(rtc.month());
  date += "/";
  date += String(rtc.day());
  return date;
}
String getNowTime(){
  String time = "";
  time += String(rtc.hour());
  time += ":";
  time += String(rtc.minute());
  time += ":";
  time += String(rtc.second());
  return time;
}
