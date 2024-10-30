/*
 *  SunData.cpp
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */

#include "Arduino.h"
#include <math.h>
#include "SunData.h"

#define PI 3.14159274101257324219

double GetDayNumber(int year, int month, int day, int hour, int minute)
{
  // http://www.stjarnhimlen.se/comp/ppcomp.html#5
  /*
   * The time scale in these formulae are counted in days. Hours, minutes, seconds are expressed as fractions of a day.
   * Day 0.0 occurs at 2000 Jan 0.0 UT (or 1999 Dec 31, 0:00 UT).
   * This "day number" d is computed as follows (y=year, m=month, D=date, UT=UT in hours+decimals):
   *
   *      d = 367*y - 7 * ( y + (m+9)/12 ) / 4 + 275*m/9 + D - 730530
   *
   * Note that ALL divisions here should be INTEGER divisions.
   * Finally, include the time of the day, by adding:
   *
   *      d = d + UT/24.0        (this is a floating-point division)
   *
   */

  int d = 367 * year - 7 * (year + (month + 9) / 12) / 4 + 275 * month / 9 + day - 730530;
  double hm = hour + (minute / 60.0);
  return d + hm / 24;
}

/// <summary>
/// Revolution function, normalizes an angle to between 0 and 360 degrees by adding or subtracting even multiples of 360.
/// </summary>
double Rev(double x)
{
  return x - floor(x / 360.0) * 360.0;
}

/// <summary>
/// Longitude of the Ascending Node (N)
/// </summary>
double Sun_LongitudeOfAscendingNode()
{
  return 0.0;
}

/// <summary>
/// Inclination to the Ecliptic (i) (plane of the Earth's orbit)
/// </summary>
double Sun_InclinationToEclipticx()
{
  return 0.0;
}

/// <summary>
/// Argument of Perihelion (w)
/// </summary>
double Sun_ArgumentOfPerihelion(double dayNumber)
{
  return 282.9404 + 4.70935e-5 * dayNumber;
}

/// <summary>
/// Semi-major Axis (a) or mean distance from sun, 1.000000 (AU)
/// </summary>
double Sun_SemiMajorAxis()
{
  return 1.0;
}

/// <summary>
/// Eccentricity (e) where 0 = circle, 0-1 = ellipse, and 1 = parabola
/// </summary>
double Sun_Eccentricity(double dayNumber)
{
  return 0.016709 - 1.151e-9 * dayNumber;
}

/// <summary>
/// Mean anomaly (M) (0 at perihelion; increases uniformly with time)
/// </summary>
double Sun_MeanAnomaly(double dayNumber)
{
  return Rev(356.0470 + 0.9856002585 * dayNumber);
}

/// <summary>
/// Ecliptic Obliquity (ecl)
/// </summary>
double EclipticObliquity(double dayNumber)
{
  return 23.4393 - 3.563e-7 * dayNumber;
}

/// <summary>
/// Longitude of Perihelion (w1)
/// </summary>
double Sun_LongitudeOfPerihelion(double dayNumber)
{
  double n = Sun_LongitudeOfAscendingNode();
  double w = Sun_ArgumentOfPerihelion(dayNumber);
  return Rev(n + w);
}

/// <summary>
/// Mean Longitude (L)
/// </summary>
double Sun_MeanLongitude(double dayNumber)
{
  double m = Sun_MeanAnomaly(dayNumber);
  double w1 = Sun_LongitudeOfPerihelion(dayNumber);
  return Rev(m + w1);
}

/// <summary>
/// Perihelion Distance (q)
/// </summary>
double Sun_PerihelionDistance(double dayNumber)
{
  double a = Sun_SemiMajorAxis();
  double e = Sun_Eccentricity(dayNumber);
  return a * (1 - e);
}

/// <summary>
/// Aphelion Distance (Q)
/// </summary>
double Sun_AphelionDistance(double dayNumber)
{
  double a = Sun_SemiMajorAxis();
  double e = Sun_Eccentricity(dayNumber);
  return a * (1 + e);
}

/// <summary>
/// Convert degrees to radians
/// </summary>
double Deg2Rad(double angleDegrees)
{
  return angleDegrees * (PI / 180.0);
}

/// <summary>
/// Convert radians to degrees
/// </summary>
double Rad2Deg(double angleRadians)
{
  return angleRadians * (180.0 / PI);
}



/// <summary>
// Revolution function, normalizes a time value (in hours) to between 0 and 24 by adding or subtracting even multiples of 24.
/// </summary>
double RevTime(double x)
{
  return x - floor(x / 24.0) * 24.0;
}

