//
// Created by josh on 12/9/20.
//
#include <iostream>
#include <cmath>
#include <cstdio>
#include <ctime>

const double deg2rad = 2*M_PI/360; //conversion
const double rad2deg = 180/M_PI;   //conversion
const double yearLength = 365.24;
const double rotPerHour = (24/M_PI); //earth spin
const double rotPerDay = (2*M_PI)/yearLength; //solar angular velocity

extern int get_year();
extern int compute_jdn(int day_of_year);
extern double winter_solstice_offset();
extern int get_dst(int day_of_year);
double eccentricity(int year);

double declination_angle(int day_of_year){
    const double e = eccentricity(get_year()); //eccentricity of earth's orbit
    const double axialTilt = -23.44*deg2rad; //earth's axial tilt. 23.44 is not correct but it is close, couldn't find a calculation
    const double wso = winter_solstice_offset();
    const double dtp = 3.5;//days from new year to perihelion (jan 2-5, we'll take the average 3.5)
    const double dtws = 10 + wso; //days from last year's winter solstice to this year's beginning (ie. jan 1 [ie. day 0])
    double t1 = rotPerDay * (day_of_year + dtws); //term 1
    double t2 = (M_PI * e) * sin(rotPerDay * (day_of_year - dtp)); //term 2
    double declination = -asin(sin(-axialTilt)*cos(t1+t2));
    return declination;
}

void calculate_w_st(double &w, double &solar_transit, int day_of_year, double declination, double latitude, u_int16_t elevation = 0){
    double julian_date = compute_jdn(day_of_year);
    double n = julian_date - 2451545 + 0.0008; //Number of days since Jan 1st 2000 12:00
    double msn = n; //Mean Solar Noon / calculations work better without the other term, but not gonna refactor
    double sma = fmod(357.5291 + 0.98560028 * msn,360); //Solar Mean Anomaly
    double eotc = 1.9148*sin(deg2rad*sma) + 0.02*sin(2*deg2rad*sma) + 0.0003*sin(3*deg2rad*sma); //Equation Of The Center
    double argument_of_perihelion = 102.9372;
    double el = fmod(sma + eotc + 180 + argument_of_perihelion,360);
    solar_transit = 2451545 + msn + 0.0053 * sin(deg2rad*sma) - 0.0069 * sin(2*deg2rad*el);
    double elevation_correction = deg2rad * (-2.076*sqrt(elevation)/60);
    double top = sin((deg2rad*(-0.83))+elevation_correction)-sin(deg2rad*latitude)*sin(declination);
    double bot = cos(deg2rad*latitude)*cos(declination);
    w = rad2deg * acos(top/bot);
}

double sunrise(int day_of_year, double declination, double latitude, u_int16_t elevation = 0){
    //https://en.wikipedia.org/wiki/Sunrise_equation#Calculate_sunrise_and_sunset
    double w, st;
    calculate_w_st(w,st,day_of_year,declination,latitude,elevation);
    double jd = 0.5 + st - w/360;
    double hours = 24*(jd - floor(jd)) + get_dst(day_of_year);
    return hours;
}

double sunset(int day_of_year, double declination, double latitude, u_int16_t elevation = 0){
    //https://en.wikipedia.org/wiki/Sunrise_equation#Calculate_sunrise_and_sunset
    double w, st;
    calculate_w_st(w,st,day_of_year,declination,latitude,elevation);
    double jd = 0.5 + st + w/360;
    double hours = 24*(jd - floor(jd)) + get_dst(day_of_year);
    return hours;
}



double day_length(double declination, double elevation, double latitude){
    double &d = declination;
    //probably should add some code to use the correct refraction
    double top = sin(deg2rad*(-0.83-2.076*sqrt(elevation)/60))-(sin(deg2rad*latitude)*sin(d));
    double bot = cos(deg2rad*latitude)*cos(d);
    double w = acos(top/bot);
    return w*rotPerHour;
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
