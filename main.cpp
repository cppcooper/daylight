#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <cmath>
#include <math.h>
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
      hours = -ceil(time);
      mins = -60*(time-hours);
    } else {
      hours = floor(time);
      mins = 60*(time-hours);
    }
    minutes = mins;
    seconds = 60*(mins-minutes);
  }
  timed operator-(timed &other){
    return timed(real - other.real);
  }
};

int DayOfYear(){
  time_t t;
  time(&t);
  tm * ti=localtime(&t);
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

double winterSolsticeVariance(){ // for the current solar year
  return 0;
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
