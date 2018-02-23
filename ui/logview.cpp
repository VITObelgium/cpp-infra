#include "uiinfra/logview.h"
#include "ui_logview.h"

#include <qevent.h>

namespace uiinfra {

LogView::LogView(QWidget* parent)
: QWidget(parent)
, _ui(std::make_unique<Ui::LogView>())
{
    _ui->setupUi(this);
}

LogView::~LogView() = default;

void LogView::setModel(QAbstractItemModel* model)
{
    _ui->tableView->setModel(model);
}

void LogView::resizeEvent(QResizeEvent* event)
{
    _ui->tableView->horizontalHeader()->resizeSection(0, event->size().width());

    QWidget::resizeEvent(event);
}
}
