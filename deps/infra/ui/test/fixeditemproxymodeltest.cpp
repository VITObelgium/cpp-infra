#include "uiinfra/fixeditemproxymodel.h"

#include "simpletreemodel.h"

#include <gtest/gtest.h>
#include <qstandarditemmodel.h>
#include <qstringlistmodel.h>

void PrintTo(const QString& str, ::std::ostream* os)
{
    *os << str.toStdString();
}

namespace uiinfra::test {

using namespace testing;

class FixedItemProxyModelTest : public Test
{
protected:
    FixedItemProxyModelTest()
    : modelData(QStringLiteral("Level1\tValue1\n"
                               "   Level2.1\tValue2.1\n"
                               "   Level2.2\tValue2.2\n"
                               "      Level3\tValue3.1\n"
                               "   Level2.3\tValue2.3\n"))
    {
    }

    QString modelData;
};

TEST_F(FixedItemProxyModelTest, listModel)
{
    QStringListModel sourceModel;
    sourceModel.setStringList({"One", "Two", "Three"});

    FixedItemProxyModel proxyModel;
    proxyModel.setFixedItems({"All"});
    proxyModel.setSourceModel(&sourceModel);

    EXPECT_EQ(4, proxyModel.rowCount());
    EXPECT_EQ(QString("All"), proxyModel.index(0, 0).data().toString());
    EXPECT_EQ(QString("One"), proxyModel.index(1, 0).data().toString());
    EXPECT_EQ(QString("Two"), proxyModel.index(2, 0).data().toString());
    EXPECT_EQ(QString("Three"), proxyModel.index(3, 0).data().toString());
}

TEST_F(FixedItemProxyModelTest, treeModel)
{
    TreeModel sourceModel(modelData);
    FixedItemProxyModel proxyModel;
    proxyModel.setFixedItems({"All", "None"});
    proxyModel.setSourceModel(&sourceModel);

    EXPECT_EQ(3, proxyModel.rowCount());
    EXPECT_EQ(QString("All"), proxyModel.index(0, 0).data().toString());
    EXPECT_EQ(QString("None"), proxyModel.index(1, 0).data().toString());
    EXPECT_EQ(QString("Level1"), proxyModel.index(2, 0).data().toString());
}

TEST_F(FixedItemProxyModelTest, treeModelRootIndex)
{
    TreeModel sourceModel(modelData);
    FixedItemProxyModel proxyModel;
    proxyModel.setFixedItems({"All"});
    proxyModel.setSourceModel(&sourceModel);

    proxyModel.setRootModelIndex(sourceModel.index(0, 0));
    EXPECT_EQ(4, proxyModel.rowCount());
    EXPECT_EQ(QString("All"), proxyModel.index(0, 0).data().toString());
    EXPECT_EQ(QString("Level2.1"), proxyModel.index(1, 0).data().toString());
    EXPECT_EQ(QString("Level2.2"), proxyModel.index(2, 0).data().toString());
    EXPECT_EQ(QString("Level2.3"), proxyModel.index(3, 0).data().toString());

    EXPECT_EQ(QString("Value2.1"), proxyModel.index(1, 1).data().toString());
    EXPECT_EQ(QString("Value2.2"), proxyModel.index(2, 1).data().toString());
    EXPECT_EQ(QString("Value2.3"), proxyModel.index(3, 1).data().toString());
}
}
