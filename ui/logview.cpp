#include "uiinfra/logview.h"
#include "ui_logview.h"

#include <qevent.h>
#include <qtimer.h>

namespace inf::ui {

static auto s_maxUpdateInterval = std::chrono::milliseconds(200);

LogView::LogView(QWidget* parent)
: QWidget(parent)
, _ui(std::make_unique<Ui::LogView>())
, _timer(new QTimer(this))
{
    _ui->setupUi(this);

    connect(_timer, &QTimer::timeout, this, &LogView::updateView);
}

LogView::~LogView() = default;

void LogView::setModel(QAbstractItemModel* model)
{
    _ui->tableView->setModel(model);

    connect(model, &QAbstractItemModel::rowsInserted, this, [this]() {
        if (!_timer->isActive()) {
            updateView();
            _timer->start(s_maxUpdateInterval);
        }
    });
}

void LogView::resizeEvent(QResizeEvent* event)
{
    _ui->tableView->horizontalHeader()->resizeSection(0, event->size().width());
    QWidget::resizeEvent(event);
}

void LogView::updateView()
{
    _timer->stop();
    if (_ui->tableView->isVisible()) {
        _ui->tableView->scrollToBottom();
    }
}

}
