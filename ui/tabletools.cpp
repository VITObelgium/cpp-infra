#include "uiinfra/tabletools.h"

#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qitemselectionmodel.h>

namespace uiinfra {

void itemSelectionToClipboard(const QItemSelectionModel* selectionModel)
{
    if (!selectionModel) {
        return;
    }

    auto* model  = selectionModel->model();
    auto indexes = selectionModel->selectedIndexes();

    if (indexes.empty()) {
        return;
    }

    std::map<int, QStringList> rows;

    QString selected_text;
    for (auto& current : indexes) {
        rows[current.row()].append(model->data(current).toString());
    }

    QStringList rowStrings;
    for (auto& row : rows) {
        rowStrings.push_back(row.second.join('\t'));
    }

    QApplication::clipboard()->setText(rowStrings.join('\n'));
}
}
