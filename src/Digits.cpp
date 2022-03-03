#include "Digits.h"
#include "Digit.h"

Digits::Digits(int number_digits)
{
    m_number_digits = number_digits;
    m_digits = new Digit[number_digits];
}

Digits::~Digits()
{
    delete[] m_digits;
}

int Digits::get_digit_segment(int digit, int segment)
{
    if (digit >= 0 && digit < m_number_digits && segment >= 0 && segment <= 7)
    {
        return m_digits[digit].get_segment(segment);
    }
}
