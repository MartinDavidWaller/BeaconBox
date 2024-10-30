/*
 *  SunData.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

struct SunData
{
  double Azimuth;
  double Altitude;
  double RightAscension;
  double Declination;
};

extern SunData *GetSunPosition(int year, int month, int day, int hour, int minute, double locationLatitude, double locationLongitude);
