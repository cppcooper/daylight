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
#include "CLI11.hpp"


extern int get_day_of_year();
extern int compute_jdn(int day_of_year);
extern double declination_angle(int day_of_year);
extern double sunrise(int day_of_year, double declination, double latitude, u_int16_t elevation);
extern double sunset(int day_of_year, double declination, double latitude, u_int16_t elevation);
extern double day_length(double declination, double elevation, double latitude);

struct timed{
    double real;
    int hours;
    int minutes;
    int seconds;
    explicit timed(double time){
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
        return timed(real - other.real);
    }
};

extern int get_cumulative_days(int month, bool isleap = false);
extern bool is_leap_year(int year);
extern int get_year();

int main(int argc, char *argv[]) {
    double elevation = 0.0;
    double latitude = 1.0;
    int day = get_day_of_year();

    CLI::App app;
    app.add_option("--day,-d", day, "specify day N of solar year");
    app.add_option("--elevation,-e", elevation, "specify the elevation of the observer(metres)");
    app.add_option("--latitude,-l", latitude, "specify the latitude of the observer(degrees)");
    //app.add_option("--longitude", longitude, "specify the longitude of the observer(degrees)");
    CLI11_PARSE(app, argc, argv);


    if(day > 0 && day < 366 || (is_leap_year(get_year()) && day == 366)){
        timed today(day_length(declination_angle(day), elevation, latitude));
        printf("Day: %d\n daylight: %dh %dm %ds\n", day, today.hours, today.minutes, today.seconds);
        timed yesterday(day_length(declination_angle(day - 1), elevation, latitude));
        auto delta = today - yesterday;

        int whitespace_count = 11 + (today.hours < 10 ? 1 : 2);
        whitespace_count += delta.minutes < 10 ? 1 : 0;
        whitespace_count += today.minutes < 10 ? 0 : 1;
        std::string whitespace;
        std::fill_n(whitespace.begin(),whitespace_count,' ');
        printf(whitespace.c_str());

        if(delta.real < 0){
            printf("-%dm %ds from yesterday\n", delta.minutes, delta.seconds);
        } else {
            printf("+%dm %ds from yesterday\n", delta.minutes, delta.seconds);
        }
        timed rise(sunrise(day, declination_angle(day), latitude, elevation));
        printf(" sunrise: %2d:%02d AM\n", rise.hours, rise.minutes);
        timed set(sunset(day, declination_angle(day), latitude, elevation));
        printf(" sunset: %3d:%02d PM\n", set.hours-12, set.minutes);
    } else {
        std::cerr << "Day is outside of valid range.\n";
        exit(1);
    }
}

