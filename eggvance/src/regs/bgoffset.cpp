#include "bgoffset.h"

#include "common/macros.h"
#include "common/utility.h"

void BGOffset::reset()
{
    *this = {};
}

u8 BGOffset::readByte(int index)
{
    EGG_ASSERT(index <= 1, "Invalid index");
    return 0;
}

void BGOffset::writeByte(int index, u8 byte)
{
    EGG_ASSERT(index <= 1, "Invalid index");
    byteArray(offset)[index] = byte;
    offset &= 0x1FF;
}
