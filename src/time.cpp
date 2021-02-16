//
// Created by josh on 12/9/20.
//
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <cmath>

double convert_dynamical_time_to_day(double val, bool isleap);
int get_year();
int get_month(int day_of_year, bool isleap);
int get_days_for_month(int month, bool isleap);
int get_cumulative_days(int month, bool isleap);

bool is_leap_year(int year){
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

int get_dst(int day_of_year){
    bool isleap = is_leap_year(get_year());
    int month = get_month(day_of_year,isleap);
    int day = day_of_year - get_cumulative_days(month,isleap);
    std::tm time_in = { 0, 0, 0,day, month-1, get_year() - 1900 };
    std::time_t time_temp = std::mktime(&time_in);
    const std::tm * time_out = std::localtime(&time_temp);
    //std::cout << day << ", " << month << ", " << time_out->tm_isdst << std::endl;
    return time_out->tm_isdst;
}

int get_year(){
    time_t t;
    time(&t);
    tm* ti=localtime(&t);
    return 1900 + ti->tm_year;
}

int get_month(int day_of_year, bool isleap = false){
    int month = 1;
    int remainder = day_of_year;
    while(remainder > get_days_for_month(month,isleap)){
        remainder -= get_days_for_month(month++,isleap);
    }
    return month;
}

int get_day_of_year(){
    time_t t;
    time(&t);
    tm* ti=localtime(&t);
    return ti->tm_yday + 1; //jan 1 = day 0
}

double winter_solstice_offset(){ // for the current solar year
    double m;
    double last_year = get_year() - 1;
    m = (last_year - 2000) / 1000;
    // Astronomical Algorithms pg 178
    double last_ws_time = 2451900.05952 + 365242.74049 * m - 0.06223 * pow(m,2) - 0.00823 * pow(m,3) + 0.00032 * pow(m,4);
    return convert_dynamical_time_to_day(last_ws_time,is_leap_year(last_year)) - 356;
}

int get_days_for_month(int month, bool isleap = false){
    switch(month){
        case 1: //January
            return 31;
        case 2: //February
            return isleap ? 29 : 28;
        case 3: //March
            return 31;
        case 4: //April
            return 30;
        case 5: //May
            return 31;
        case 6: //June
            return 30;
        case 7: //July
            return 31;
        case 8: //August
            return 31;
        case 9: //September
            return 30;
        case 10: //October
            return 31;
        case 11: //November
            return 30;
        case 12: //December
            return 31;
        default:
            return 0;
    }
}

int get_cumulative_days(int month, bool isleap = false){
    if(month == 0){
        return 0;
    }
    int days = get_days_for_month(month-1,isleap);
    return days + get_cumulative_days(month-1, isleap);
}

int compute_jdn(int month, int day, int year){
    //source: https://www.programmingassignmenthelper.com/convert-a-date-to-the-julian-day-number-in-c/
    int a, m, y, leap_days;
    a = ((14- month) / 12);
    m = (month- 3) + (12 *a);
    y = year + 4800- a;
    leap_days = (y / 4)-(y / 100) + (y / 400);
    return day + (((153 *m) + 2) / 5) + (365 *y) + leap_days- 32045;
}

int compute_jdn(int day_of_year){
    bool isleap = is_leap_year(get_year());
    int month = get_month(day_of_year,isleap);
    int day = day_of_year - get_cumulative_days(month,isleap);
    return compute_jdn(month,day,get_year());
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
    //modified for our purposes (we just need the fractional day of the year passed)
    double ut ;
    int jdn ;
    long hour, minute ;
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

    hour = (int)(ut * 24) ;
    minute = (int)((ut * 24 - hour) * 60) ;  //  Accurate to about 15 minutes c. 2000 CE.

    double ret_day = d + (hour/24.) + (minute / (24.f*60.));
    return ret_day + get_cumulative_days(m,isleap);
}
