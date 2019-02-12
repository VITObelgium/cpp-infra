#pragma once

#include <chrono>
#include <iostream>

#include "infra/cast.h"

namespace inf {

class ProgressBar
{
private:
    const unsigned int bar_width;
    const char complete_char                               = '=';
    const char incomplete_char                             = ' ';
    const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

public:
    ProgressBar(unsigned int width, char complete, char incomplete)
    : bar_width{width}, complete_char{complete}, incomplete_char{incomplete}
    {
    }

    ProgressBar(unsigned int width)
    : bar_width{width}
    {
    }

    void display(float progress) const
    {
        unsigned int pos = truncate<unsigned int>(bar_width * progress);

        progress *= 100.f;

        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        auto time_elapsed                         = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);

        std::cout << "[";

        for (unsigned int i = 0; i < bar_width; ++i) {
            if (i < pos)
                std::cout << complete_char;
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << incomplete_char;
        }
        std::cout << "] " << truncate<int>(progress) << "% " << time_elapsed.count() << "s\r";
        std::cout.flush();
    }

    void done() const
    {
        display(1.f);
        std::cout << std::endl;
    }
};

}
