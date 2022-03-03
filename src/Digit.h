#pragma once

class Digit {
    private:
        int value = 0;
    public:
    Digit &set(int value);
    int get();
    int get_segment(int segment_index);
};