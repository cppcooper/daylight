# daylight
A CLI app to calculate hours of daylight for a given latitude, local geography not considered.

## Install

```shell
$ git clone https://github.com/cppcooper/daylight.git
$ cd daylight
$ mkdir build
$ cd build
$ cmake .. -G Ninja
$ sudo ninja install
```

## Example
```shell
$ daylight -l 49.88
Day: 100
 daylight: 13h 29m 36s
               +3m 39s from yesterday
 sunrise:  6:17 AM
 sunset:   7:47 PM
```
