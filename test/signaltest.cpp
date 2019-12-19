#include "infra/signal.h"

#include <memory>

#include <doctest/doctest.h>
#include <doctest/trompeloeil.hpp>

namespace inf::test {

using namespace std::placeholders;

using trompeloeil::_;

template <typename ArgType>
class ReceiverMock
{
public:
    MAKE_MOCK0(onItem, void());
    MAKE_MOCK1(onItem1, void(ArgType));
    MAKE_MOCK2(onItem2, void(ArgType, ArgType));
    MAKE_MOCK3(onItem3, void(ArgType, ArgType, ArgType));
};

class SignalTest
{
protected:
    ReceiverMock<const int&> _mock1;
    ReceiverMock<const int&> _mock2;
};

TEST_CASE_FIXTURE(SignalTest, "ConnectDisconnect")
{
    Signal<> sig;

    {
        FORBID_CALL(_mock1, onItem());
        FORBID_CALL(_mock2, onItem());
        sig();
    }

    {
        sig.connect(&_mock1, std::bind(&ReceiverMock<const int&>::onItem, &_mock1));
        sig.connect(&_mock2, std::bind(&ReceiverMock<const int&>::onItem, &_mock2));

        REQUIRE_CALL(_mock1, onItem()).TIMES(1);
        REQUIRE_CALL(_mock2, onItem()).TIMES(1);

        sig();
    }

    {
        FORBID_CALL(_mock1, onItem());
        REQUIRE_CALL(_mock2, onItem()).TIMES(1);

        sig.disconnect(&_mock1);
        sig();
    }

    {
        FORBID_CALL(_mock1, onItem());
        FORBID_CALL(_mock2, onItem());

        sig.disconnect(&_mock2);
        sig();
    }
}

TEST_CASE_FIXTURE(SignalTest, "ConnectDisconnect1")
{
    Signal<const int&> sig;

    {
        FORBID_CALL(_mock1, onItem1(_));
        FORBID_CALL(_mock2, onItem1(_));

        sig(0);
    }

    {
        sig.connect(&_mock1, std::bind(&ReceiverMock<const int&>::onItem1, &_mock1, _1));
        sig.connect(&_mock2, std::bind(&ReceiverMock<const int&>::onItem1, &_mock2, _1));

        REQUIRE_CALL(_mock1, onItem1(1)).TIMES(1);
        REQUIRE_CALL(_mock2, onItem1(1)).TIMES(1);

        sig(1);
    }

    {
        FORBID_CALL(_mock1, onItem1(_));
        REQUIRE_CALL(_mock2, onItem1(2)).TIMES(1);

        sig.disconnect(&_mock1);
        sig(2);
    }

    {
        FORBID_CALL(_mock1, onItem1(_));
        FORBID_CALL(_mock2, onItem1(_));

        sig.disconnect(&_mock2);
        sig(2);
    }
}

TEST_CASE_FIXTURE(SignalTest, "ConnectDisconnect2")
{
    Signal<const int&, const int&> sig;

    {
        FORBID_CALL(_mock1, onItem2(_, _));
        FORBID_CALL(_mock2, onItem2(_, _));

        sig(0, 0);
    }

    {
        sig.connect(&_mock1, std::bind(&ReceiverMock<const int&>::onItem2, &_mock1, _1, _2));
        sig.connect(&_mock2, std::bind(&ReceiverMock<const int&>::onItem2, &_mock2, _1, _2));

        REQUIRE_CALL(_mock1, onItem2(1, 5)).TIMES(1);
        REQUIRE_CALL(_mock2, onItem2(1, 5)).TIMES(1);

        sig(1, 5);
    }

    {
        FORBID_CALL(_mock1, onItem2(_, _));
        REQUIRE_CALL(_mock2, onItem2(2, 7)).TIMES(1);

        sig.disconnect(&_mock1);
        sig(2, 7);
    }

    {
        FORBID_CALL(_mock1, onItem2(_, _));
        FORBID_CALL(_mock2, onItem2(_, _));

        sig.disconnect(&_mock2);

        sig(2, 7);
    }
}

TEST_CASE_FIXTURE(SignalTest, "ConnectDisconnect3")
{
    Signal<const int&, const int&, const int&> sig;

    {
        FORBID_CALL(_mock1, onItem3(_, _, _));
        FORBID_CALL(_mock2, onItem3(_, _, _));

        sig(0, 0, 0);
    }

    {
        sig.connect(&_mock1, std::bind(&ReceiverMock<const int&>::onItem3, &_mock1, _1, _2, _3));
        sig.connect(&_mock2, std::bind(&ReceiverMock<const int&>::onItem3, &_mock2, _1, _2, _3));

        REQUIRE_CALL(_mock1, onItem3(1, 5, 9)).TIMES(1);
        //REQUIRE_CALL(_mock2, onItem3(1, 5, 9)).TIMES(1);
        REQUIRE_CALL(_mock2, onItem3(1, 5, 9)).TIMES(1);

        sig(1, 5, 9);
    }

    {
        FORBID_CALL(_mock1, onItem3(_, _, _));
        REQUIRE_CALL(_mock2, onItem3(2, 7, 0)).TIMES(1);

        sig.disconnect(&_mock1);
        sig(2, 7, 0);
    }

    {
        FORBID_CALL(_mock1, onItem3(_, _, _));
        FORBID_CALL(_mock2, onItem3(_, _, _));

        sig.disconnect(&_mock2);
        sig(2, 7, 0);
    }
}

TEST_CASE_FIXTURE(SignalTest, "ValueArgument")
{
    int32_t integer1 = 0;
    int32_t integer2 = 1;

    ReceiverMock<int32_t> mock;

    Signal<int32_t, int32_t> sig;
    REQUIRE_CALL(mock, onItem2(integer1, integer2)).TIMES(1);

    sig.connect(&mock, std::bind(&ReceiverMock<int32_t>::onItem2, &mock, _1, _2));
    sig(integer1, integer2);
}

TEST_CASE_FIXTURE(SignalTest, "PointerArgument")
{
    int integer = 0;

    ReceiverMock<int32_t*> mock;

    Signal<int32_t*> sig;
    REQUIRE_CALL(mock, onItem1(&integer)).TIMES(1);

    sig.connect(&mock, std::bind(&ReceiverMock<int32_t*>::onItem1, &mock, _1));
    sig(&integer);
}

TEST_CASE_FIXTURE(SignalTest, "NonCopyableArgument")
{
    std::unique_ptr<int32_t> ptr;
    ReceiverMock<std::unique_ptr<int32_t>&> mock;

    Signal<std::unique_ptr<int32_t>&> sig;
    REQUIRE_CALL(mock, onItem1(_)).TIMES(1);

    sig.connect(&mock, std::bind(&ReceiverMock<std::unique_ptr<int32_t>&>::onItem1, &mock, _1));
    sig(ptr);
}
}
