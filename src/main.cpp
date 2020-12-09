/*
 * These calculations are scrounged together from wiki articles and other source code
 * I will probably try to find the source material to include at a later date,
 * for now they are lost to the sands of time [read: my browser history]
 * https://www.codeguru.com/cpp/cpp/date_time/article.php/c4763/Equinox-and-Solstice-Calculation.htm
 * https://stellafane.org/misc/equinox.html
 * http://www.jgiesen.de/kepler/eccentricity.html
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
    return 1900 + ti->tm_year;
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
const double rotPerHour = (24/M_PI); //earth spin
const double rotPerDay = (2*M_PI)/yearLength; //solar angular velocity
const double original_value = 0.0167;
const double hack_value = 0.005109; //tweakin it to get the correct declination value
/*solar declination: the largest errors in this equation are less than ± 0.2°, but are less than ± 0.03° for a given year if the number 10 is adjusted up or down in fractional days as determined by how far the previous year's December solstice occurred before or after noon on December 22.
*/

double convert_dynamical_time_to_day(double val, bool isleap);
double winter_solstice_offset(){ // for the current solar year
    double m;
    double last_year = Year() - 1;
    m = (last_year - 2000) / 1000;
    // Astronomical Algorithms pg 178
    double last_ws_time = 2451900.05952 + 365242.74049 * m - 0.06223 * pow(m,2) - 0.00823 * pow(m,3) + 0.00032 * pow(m,4);
    return convert_dynamical_time_to_day(last_ws_time,is_leap_year(last_year)) - 356;
}

double eccentricity(int year);
double declination_angle(int day_of_year){
    const double e = eccentricity(Year()); //eccentricity of earth's orbit
    const double axialTilt = -23.44*deg2rad; //earth's axial tilt. 23.44 is not correct but it is close, couldn't find a calculation
    const double wso = winter_solstice_offset();
    const double dtp = 3.5;//days from new year to perihelion (jan 2-5, we'll take the average 3.5)
    const double dtws = 10 + wso; //days from last year's winter solstice to this year's beginning
                                                                                            // (ie. jan 1 [ie. day 0])
    double t1 = rotPerDay * (day_of_year + dtws); //term 1
    double t2 = (M_PI * e) * sin(rotPerDay * (day_of_year - dtp)); //term 2
    double declination = -asin(sin(-axialTilt)*cos(t1+t2));
    //std::cout << asin(sin(-axialTilt)*cos(t1+t3)) << " - complex calc" << std::endl;
    //std::cout << axialTilt*sin(2*M_PI*((284+n)/yearLength)) << " - simplest calc" << std::endl;
    //std::cout << rad2deg*declination << std::endl;
    return declination;
}

double sunrise(int julian_date, double declination, double latitude, double longitude){
    double n = julian_date - 2451545 + 0.0008; //Number of days since Jan 1st 2000 12:00
    double msn = n - longitude / (deg2rad*360); //Mean Solar Noon
    double sma = deg2rad * fmod(357.5291 + 0.98560028 * msn,360); //Solar Mean Anomaly
    double eotc = 1.9148*sin(sma) + 0.02*sin(2*sma) + 0.0003*sin(3*sma); //Equation Of The Center
    double argument_of_perihelion = 102.9372;
    double el = deg2rad * fmod(sma + eotc + 180 + argument_of_perihelion,360);
    double st = 2451545 + msn + 0.0053 * sin(sma) - 0.0069 * sin(2*el);
    double top = sin(deg2rad*(-0.83))-sin(deg2rad*latitude)*sin(declination);
    double bot = cos(deg2rad*latitude)*cos(declination);
    double w = acos(top/bot);
    return st - w/(deg2rad*360);
    //return acos(-tan(deg2rad*latitude)*tan(deg2rad*declination))*rotPerHour;
}

