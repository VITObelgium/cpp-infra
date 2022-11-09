#include "uiinfra/tabletools.h"

#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qitemselectionmodel.h>

namespace inf::ui {

void itemSelectionToClipboard(const QItemSelectionModel* selectionModel, char doubleFormat, int precision)
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
        QVariant data = model->data(current);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const bool isDouble = data.type() == QVariant::Double;
#else
        const bool isDouble = data.metaType().id() == QMetaType::Double;
#endif

        if (isDouble) {
            // make sure to convert to string in the system locale, otherwise tools like excel
            // don't understand it
            rows[current.row()].append(QLocale().toString(data.toDouble(), doubleFormat, precision));
        } else {
            rows[current.row()].append(data.toString());
        }
    }

    QStringList rowStrings;
    for (auto& row : rows) {
        rowStrings.push_back(row.second.join('\t'));
    }

    QApplication::clipboard()->setText(rowStrings.join('\n'));
}
}
