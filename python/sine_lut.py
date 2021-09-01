import math

sine_lut_h = open("../tasks/Src/sine_lut.c", "w")

preamble = """
/*
 * sine_lut.c
 *
 *  Created on: 24 Jul 2021
 *      Author: betocool
 */

#include "sine_lut.h"

/* Public variables */
const int32_t sine_lut[96000] = {
"""

end = """
};

"""

sine_lut_text = preamble

count = 0

for idx in range(96000):
    sine_lut_text += "{}".format(int(math.sin(2 * math.pi / 96000 * idx) * 0x7FFFFFFF))

    if idx != 95999:
        sine_lut_text += ","

    count += 1

    if count == 16:
        count = 0
        sine_lut_text += '\n'

sine_lut_text += end

sine_lut_h.write(sine_lut_text)
sine_lut_h.close()
