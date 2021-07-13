#include "infra/cliprogressbar.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#endif
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <indicators/progress_bar.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace inf {

using namespace indicators;

#ifdef WIN32
using ProgressBarType = indicators::ProgressBar;
#else
using ProgressBarType = indicators::BlockProgressBar;
#endif

struct ProgressBar::Pimpl
{
    int width = 0;
    std::unique_ptr<ProgressBarType> bar;
};

static std::unique_ptr<ProgressBarType> create_progress_bar(int width)
{
    auto bar = std::make_unique<ProgressBarType>(
        option::BarWidth(width),
        option::Start("["),
        option::End("]"),
        option::FontStyles(std::vector<FontStyle>{{FontStyle::bold}}));

    if constexpr (std::is_same_v<ProgressBarType, indicators::ProgressBar>) {
        bar->set_option(option::Fill("■"));
        bar->set_option(option::Lead("■"));
        bar->set_option(option::Remainder("-"));
    }

    return bar;
}

ProgressBar::ProgressBar(int width)
: _pimpl(std::make_unique<Pimpl>())
{
    _pimpl->width = width;
    _pimpl->bar   = create_progress_bar(width);
}

ProgressBar::~ProgressBar() noexcept = default;

void ProgressBar::display(float progress) noexcept
{
    set_progress(progress);
}

void ProgressBar::set_progress(float progress) noexcept
{
    if (_pimpl->bar->is_completed() && progress < 1.0) {
        _pimpl->bar = create_progress_bar(_pimpl->width);
    }

    if (!_pimpl->bar->is_completed()) {
        if constexpr (std::is_same_v<ProgressBarType, indicators::BlockProgressBar>) {
            _pimpl->bar->set_progress(progress * 100.f);
        } else if constexpr (std::is_same_v<ProgressBarType, indicators::ProgressBar>) {
            _pimpl->bar->set_progress(static_cast<size_t>(progress * 100.f));
        }
    }

    show_console_cursor(!_pimpl->bar->is_completed());
}

void ProgressBar::set_postfix_text(std::string_view text)
{
    _pimpl->bar->set_option(option::PostfixText(text));
}

void ProgressBar::set_postfix_text(std::string_view text, size_t current, size_t total)
{
    _pimpl->bar->set_option(option::PostfixText(fmt::format("{} {}/{}", text, current, total)));
}

void ProgressBar::done() noexcept
{
    if (_pimpl->bar->is_completed()) {
        return;
    }

    if (_pimpl->bar->current() < 100) {
        _pimpl->bar->set_progress(100);
    }

    _pimpl->bar->mark_as_completed();
}

}
