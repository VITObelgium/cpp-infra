#pragma once

namespace pybind11 {
class module;
}
namespace gdx {

void initIoModule(pybind11::module& ioMod);
}
