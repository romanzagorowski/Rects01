#include "framework.h"

#include <algorithm>

LONG GetRectWidth(const RECT& rc)
{
    return rc.right - rc.left;
}

LONG GetRectHeight(const RECT& rc)
{
    return rc.bottom - rc.top;
}

void NormalizeRect(RECT& rc)
{
    if(rc.left > rc.right)
        std::swap(rc.left, rc.right);

    if(rc.top > rc.bottom)
        std::swap(rc.top, rc.bottom);
}
