/*
 * These calculations are scrounged together from wiki articles and other source code
 * I will probably try to find the source material to include at a later date,
 * for now they are lost to the sands of time [read: my browser history]
 */
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <cmath>
#include "/data/dev/libraries/cxx/CLI11.hpp"


struct timed{
    double real;
    int hours;
    int minutes;
    int seconds;
    timed(double time){
        double mins= 0.0;
        real = time;
        if(time < 0){
            hours = (int)-std::ceil(time);
            mins = -60*(time-hours);
        } else {
            hours = std::floor(time);
            mins = 60*(time-hours);
        }
        minutes = (int)mins;
        seconds = (int)(60*(mins-minutes));
    }
    timed operator-(timed &other) const{
        return {real - other.real};
    }
};

int Year(){
    time_t t;
    time(&t);
    tm* ti=localtime(&t);
    return ti->tm_year;
}

int DayOfYear(){
    time_t t;
    time(&t);
    tm* ti=localtime(&t);
    return ti->tm_yday;
}

const double deg2rad = 2*M_PI/360; //conversion
const double rad2deg = 180/M_PI;   //conversion
const double yearLength = 365.24;
const double axialTilt = 23.44*deg2rad; //earth tilt
const double rotPerHour = (24/M_PI); //earth spin
const double rotPerDay = (2*M_PI)/yearLength; //solar angular velocity
const double original_value = 0.0167;
const double hack_value = 0.005109; //tweakin it to get the correct declination value
/*solar declination: the largest errors in this equation are less than ± 0.2°, but are less than ± 0.03° for a given year if the number 10 is adjusted up or down in fractional days as determined by how far the previous year's December solstice occurred before or after noon on December 22.
*/

#define PAPAL

#ifdef PAPAL                    // Pope Gregory XIII's decree
#define LASTJULDATE 15821004L   // last day to use Julian calendar
#define LASTJULJDN  2299160L    // jdn of same
#else                           // British-American usage
#define LASTJULDATE 17520902L   // last day to use Julian calendar
#define LASTJULJDN  2361221L    // jdn of same
#endif

double convert_dynamical_time(double val)
{
    double ut ;
    int jdn ;
    int year, month, day ;
    int hour, minute ;
    bool julian ;
    long x, z, m, d, y ;
    long daysPer400Years = 146097L ;
    long fudgedDaysPer4000Years = 1460970L + 31 ;

    val += 0.5 ;       //  Convert astronomical JDN to chronological

    jdn = (int)floor(val) ;
    ut = val - jdn ;
    julian = (jdn <= LASTJULJDN) ;
    x = jdn + 68569L ;

    if (julian) {
        x += 38 ;
        daysPer400Years = 146100L ;
        fudgedDaysPer4000Years = 1461000L + 1 ;
    }

    z = 4 * x / daysPer400Years ;
    x = x - (daysPer400Years * z + 3) / 4 ;
    y = 4000 * (x + 1) / fudgedDaysPer4000Years ;
    x = x - 1461 * y / 4 + 31 ;
    m = 80 * x / 2447 ;
    d = x - 2447 * m / 80 ;
    x = m / 11 ;
    m = m + 2 - 12 * x ;
    y = 100 * (z - 49) + y + x ;
    year = (int)y ;
    month = (int)m ;
    day = (int)d ;
    if (year <= 0)                   // adjust BC years
        year-- ;

    hour = (int)(ut * 24) ;
    minute = (int)((ut * 24 - hour) * 60) ;  //  Accurate to about 15 minutes c. 2000 CE.

    return 0.0;
}


double winterSolsticeVariance(){ // for the current solar year
    double m;
    m = ((double)Year() - 2000.) / 1000. ;
    double ws_time = 2451900.05952 + 365242.74049 * m - 0.06223 * m * m - 0.00823 * m * m * m + 0.00032 * m * m * m * m ;
    return convert_dynamical_time(ws_time);
}

double declinationAngle(int n){
    //std::cout << "july 28, solar declination 18.66 degrees,\n" << 18.66*deg2rad << " in radians" << std::endl;
    double t0 = winterSolsticeVariance();
    double t1 = rotPerDay*(n+t0+10);
    double t2 = rotPerDay*(n+t0-2);
    double t3 = M_PI*hack_value*sin(t2);
    //std::cout << asin(sin(-axialTilt)*cos(t1+t3)) << " - complex calc" << std::endl;
    //std::cout << axialTilt*sin(2*M_PI*((284+n)/yearLength)) << " - simplest calc" << std::endl;
    return asin(sin(-axialTilt)*cos(t1+t3));
}

double sunrise(double declination, double elevation, double latitude){
    return acos(-tan(deg2rad*latitude)*tan(deg2rad*declination))*rotPerHour;
}

double hoursOfDaylight(double declination, double elevation, double latitude){
    double &d = declination;
    double top = sin(deg2rad*(-0.83-2.076*sqrt(elevation)/60))-(sin(deg2rad*latitude)*sin(d));
    double bot = cos(deg2rad*latitude)*cos(d);
    double w = acos(top/bot);
    return w*rotPerHour;
}

int main(int argc, char *argv[]) {
    CLI::App app;
    int day = DayOfYear();
    double elevation = 0.0;
    double latitude = 1.0;
    app.add_option("--day,-d",day,"specify day N of solar year");
    app.add_option("--elevation,-e",elevation,"specify the elevation of the observer(metres)");
    app.add_option("--latitude,-l",latitude,"specify the latitude of the observer(degrees)");
    CLI11_PARSE(app,argc,argv);
    int yesterday = day - 1;
    timed a(hoursOfDaylight(declinationAngle(day),elevation,latitude));
    std::cout << a.hours << ":" << (a.minutes < 10 ? "0" : "") << a.minutes << " hours on day " << day << std::endl;
    timed b(hoursOfDaylight(declinationAngle(day-1),elevation,latitude));
    auto c = a-b;
    if (c.real < 0)
    std::cout << "delta: -" << c.minutes << ":" << (c.seconds < 10 ? "0" : "") << c.seconds;
    else
    std::cout << "delta: " << c.minutes << ":" << (c.seconds < 10 ? "0" : "") << c.seconds;
    std::cout << " minutes" << std::endl;
    //std::cout << "sunrise: " << sunrise(declinationAngle(day),elevation,latitude) << std::endl;
    //std::cout << hoursOfDaylight(day+1,49.88) << std::endl;
}
