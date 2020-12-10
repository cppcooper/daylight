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

int main(int argc, char *argv[]) {
    double elevation = 0.0;
    double latitude = 1.0;
    double longitude = 1.0;
    int day = get_day_of_year();
    CLI::App app;
    app.add_option("--day,-d", day, "specify day N of solar year");
    app.add_option("--elevation,-e", elevation, "specify the elevation of the observer(metres)");
    app.add_option("--latitude,-l", latitude, "specify the latitude of the observer(degrees)");
    app.add_option("--longitude", longitude, "specify the longitude of the observer(degrees)");
    CLI11_PARSE(app, argc, argv);
    int yesterday = day - 1;
    compute_jdn(day);
    //longitude = 1;
    timed a(day_length(declination_angle(day), elevation, latitude));
    printf("%d:%02d hours of daylight, (day: %d)\n", a.hours, a.minutes, day);
    timed b(day_length(declination_angle(day - 1), elevation, latitude));
    auto c = a - b;
    printf(" delta: %2d:%02d mins\n", c.real < 0 ? -c.minutes : c.minutes, c.seconds);
    timed rise(sunrise(day, declination_angle(day), latitude, elevation));
    printf("sunrise: %d:%02d AM\n", rise.hours, rise.minutes);
    timed set(sunset(day, declination_angle(day), latitude, elevation));
    printf("sunrise: %d:%02d PM\n", set.hours-12, set.minutes);
}

