/*
 * These calculations are scrounged together from wiki articles and other source code
 * I will probably try to find the source material to include at a later date,
 * for now they are lost to the sands of time [read: my browser history]
 * https://www.codeguru.com/cpp/cpp/date_time/article.php/c4763/Equinox-and-Solstice-Calculation.htm
 * https://stellafane.org/misc/equinox.html
 * Astronomical Algorithms Second Edition by Jean Meeus, ©1998, published by Willmann-Bell
 */
#include <iostream>
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

bool is_leap_year(int year){
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

int get_cumulative_days(int month, bool isleap = false){
    int days = 0;
    //We are adding up previous months
    //Therefore subtract one
    switch(month-1){
        case 1: //January
            return 31;
        case 2: //February
            days = isleap ? 29 : 28;
            break;
        case 3: //March
            days = 31;
            break;
        case 4: //April
            days = 30;
            break;
        case 5: //May
            days = 31;
            break;
        case 6: //June
            days = 30;
            break;
        case 7: //July
            days = 31;
            break;
        case 8: //August
            days = 31;
            break;
        case 9: //September
            days = 30;
            break;
        case 10: //October
            days = 31;
            break;
        case 11: //November
            days = 30;
            break;
        case 12: //December
            days = 31;
            break;
        default:
            return 0;
    }
    return days + get_cumulative_days(month-1, isleap);
}

const double deg2rad = 2*M_PI/360; //conversion
const double rad2deg = 180/M_PI;   //conversion
const double yearLength = 365.24;
const double axialTilt = -23.44*deg2rad; //earth tilt
const double rotPerHour = (24/M_PI); //earth spin
const double rotPerDay = (2*M_PI)/yearLength; //solar angular velocity
const double original_value = 0.0167;
const double hack_value = 0.005109; //tweakin it to get the correct declination value
/*solar declination: the largest errors in this equation are less than ± 0.2°, but are less than ± 0.03° for a given year if the number 10 is adjusted up or down in fractional days as determined by how far the previous year's December solstice occurred before or after noon on December 22.
*/

#ifdef PAPAL                    // Pope Gregory XIII's decree
#define LASTJULDATE 15821004L   // last day to use Julian calendar
#define LASTJULJDN  2299160L    // jdn of same
#else                           // British-American usage
#define LASTJULDATE 17520902L   // last day to use Julian calendar
#define LASTJULJDN  2361221L    // jdn of same
#endif

double convert_dynamical_time_to_day(double val, bool isleap)
{
    double ut ;
    int jdn ;
    int month, day, hour, minute ;
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
    month = (int)m ;
    day = (int)d ;

    hour = (int)(ut * 24) ;
    minute = (int)((ut * 24 - hour) * 60) ;  //  Accurate to about 15 minutes c. 2000 CE.

    double ret_day = day + (hour/24.f) + (minute / (24.f*60.f));
    return ret_day + get_cumulative_days(month,isleap);
}

double winterSolsticeVariance(){ // for the current solar year
    double m;
    double last_year = Year() - 1;
    m = (last_year - 2000) / 1000;
    // Astronomical Algorithms pg 178
    double last_ws_time = 2451900.05952 + 365242.74049 * m - 0.06223 * pow(m,2) - 0.00823 * pow(m,3) + 0.00032 * pow(m,4);
    return 356 - convert_dynamical_time_to_day(last_ws_time,is_leap_year(last_year));
}

double declinationAngle(int day_of_year){
    //std::cout << "july 28, solar declination 18.66 degrees,\n" << 18.66*deg2rad << " in radians" << std::endl;
    const double eccentricity = 0.0167;
    const double wsv_lastyear = winterSolsticeVariance();
    const double days_from_ws_to_nearyear = 10 + wsv_lastyear; //jan 1 = day 0
    const double days_to_perihelion = 2;
    double t1 = rotPerDay * (day_of_year + days_from_ws_to_nearyear);
    double t2 = (M_PI * eccentricity) * sin(rotPerDay * (day_of_year - 2));
    double declination = -asin(sin(-axialTilt)*cos(t1+t2));
    //std::cout << asin(sin(-axialTilt)*cos(t1+t3)) << " - complex calc" << std::endl;
    //std::cout << axialTilt*sin(2*M_PI*((284+n)/yearLength)) << " - simplest calc" << std::endl;
    //std::cout << declination << std::endl;
    return declination;
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
    double elevation = 0.0;
    double latitude = 1.0;
    int day = DayOfYear();
    CLI::App app;
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
