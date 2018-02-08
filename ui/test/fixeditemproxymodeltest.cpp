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
public:
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

TEST_F(FixedItemProxyModelTest, DISABLED_treeModel)
{
    QString modelData = "Level1\tValue1\n"
                        "   Level2\tValue2.1\n"
                        "   Level2\tValue2.2\n"
                        "      Level3\tValue3.1\n"
                        "   Level2\tValue2.3\n";

    TreeModel sourceModel(modelData);
    FixedItemProxyModel proxyModel;
    proxyModel.setFixedItems({"All"});
    proxyModel.setSourceModel(&sourceModel);

    EXPECT_EQ(2, proxyModel.rowCount());
    EXPECT_EQ(QString("All"), proxyModel.index(0, 0).data().toString());
    EXPECT_EQ(QString("Level1"), proxyModel.index(1, 0).data().toString());

    EXPECT_EQ(4, proxyModel.rowCount(proxyModel.index(0, 0)));
}
}
