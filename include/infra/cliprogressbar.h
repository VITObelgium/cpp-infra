#pragma once

#include "infra/cast.h"

#include <memory>
#include <string_view>

namespace inf {

class ProgressBar
{
public:
    ProgressBar(int width);
    ProgressBar(int width, std::string_view fill, std::string_view lead, std::string_view remainder);
    ~ProgressBar() noexcept;

    void display(float progress) noexcept;
    /*! Progress value in the range [0.0-1.0] */
    void set_progress(float progress) noexcept;
    void set_postfix_text(std::string_view text);
    void set_postfix_text(std::string_view text, size_t current, size_t total);

    void done() noexcept;

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

}
