#include <gmock/gmock.h>

using namespace testing;

int main(int argc, char **argv)
{
    FLAGS_gmock_verbose = "error";

    InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}