#include "infra/cliprogressbar.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#endif
#include <indicators/progress_bar.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace inf {

using namespace indicators;

ProgressBar::ProgressBar(int width)
: _bar(std::make_unique<indicators::ProgressBar>(
      option::BarWidth(width),
      option::Start{"["},
      option::Fill{"■"},
      option::Lead{"■"},
      option::Remainder{"-"},
      option::End{" ]"}))
{
}

ProgressBar::~ProgressBar() noexcept = default;

void ProgressBar::display(float progress) noexcept
{
    set_progress(progress);
}

void ProgressBar::set_progress(float progress) noexcept
{
    _bar->set_progress(truncate<size_t>(progress * 100.f));
}

void ProgressBar::set_postfix_text(std::string_view text)
{
    _bar->set_option(option::PostfixText(text));
}

void ProgressBar::set_postfix_text(std::string_view text, size_t current, size_t total)
{
    _bar->set_option(option::PostfixText(fmt::format("{} {}/{}", text, current, total)));
}

void ProgressBar::done() noexcept
{
    if (_bar->current() < 100) {
        _bar->set_progress(100);
    }
}

}
