#pragma once

enum BlendMode
{
    BLD_DISABLED = 0b00,  // Blend disabled
    BLD_ALPHA    = 0b01,  // Blend alpha
    BLD_WHITE    = 0b10,  // Blend with white
    BLD_BLACK    = 0b11   // Blend with black
};

struct BlendControl
{
    struct Layer
    {
        union
        {
            struct
            {
                int bg0;  // BG0 enable
                int bg1;  // BG1 enable
                int bg2;  // BG2 enable
                int bg3;  // BG3 enable
            };
            int bg[4];
        };
        int obj;  // Objects enable
        int bdp;  // Backdrop enable
    };

    int mode;          // Mode (0 = none, 1 = alpha blending, 2 = brightness increase, 3 = brightness decrease)
    Layer upper;  // Upper blend layer
    Layer lower;  // Lower blend layer
};
