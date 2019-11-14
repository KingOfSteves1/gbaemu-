#include "oamentry.h"

#include "common/utility.h"
#include "common/macros.h"

static constexpr int sizes[4][4][2] = 
{
    {
        {  8,  8 },
        { 16, 16 },
        { 32, 32 },
        { 64, 64 },
    },
    {
        { 16,  8 },
        { 32,  8 },
        { 32, 16 },
        { 64, 32 },
    },
    {
        {  8, 16 },
        {  8, 32 },
        { 16, 32 },
        { 32, 64 },
    },
    {
        {  0,  0 },
        {  0,  0 },
        {  0,  0 },
        {  0,  0 }
    }
};

void OAMEntry::reset()
{
    *this = {};
}

void OAMEntry::writeHalf(int attr, u16 half)
{
    EGG_ASSERT(attr % 2 == 0 && attr <= 4, "Invalid attribute");

    switch (attr)
    {
    case 0:
        attr0       = half;
        y           = bits< 0, 8>(half);
        affine      = bits< 8, 1>(half);
        double_size = bits< 9, 1>(half);
        disabled    = bits< 9, 1>(half);
        gfx_mode    = bits<10, 2>(half);
        mosaic      = bits<12, 1>(half);
        color_mode  = bits<13, 1>(half);
        shape       = bits<14, 2>(half);
        break;

    case 2:
        attr1     = half;
        x         = bits< 0, 9>(half);
        parameter = bits< 9, 5>(half);
        flip_x    = bits<12, 1>(half);
        flip_y    = bits<13, 1>(half);
        size      = bits<14, 2>(half);
        break;

    case 4:
        attr2        = half;
        tile         = bits< 0, 10>(half);
        priority     = bits<10,  2>(half);
        palette_bank = bits<12,  4>(half);
        break;

    default:
        EGG_UNREACHABLE;
        break;
    }
}

int OAMEntry::width() const
{
    return sizes[shape][size][0];
}

int OAMEntry::height() const
{
    return sizes[shape][size][1];
}

bool OAMEntry::isUninitialized() const
{
    return attr0 == 0 && attr1 == 0 && attr2 == 0;
}
