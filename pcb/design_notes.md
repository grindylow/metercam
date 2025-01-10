# Power Supply

The ESP32CAM-Module is designed to operate from 5 V DC power.

According to the schematic, e.g. in [1], this is internally converted to
3.3 V with an AMS1117 regulator. At the expense of additional heat
dissipation, this will easily work with > 5 V.

In an alternative design, we could switch 4.5 V (from 3 AAA batteries) and
convert these to 3.3 V ourselves with a very low drop regulator.

We choose a power supply of 4 x AAA batteries. These will give a voltage of
1.6 V x 4 = 6.4 V (see, e.g., [2]).

The ESP32COM datasheet gives maximum power consumption of 310 mA  @ 5 V.

The AMS1117 guarantees a maximum dropout voltage of 1.3 V. For 3.3 V output,
we can safely operate down to 3.3 V + 1.3 V = 4.6 V.

4.6 V translates into a single battery voltage of 4.6 V / 4 = 1.15 V.

According to [2], when discharged at 250 mA, the battery voltage will drop to
1.15 V in about 1.5 hours. This roughly marks the end of usable live time.


## MCU Power Supply

The Microchip PIC16F15213 is rated from 1.8 V to 5.5 V. However, the
"absolute maximum ratings" list a permissible operating voltage of up to
6.5 V, which is higher than our maximum battery voltage of 6.4 V.

Hence, no additional voltage dropping diodes are needed.

Serial communication to the ESP32 should also work work both at "full" and
"empty" battery voltage levels (tbc).


## Power Switch Power Supply

The SIP32510 is rated for a maximum operating voltage of 6.0 V.

We operate outside the specification by 0.4 V during off periods, which should
not be a problem.

EN input high voltage must be >= 1.8 V (for V_IN = 5.0 V) which will be the
case even at the end of the battery life time.

An alternative design could employ a simple MOSFET instead of a power switch.

The MCU's charge pump feature may be beneficial in this case?


 

[1]: https://github.com/SeeedDocument/forum_doc/blob/master/reg/ESP32_CAM_V1.6.pdf

[2]: Duracell AAA: https://docs.rs-online.com/3eb6/A700000006538580.pdf