double day_length(double declination, double elevation, double latitude){
    double &d = declination;
    //probably should add some code to use the correct refraction
    double top = sin(deg2rad*(-0.83-2.076*sqrt(elevation)/60))-(sin(deg2rad*latitude)*sin(d));
    double bot = cos(deg2rad*latitude)*cos(d);
    double w = acos(top/bot);
    return w*rotPerHour;
}

int main(int argc, char *argv[]) {
    double elevation = 0.0;
    double latitude = 1.0;
    double longitude = 1.0;
    int day = DayOfYear();
    CLI::App app;
    app.add_option("--day,-d",day,"specify day N of solar year");
    app.add_option("--elevation,-e",elevation,"specify the elevation of the observer(metres)");
    app.add_option("--latitude,-l",latitude,"specify the latitude of the observer(degrees)");
    app.add_option("--longitude",longitude,"specify the longitude of the observer(degrees)");
    CLI11_PARSE(app,argc,argv);
    int yesterday = day - 1;
    timed a(day_length(declination_angle(day), elevation, latitude));
    std::cout << a.hours << ":" << (a.minutes < 10 ? "0" : "") << a.minutes << " hours on day " << day << std::endl;
    timed b(day_length(declination_angle(day - 1), elevation, latitude));
    std::cout << "sunrise: " << sunrise(1,declination_angle(day), latitude, longitude) << std::endl;
    auto c = a-b;
    if (c.real < 0) {
        std::cout << "delta: -" << c.minutes << ":" << (c.seconds < 10 ? "0" : "") << c.seconds;
    } else {
        std::cout << "delta: " << c.minutes << ":" << (c.seconds < 10 ? "0" : "") << c.seconds;
    }
    std::cout << " minutes" << std::endl;
    //std::cout << "sunrise: " << sunrise(declination_angle(day),elevation,latitude) << std::endl;
    //std::cout << day_length(day+1,49.88) << std::endl;
}


// This programme uses formulae taken from
// Jean Meeus's "Astronomical Algorithms" (1991).
// CalculateDate() function based on formulae originally posted by
// Tom Van Flandern / Washington, DC / metares@well.sf.ca.us
// in the UseNet newsgroup sci.astro.
// Reposted 14 May 1991 in FidoNet C Echo conference by
// Paul Schlyter (Stockholm)
// Minor corrections by
// Raymond Gardner Englewood, Colorado
/// Removed other macros, this is the only one being used
#define LASTJULJDN  2361221L    // jdn of same

double convert_dynamical_time_to_day(double val, bool isleap){
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

// (c) 2006 J. Giesen
//2006, Mar 21
//original javascript source available here: http://www.jgiesen.de/kepler/eccentricity.html
//"According to Meeus (More Mathematical Astronomy Morsels, Chapter 33) Simon et al. published in 1994 an expression for the eccentricity e:"
//e = 0.0167086342-0.0004203654*T-0.0000126734*T^2+0.0000001444*T^3-0.0000000002*T^4+0.0000000003*T^5

double JulD(int YY) {
    double MM=1;
    double DD=1;
    double HR=12;
    double MN=0;
    double SC=0;
    HR = HR + (MN/60) + (SC/3600);
    double GGG = 1;
    if (YY<=1585) GGG=0;
    double JD = -1*floor(7*(floor((MM+9)/12)+YY)/4);
    double S = 1;
    if ((MM-9)<0) S=-1;
    double A = abs(MM-9);
    double J1 = floor(YY + S*floor(A/7));
    J1 = -1*floor((floor(J1/100)+1)*3/4);
    JD = JD + floor(275*MM/9) + DD + (GGG*J1);
    JD = JD + 1721027 + 2*GGG + 367 * YY - 0.5;
    JD = JD + (HR/24);
    return JD;
}

double eccentricity(int Y) {
    double jd=JulD(Y);
    double T=(jd-2451545)/365250.0;
    return 0.0167086342-0.0004203654*T-0.0000126734*pow(T,2)+0.0000001444*pow(T,3)-0.0000000002*pow(T,4)+0.0000000003*pow(T,5);
}
