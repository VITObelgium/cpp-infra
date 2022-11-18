#include "infra/filelock.h"
#include "infra/test/tempdir.h"

#include <doctest/doctest.h>
#include <fstream>

namespace inf {

TEST_CASE("Filelock")
{
    TempDir temp("filelock");

    {
        FileLock lock1(temp.path() / "file.lock");
        FileLock lock2(temp.path() / "file.lock");

        REQUIRE_NOTHROW(lock1.lock());
        CHECK(lock2.try_lock() == false);
        REQUIRE_NOTHROW(lock1.unlock());
        REQUIRE_NOTHROW(lock2.lock());
    }
}

}
