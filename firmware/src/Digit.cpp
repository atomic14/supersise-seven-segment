#include "Digit.h"

//  aaa
// f   b
// f   b
//  ggg
// e   c
// e   c
//  ddd

static const int digit_mapping[10][7] = {
    // a, b, c, d, e, f, g
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}  // 9
};

Digit &Digit::set(int value)
{
    if (value >= 0 && value <= 10)
    {
        this->value = value;
    }
    return *this;
}

int Digit::get()
{
    return this->value;
}

int Digit::get_segment(int segment_index)
{
    if (segment_index >= 0 && segment_index <= 7)
    {
        return digit_mapping[this->value][segment_index];
    }
    return 0;
}