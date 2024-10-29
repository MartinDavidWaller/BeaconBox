using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ConsoleApp14
{
    public static class SunCalc
    {
        public struct SunData
        {
            public double Azimuth;
            public double Altitude;
            public double RightAscension;
            public double Declination;
        }

        public static SunData GetSunPosition(DateTime utcTime, double locationLatitude, double locationLongitude)
        {
            var dayNumber = GetDayNumber(utcTime);
            var argumentOfPerihelion = Sun_ArgumentOfPerihelion(dayNumber);
            var eclipticObliquity = EclipticObliquity(dayNumber);
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
            var meanAnomaly = Sun_MeanAnomaly(dayNumber);
            var eccentricity = Sun_Eccentricity(dayNumber);
            var eccentricAnomaly = meanAnomaly + (180 / Math.PI) * eccentricity * Math.Sin(Deg2Rad(meanAnomaly)) * (1.0 + eccentricity * Math.Cos(Deg2Rad(meanAnomaly)));

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
            var xv = Math.Cos(Deg2Rad(eccentricAnomaly)) - eccentricity;
            var yv = Math.Sqrt(1.0 - eccentricity * eccentricity) * Math.Sin(Deg2Rad(eccentricAnomaly));
            var v = Rad2Deg(Math.Atan2(yv, xv));
            var r = Math.Sqrt(xv * xv + yv * yv);

            /*
             * Now, compute the Sun's true longitude:
             * 
             *      lonsun = v + w
             */
            var sunTrueLongitude = Rev(v + argumentOfPerihelion);

            /*
             * Convert lonsun, r to ecliptic rectangular geocentric coordinates xs,ys:
             * 
             *     xs = r * cos(lonsun)
             *     ys = r * sin(lonsun)
             */
            var xs = r * Math.Cos(Deg2Rad(sunTrueLongitude));
            var ys = r * Math.Sin(Deg2Rad(sunTrueLongitude));

            /*
             * (since the Sun always is in the ecliptic plane, zs is of course zero).
             * xs,ys is the Sun's position in a coordinate system in the plane of the ecliptic.
             * To convert this to equatorial, rectangular, geocentric coordinates, compute:
             * 
             *     xe = xs
             *     ye = ys * cos(ecl)
             *     ze = ys * sin(ecl)
             */
            var xe = xs;
            var ye = ys * Math.Cos(Deg2Rad(eclipticObliquity));
            var ze = ys * Math.Sin(Deg2Rad(eclipticObliquity));

            /*
             * Finally, compute the Sun's Right Ascension (RA) and Declination (Dec):
             *     RA  = atan2( ye, xe )
             *     Dec = atan2( ze, sqrt(xe*xe+ye*ye) )
             */
            var rightAscension = Rad2Deg(Math.Atan2(ye, xe));
            var declination = Rad2Deg(Math.Atan2(ze, Math.Sqrt(xe * xe + ye * ye)));

            /*
             * Calculate Greenwich Sidereal Time, Sidereal Time and the Sun's Hour Angle
             */
            var sunMeanLongitude = Sun_MeanLongitude(dayNumber);
            var gmst0 = sunMeanLongitude / 15 + 12;

            var siderealTime = RevTime(gmst0 + utcTime.Hour + (utcTime.Minute / 60F) + locationLongitude / 15);
            var hourAngle = RevTime(siderealTime - rightAscension / 15);

            /*
             * Convert the Sun's Hour Angle and Declination to a rectangular coordinate system where the X
             * axis points to the celestial equator in the south, the Y axis to the horizon in the west,
             * and the Z axis to the north celestial pole.
             */
            var x = Math.Cos(Deg2Rad(hourAngle * 15)) * Math.Cos(Deg2Rad(declination));
            var y = Math.Sin(Deg2Rad(hourAngle * 15)) * Math.Cos(Deg2Rad(declination));
            var z = Math.Sin(Deg2Rad(declination));

            /*
             * Rotate this x,y,z axis system along an axis going east-west (Y axis) in such a way that the
             * Z axis will point to the zenith. At the North Pole, the angle of rotation will be zero since
             * there the north celestial pole already is in the zenith. At other latitudes the angle of
             * rotation becomes 90 - latitude.
             */
            var xhor = x * Math.Sin(Deg2Rad(locationLatitude)) - z * Math.Cos(Deg2Rad(locationLatitude));
            var yhor = y;
            var zhor = x * Math.Cos(Deg2Rad(locationLatitude)) + z * Math.Sin(Deg2Rad(locationLatitude));

            /*
             * Compute azimuth and altitude.
             */
            var azimuth = Rad2Deg(Math.Atan2(yhor, xhor)) + 180;
            var altitude = Rad2Deg(Math.Asin(zhor));

            return new SunData
            {
                RightAscension = rightAscension,
                Declination = declination,
                Azimuth = azimuth,
                Altitude = altitude
            };
        }

        private static double GetDayNumber(DateTime dt)
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

            var d = 367 * dt.Year - 7 * (dt.Year + (dt.Month + 9) / 12) / 4 + 275 * dt.Month / 9 + dt.Day - 730530;
            double hm = dt.Hour + (dt.Minute / 60F);
            return d + hm / 24;
        }

        /// <summary>
        /// Longitude of the Ascending Node (N)
        /// </summary>
        private static double Sun_LongitudeOfAscendingNode()
        {
            return 0.0D;
        }

        /// <summary>
        /// Inclination to the Ecliptic (i) (plane of the Earth's orbit)
        /// </summary>
        private static double Sun_InclinationToEclipticx()
        {
            return 0.0D;
        }

        /// <summary>
        /// Argument of Perihelion (w)
        /// </summary>
        private static double Sun_ArgumentOfPerihelion(double dayNumber)
        {
            return 282.9404 + 4.70935e-5 * dayNumber;
        }

        /// <summary>
        /// Semi-major Axis (a) or mean distance from sun, 1.000000 (AU)
        /// </summary>
        private static double Sun_SemiMajorAxis()
        {
            return 1.0D;
        }

        /// <summary>
        /// Eccentricity (e) where 0 = circle, 0-1 = ellipse, and 1 = parabola
        /// </summary>
        private static double Sun_Eccentricity(double dayNumber)
        {
            return 0.016709 - 1.151e-9 * dayNumber;
        }

        /// <summary>
        /// Mean anomaly (M) (0 at perihelion; increases uniformly with time)
        /// </summary>
        private static double Sun_MeanAnomaly(double dayNumber)
        {
            return Rev(356.0470 + 0.9856002585 * dayNumber);
        }

        /// <summary>
        /// Ecliptic Obliquity (ecl)
        /// </summary>
        private static double EclipticObliquity(double dayNumber)
        {
            return 23.4393 - 3.563e-7 * dayNumber;
        }

        /// <summary>
        /// Longitude of Perihelion (w1)
        /// </summary>
        private static double Sun_LongitudeOfPerihelion(double dayNumber)
        {
            var n = Sun_LongitudeOfAscendingNode();
            var w = Sun_ArgumentOfPerihelion(dayNumber);
            return Rev(n + w);
        }

        /// <summary>
        /// Mean Longitude (L)
        /// </summary>
        private static double Sun_MeanLongitude(double dayNumber)
        {
            var m = Sun_MeanAnomaly(dayNumber);
            var w1 = Sun_LongitudeOfPerihelion(dayNumber);
            return Rev(m + w1);
        }

        /// <summary>
        /// Perihelion Distance (q)
        /// </summary>
        private static double Sun_PerihelionDistance(double dayNumber)
        {
            var a = Sun_SemiMajorAxis();
            var e = Sun_Eccentricity(dayNumber);
            return a * (1 - e);
        }

        /// <summary>
        /// Aphelion Distance (Q)
        /// </summary>
        private static double Sun_AphelionDistance(double dayNumber)
        {
            var a = Sun_SemiMajorAxis();
            var e = Sun_Eccentricity(dayNumber);
            return a * (1 + e);
        }

        /// <summary>
        /// Convert degrees to radians
        /// </summary>
        private static double Deg2Rad(double angleDegrees)
        {
            return angleDegrees * (Math.PI / 180.0);
        }

        /// <summary>
        /// Convert radians to degrees
        /// </summary>
        private static double Rad2Deg(double angleRadians)
        {
            return angleRadians * (180.0 / Math.PI);
        }

        /// <summary>
        /// Revolution function, normalizes an angle to between 0 and 360 degrees by adding or subtracting even multiples of 360.
        /// </summary>
        private static double Rev(double x)
        {
            return x - Math.Floor(x / 360.0) * 360.0;
        }

        /// <summary>
        /// Revolution function, normalizes a time value (in hours) to between 0 and 24 by adding or subtracting even multiples of 24.
        /// </summary>
        private static double RevTime(double x)
        {
            return x - Math.Floor(x / 24.0) * 24.0;
        }
    }
}