SunData *GetSunPosition(int year, int month, int day, int hour, int minute, double locationLatitude, double locationLongitude)
{
  double dayNumber = GetDayNumber(year, month, day, hour,  minute);
  double argumentOfPerihelion = Sun_ArgumentOfPerihelion(dayNumber);
  double eclipticObliquity = EclipticObliquity(dayNumber);
  /*
   * First, compute the eccentric anomaly E from the mean anomaly M and from the eccentricity e (degrees):
   *
   *      E = M + e*(180/pi) * sin(M) * ( 1.0 + e * cos(M) )
   *
   * or (if E and M are expressed in radians):
   *
   *      E = M + e * sin(M) * ( 1.0 + e * cos(M) )
   *
   */
  double meanAnomaly = Sun_MeanAnomaly(dayNumber);
  double eccentricity = Sun_Eccentricity(dayNumber);
  double eccentricAnomaly = meanAnomaly + (180 / PI) * eccentricity * sin(Deg2Rad(meanAnomaly)) * (1.0 + eccentricity * cos(Deg2Rad(meanAnomaly)));

  /*
   * Then compute the Sun's distance r and its true anomaly v from:
   *
   *      xv = r * cos(v) = cos(E) - e
   *      yv = r * sin(v) = sqrt(1.0 - e*e) * sin(E)
   *
   *      v = atan2( yv, xv )
   *      r = sqrt( xv*xv + yv*yv )
   *
   * (note that the r computed here is later used as rs)
   */
  double xv = cos(Deg2Rad(eccentricAnomaly)) - eccentricity;
  double yv = sqrt(1.0 - eccentricity * eccentricity) * sin(Deg2Rad(eccentricAnomaly));
  double v = Rad2Deg(atan2(yv, xv));
  double r = sqrt(xv * xv + yv * yv);

  /*
   * Now, compute the Sun's true longitude:
   *
   *      lonsun = v + w
   */
  double sunTrueLongitude = Rev(v + argumentOfPerihelion);

  /*
   * Convert lonsun, r to ecliptic rectangular geocentric coordinates xs,ys:
   *
   *     xs = r * cos(lonsun)
   *     ys = r * sin(lonsun)
   */
  double xs = r * cos(Deg2Rad(sunTrueLongitude));
  double ys = r * sin(Deg2Rad(sunTrueLongitude));

  /*
   * (since the Sun always is in the ecliptic plane, zs is of course zero).
   * xs,ys is the Sun's position in a coordinate system in the plane of the ecliptic.
   * To convert this to equatorial, rectangular, geocentric coordinates, compute:
   *
   *     xe = xs
   *     ye = ys * cos(ecl)
   *     ze = ys * sin(ecl)
   */
  double xe = xs;
  double ye = ys * cos(Deg2Rad(eclipticObliquity));
  double ze = ys * sin(Deg2Rad(eclipticObliquity));

  /*
   * Finally, compute the Sun's Right Ascension (RA) and Declination (Dec):
   *     RA  = atan2( ye, xe )
   *     Dec = atan2( ze, sqrt(xe*xe+ye*ye) )
   */
  double rightAscension = Rad2Deg(atan2(ye, xe));
  double declination = Rad2Deg(atan2(ze, sqrt(xe * xe + ye * ye)));

  /*
   * Calculate Greenwich Sidereal Time, Sidereal Time and the Sun's Hour Angle
   */
  double sunMeanLongitude = Sun_MeanLongitude(dayNumber);
  double gmst0 = sunMeanLongitude / 15 + 12;

  double siderealTime = RevTime(gmst0 + hour + (minute / 60.0) + locationLongitude / 15);
  double hourAngle = RevTime(siderealTime - rightAscension / 15);

  /*
   * Convert the Sun's Hour Angle and Declination to a rectangular coordinate system where the X
   * axis points to the celestial equator in the south, the Y axis to the horizon in the west,
   * and the Z axis to the north celestial pole.
   */
  double x = cos(Deg2Rad(hourAngle * 15)) * cos(Deg2Rad(declination));
  double y = sin(Deg2Rad(hourAngle * 15)) * cos(Deg2Rad(declination));
  double z = sin(Deg2Rad(declination));

  /*
   * Rotate this x,y,z axis system along an axis going east-west (Y axis) in such a way that the
   * Z axis will point to the zenith. At the North Pole, the angle of rotation will be zero since
   * there the north celestial pole already is in the zenith. At other latitudes the angle of
   * rotation becomes 90 - latitude.
   */
  double xhor = x * sin(Deg2Rad(locationLatitude)) - z * cos(Deg2Rad(locationLatitude));
  double yhor = y;
  double zhor = x * cos(Deg2Rad(locationLatitude)) + z * sin(Deg2Rad(locationLatitude));

  /*
   * Compute azimuth and altitude.
   */
  double azimuth = Rad2Deg(atan2(yhor, xhor)) + 180;
  double altitude = Rad2Deg(asin(zhor));

  static struct SunData SunData;
 
  SunData.RightAscension = rightAscension;
  SunData.Declination = declination;
  SunData.Azimuth = azimuth;
  SunData.Altitude = altitude;

  return &SunData;
}
