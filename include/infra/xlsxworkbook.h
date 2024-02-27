#pragma once

#include "infra/exception.h"
#include "infra/log.h"
#include "infra/string.h"

#include <string>
#include <xlsxwriter.h>

namespace inf {
namespace xl {

class WorkBook
{
public:
    WorkBook(const fs::path& path)
    : _ptr(workbook_new(inf::str::from_u8(path.u8string()).c_str()))
    {
        if (!_ptr) {
            throw inf::RuntimeError("Failed to create workbook: {}", path);
        }
    }

    ~WorkBook()
    {
        auto error = workbook_close(_ptr);
        if (error != LXW_NO_ERROR) {
            inf::Log::error("Failed to write excel file: {}", lxw_strerror(error));
        }
    }

    operator lxw_workbook*()
    {
        return _ptr;
    }

    lxw_workbook* _ptr;
};

}
}
