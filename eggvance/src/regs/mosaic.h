#pragma once

#include "register.h"

class Mosaic : public Register<4>
{
public:
    struct Stretch
    {
        int x;
        int y;
    };

    void reset();

    u8 readByte(int index);
    void writeByte(int index, u8 byte);

    Stretch bgs;
    Stretch obj;
};
