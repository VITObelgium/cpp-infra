#include "infra/signal.h"

#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace infra::test {

using namespace testing;
using namespace std::placeholders;

template <typename ArgType>
class ReceiverMock
{
public:
    MOCK_METHOD0_T(onItem, void());
    MOCK_METHOD1_T(onItem1, void(ArgType));
    MOCK_METHOD2_T(onItem2, void(ArgType, ArgType));
    MOCK_METHOD3_T(onItem3, void(ArgType, ArgType, ArgType));
};

class SignalTest : public Test
{
protected:
    ReceiverMock<const int&> m_Mock1;
    ReceiverMock<const int&> m_Mock2;
};

TEST_F(SignalTest, ConnectDisconnect)
{
    Signal<> sig;
    EXPECT_CALL(m_Mock1, onItem()).Times(0);
    EXPECT_CALL(m_Mock2, onItem()).Times(0);

    sig();
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    sig.connect(&m_Mock1, std::bind(&ReceiverMock<const int&>::onItem, &m_Mock1));
    sig.connect(&m_Mock2, std::bind(&ReceiverMock<const int&>::onItem, &m_Mock2));

    EXPECT_CALL(m_Mock1, onItem()).Times(1);
    EXPECT_CALL(m_Mock2, onItem()).Times(1);

    sig();
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    EXPECT_CALL(m_Mock1, onItem()).Times(0);
    EXPECT_CALL(m_Mock2, onItem()).Times(1);

    sig.disconnect(&m_Mock1);
    sig();
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    EXPECT_CALL(m_Mock1, onItem()).Times(0);
    EXPECT_CALL(m_Mock2, onItem()).Times(0);

    sig.disconnect(&m_Mock2);

    sig();
}

TEST_F(SignalTest, ConnectDisconnect1)
{
    Signal<const int&> sig;
    EXPECT_CALL(m_Mock1, onItem1(_)).Times(0);
    EXPECT_CALL(m_Mock2, onItem1(_)).Times(0);

    sig(0);
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    sig.connect(&m_Mock1, std::bind(&ReceiverMock<const int&>::onItem1, &m_Mock1, _1));
    sig.connect(&m_Mock2, std::bind(&ReceiverMock<const int&>::onItem1, &m_Mock2, _1));

    EXPECT_CALL(m_Mock1, onItem1(1)).Times(1);
    EXPECT_CALL(m_Mock2, onItem1(1)).Times(1);

    sig(1);
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    EXPECT_CALL(m_Mock1, onItem1(_)).Times(0);
    EXPECT_CALL(m_Mock2, onItem1(2)).Times(1);

    sig.disconnect(&m_Mock1);
    sig(2);
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    EXPECT_CALL(m_Mock1, onItem1(_)).Times(0);
    EXPECT_CALL(m_Mock2, onItem1(_)).Times(0);

    sig.disconnect(&m_Mock2);

    sig(2);
}

TEST_F(SignalTest, ConnectDisconnect2)
{
    Signal<const int&, const int&> sig;
    EXPECT_CALL(m_Mock1, onItem2(_, _)).Times(0);
    EXPECT_CALL(m_Mock2, onItem2(_, _)).Times(0);

    sig(0, 0);
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    sig.connect(&m_Mock1, std::bind(&ReceiverMock<const int&>::onItem2, &m_Mock1, _1, _2));
    sig.connect(&m_Mock2, std::bind(&ReceiverMock<const int&>::onItem2, &m_Mock2, _1, _2));

    EXPECT_CALL(m_Mock1, onItem2(1, 5)).Times(1);
    EXPECT_CALL(m_Mock2, onItem2(1, 5)).Times(1);

    sig(1, 5);
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    EXPECT_CALL(m_Mock1, onItem2(_, _)).Times(0);
    EXPECT_CALL(m_Mock2, onItem2(2, 7)).Times(1);

    sig.disconnect(&m_Mock1);
    sig(2, 7);
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    EXPECT_CALL(m_Mock1, onItem2(_, _)).Times(0);
    EXPECT_CALL(m_Mock2, onItem2(_, _)).Times(0);

    sig.disconnect(&m_Mock2);

    sig(2, 7);
}

TEST_F(SignalTest, ConnectDisconnect3)
{
    Signal<const int&, const int&, const int&> sig;
    EXPECT_CALL(m_Mock1, onItem3(_, _, _)).Times(0);
    EXPECT_CALL(m_Mock2, onItem3(_, _, _)).Times(0);

    sig(0, 0, 0);
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    sig.connect(&m_Mock1, std::bind(&ReceiverMock<const int&>::onItem3, &m_Mock1, _1, _2, _3));
    sig.connect(&m_Mock2, std::bind(&ReceiverMock<const int&>::onItem3, &m_Mock2, _1, _2, _3));

    EXPECT_CALL(m_Mock1, onItem3(1, 5, 9)).Times(1);
    EXPECT_CALL(m_Mock2, onItem3(1, 5, 9)).Times(1);

    sig(1, 5, 9);
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    EXPECT_CALL(m_Mock1, onItem3(_, _, _)).Times(0);
    EXPECT_CALL(m_Mock2, onItem3(2, 7, 0)).Times(1);

    sig.disconnect(&m_Mock1);
    sig(2, 7, 0);
    Mock::VerifyAndClearExpectations(&m_Mock1);
    Mock::VerifyAndClearExpectations(&m_Mock2);

    EXPECT_CALL(m_Mock1, onItem3(_, _, _)).Times(0);
    EXPECT_CALL(m_Mock2, onItem3(_, _, _)).Times(0);

    sig.disconnect(&m_Mock2);

    sig(2, 7, 0);
}

TEST_F(SignalTest, ValueArgument)
{
    int32_t integer1 = 0;
    int32_t integer2 = 1;

    ReceiverMock<int32_t> mock;

    Signal<int32_t, int32_t> sig;
    EXPECT_CALL(mock, onItem2(integer1, integer2)).Times(1);

    sig.connect(&mock, std::bind(&ReceiverMock<int32_t>::onItem2, &mock, _1, _2));
    sig(integer1, integer2);
}

TEST_F(SignalTest, PointerArgument)
{
    int integer = 0;

    ReceiverMock<int32_t*> mock;

    Signal<int32_t*> sig;
    EXPECT_CALL(mock, onItem1(&integer)).Times(1);

    sig.connect(&mock, std::bind(&ReceiverMock<int32_t*>::onItem1, &mock, _1));
    sig(&integer);
}

TEST_F(SignalTest, NonCopyableArgument)
{
    std::unique_ptr<int32_t> ptr;
    ReceiverMock<std::unique_ptr<int32_t>&> mock;

    Signal<std::unique_ptr<int32_t>&> sig;
    EXPECT_CALL(mock, onItem1(_)).Times(1);

    sig.connect(&mock, std::bind(&ReceiverMock<std::unique_ptr<int32_t>&>::onItem1, &mock, _1));
    sig(ptr);
}
}
