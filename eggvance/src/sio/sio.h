#pragma once

#include "io.h"

class Sio
{
public:
    Sio();

    SioControl siocnt;

    Register<u16> rcnt;
    Register<u16> joycnt;
    Register<u32> joyrecv;
    Register<u32> joytrans;
    Register<u16> joystat;
    Register<u16> siomulti[4];
    Register<u16> siosend;
};

inline Sio sio;
