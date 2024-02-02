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
    std::string fill;
    std::string lead;
    std::string remainder;
    std::mutex mutex;
    std::unique_ptr<ProgressBarType> bar;
};

static std::unique_ptr<ProgressBarType> create_progress_bar(int width, std::string_view fill, std::string_view lead, std::string_view remainder)
{
    auto bar = std::make_unique<ProgressBarType>(
        option::BarWidth(width),
        option::Start("["),
        option::End("]"),
        option::FontStyles(std::vector<FontStyle>({FontStyle::bold})));

#ifdef WIN32
    bar->set_option(option::Fill(fill));
    bar->set_option(option::Lead(lead));
    bar->set_option(option::Remainder(remainder));
#else
    (void)fill;
    (void)lead;
    (void)remainder;
#endif

    return bar;
}

ProgressBar::ProgressBar(int width)
: ProgressBar(width, "■", "■", "-")
{
}

ProgressBar::ProgressBar(int width, std::string_view fill, std::string_view lead, std::string_view remainder)
: _pimpl(std::make_unique<Pimpl>())
{
    _pimpl->width     = width;
    _pimpl->fill      = fill;
    _pimpl->lead      = lead;
    _pimpl->remainder = remainder;
    _pimpl->bar       = create_progress_bar(width, fill, lead, remainder);
}

ProgressBar::~ProgressBar() noexcept = default;

void ProgressBar::display(float progress) noexcept
{
    set_progress(progress);
}

void ProgressBar::set_progress(float progress) noexcept
{
    std::scoped_lock lock(_pimpl->mutex);

    if (_pimpl->bar->is_completed() && progress < 1.0) {
        _pimpl->bar = create_progress_bar(_pimpl->width, _pimpl->fill, _pimpl->lead, _pimpl->remainder);
    }

    if (!_pimpl->bar->is_completed()) {
#ifdef WIN32
        static_assert(std::is_same_v<ProgressBarType, indicators::ProgressBar>, "Progressbar type mismatch");
        _pimpl->bar->set_progress(static_cast<size_t>(progress * 100.f));
#else
        static_assert(std::is_same_v<ProgressBarType, indicators::BlockProgressBar>, "Progressbar type mismatch");
        _pimpl->bar->set_progress(progress * 100.f);
#endif
    }

    show_console_cursor(_pimpl->bar->is_completed());
}

void ProgressBar::set_postfix_text(std::string_view text)
{
    std::scoped_lock lock(_pimpl->mutex);
    _pimpl->bar->set_option(option::PostfixText(text));
}

void ProgressBar::set_postfix_text(std::string_view text, size_t current, size_t total)
{
    std::scoped_lock lock(_pimpl->mutex);
    _pimpl->bar->set_option(option::PostfixText(fmt::format("{} {}/{}", text, current, total)));
}

void ProgressBar::done() noexcept
{
    std::scoped_lock lock(_pimpl->mutex);
    if (_pimpl->bar->is_completed()) {
        return;
    }

    if (_pimpl->bar->current() < 100) {
        _pimpl->bar->set_progress(100);
    }
}

}
