using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ConsoleApp14
{
    class Program
    {
        private static int ord(char ch)
        {
            return (int)(ch);
        }

        public static void GridCenter(string grid, out double lat, out double lon)
        {
            if (String.IsNullOrEmpty(grid))
                throw new ArgumentException("Grid argument was empty");

            if (grid.Length == 4)
                grid = grid + "LL"; // {choose middle if only 4-character}
            if (grid.Length != 6)
                throw new ArgumentException("Invalid grid square length");

            double lonmin, londeg, latmin, latdeg;
            grid = grid.ToUpper();

            lonmin = (5.0 * (ord(grid[4]) - ord('A'))) + 2.5;    // center
            londeg = 180.0 - (20.0 * (ord(grid[0]) - ord('A')))  // tens of deg
                        - (2.0 * (ord(grid[2]) - ord('0')));     // two deg
            lon = Math.Abs(londeg - (lonmin / 60.0));
            if (grid[0] <= 'I')
                lon = -lon;

            latdeg = -90.0 + (10.0 * (ord(grid[1]) - ord('A')))  // tens of deg
                    + (ord(grid[3]) - ord('0'));                 // degrees
            latmin = 2.5 * (ord(grid[5]) - ord('A'))             // minutes
                       + 1.25;                                   // for center
            lat = Math.Abs(latdeg + (latmin / 60.0));
            if (grid[1] <= 'I')
                lat = -lat;
        }

        static void Main(string[] args)
        {
            DateTime n = new DateTime(2024, 10, 30, 15, 30, 0);
            SunCalc.SunData xy = SunCalc.GetSunPosition(n, 52.0597, 1.1481);

            Beacon B_4U1UN = new Beacon() { Call = "4U1UN", GridSquare = "FN30as" };
            Beacon B_VE8AT = new Beacon() { Call = "VE8AT", GridSquare = "CP38gh" };
            Beacon B_W6WX = new Beacon() { Call = "W6WX", GridSquare = "CM97bd" };
            Beacon B_KH6RS = new Beacon() { Call = "KH6RS", GridSquare = "BL10ts" };
            Beacon B_ZL6B = new Beacon() { Call = "ZL6B", GridSquare = "RE78tw" };
            Beacon B_VK6RBP = new Beacon() { Call = "VK6RBP", GridSquare = "OF87av" };
            Beacon B_JA2IGY = new Beacon() { Call = "JA2IGY", GridSquare = "PM84jk" };
            Beacon B_RR90 = new Beacon() { Call = "RR90", GridSquare = "NO14kx" };
            Beacon B_VR2B = new Beacon() { Call = "VR2B", GridSquare = "OL72bg" };
            Beacon B_4S7B = new Beacon() { Call = "4S7B", GridSquare = "MJ96wv" };
            Beacon B_ZS6DN = new Beacon() { Call = "ZS6DN", GridSquare = "KG33xi" };
            Beacon B_5Z4B = new Beacon() { Call = "5Z4B", GridSquare = "KI88hr" };
            Beacon B_4X6TU = new Beacon() { Call = "4X6TU", GridSquare = "KM72jb" };
            Beacon B_OH2B = new Beacon() { Call = "OH2B", GridSquare = "KP20eh" };
            Beacon B_CS3B = new Beacon() { Call = "CS3B", GridSquare = "IM12jt" };
            Beacon B_LU4AA = new Beacon() { Call = "LU4AA", GridSquare = "GF05tj" };
            Beacon B_OA4B = new Beacon() { Call = "OA4B", GridSquare = "FH17mw" };
            Beacon B_YV5B = new Beacon() { Call = "YV5B", GridSquare = "FJ69cc" };

            List<Beacon> beacons = new List<Beacon>();
            beacons.Add(B_4U1UN);
            beacons.Add(B_VE8AT);
            beacons.Add(B_W6WX);
            beacons.Add(B_KH6RS);
            beacons.Add(B_ZL6B);
            beacons.Add(B_VK6RBP);
            beacons.Add(B_JA2IGY);
            beacons.Add(B_RR90);
            beacons.Add(B_VR2B);
            beacons.Add(B_4S7B);
            beacons.Add(B_ZS6DN);
            beacons.Add(B_5Z4B);
            beacons.Add(B_4X6TU);
            beacons.Add(B_OH2B);
            beacons.Add(B_CS3B);
            beacons.Add(B_LU4AA);
            beacons.Add(B_OA4B);
            beacons.Add(B_YV5B);

            foreach(Beacon b in beacons)
            {
                GridCenter(b.GridSquare, out double lat, out double lon);
                SunCalc.SunData x = SunCalc.GetSunPosition(DateTime.Now, lat, lon);

                Console.WriteLine(string.Format("{0} {1} {2} {3} {4}", b.Call, x.Altitude, x.Altitude > 0 ? "Day" : "Night", lat, lon));
            }
            

            Console.WriteLine("j");
        }
    }
}
