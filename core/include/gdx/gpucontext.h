#pragma once

#include <fmt/format.h>
#include <boost/compute/core.hpp>

namespace gdx::gpu
{

class Context
{
public:
    static boost::compute::context& instance()
    {
        static auto context = boost::compute::system::default_context();
        return context;
    }
};

}
