#include <stdio.h>
#include "WS2812B.h"
void uint64ToRGBArray(uint64_t value[], uint8_t rgbArray[2][64][3], uint8_t r, uint8_t g,uint8_t b) {
    for(int j =0 ; j < sizeof(value);j++ )
    for (int i = 0; i < 64; i++) {
        // Extract the bit at position i
        uint8_t bit = (value[j] >> (63 - i)) & 1;
        
        // Set RGB values based on the bit
        rgbArray[j][i][0] = bit ? r : 0; // Red
        rgbArray[j][i][1] = bit ? b : 0; // Green
        rgbArray[j][i][2] = bit ? b : 0; // Blue
    }
}

