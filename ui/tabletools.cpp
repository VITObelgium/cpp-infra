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

    QStringList currentRow;

    QString selected_text;
    // You need a pair of indexes to find the row changes
    int previousRow = indexes.front().row();
    for (auto& current : indexes) {
        if (current.row() != previousRow) {
            selected_text.append(currentRow.join('\t'));
            selected_text.append('\n');
            currentRow.clear();
        }

        currentRow.append(model->data(current).toString());
        previousRow = current.row();
    }

    if (!currentRow.empty()) {
        selected_text.append(currentRow.join('\t'));
    }

    QApplication::clipboard()->setText(selected_text);
}
}
