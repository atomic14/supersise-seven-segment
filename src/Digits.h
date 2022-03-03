#pragma once

class Digit;

class Digits {
    private:
        Digit *m_digits;
        int m_number_digits;
        public:
        Digits(int number_digits);
        ~Digits();
        int get_digit_segment(int digit, int segment);
};