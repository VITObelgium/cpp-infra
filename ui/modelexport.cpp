#include "uiinfra/modelexport.h"
#include "infra/cast.h"
#include "infra/exception.h"
#include "infra/log.h"
#include "infra/string.h"

#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <xlsxwriter.h>

namespace inf::ui {

namespace xl {
class WorkBook
{
public:
    WorkBook(const std::string& name)
    : _ptr(workbook_new(name.c_str()))
    {
        if (!_ptr) {
            throw RuntimeError("Failed to create workbook: {}", name);
        }
    }

    ~WorkBook()
    {
        auto error = workbook_close(_ptr);
        if (error != LXW_NO_ERROR) {
            Log::error("Failed to write excel file: {}", lxw_strerror(error));
        }
    }

    operator lxw_workbook*()
    {
        return _ptr;
    }

    lxw_workbook* _ptr;
};
}

bool exportModel(QWidget* parent, QAbstractItemModel* model, std::string_view name, Flags<ExportModelOptions> options)
{
    auto filename = QFileDialog::getSaveFileName(parent, QApplication::tr("Export table"), QString::fromStdString(std::string(name)), QStringLiteral("Spreadsheet (*.xlsx)"));
    if (!filename.isEmpty()) {
        exportModel(model, name, fs::path(filename.toStdString()), options);
        return true;
    }

    return false;
}

void exportModel(QAbstractItemModel* model, std::string_view name, fs::path outputPath, Flags<ExportModelOptions> options)
{
    if (fs::exists(outputPath)) {
        fs::remove(outputPath);
    }

    // Each field definition defines a column in the spreadsheet
    std::string layerName(name);
    if (layerName.size() > 31) {
        // Excel does not support tab names longer then 31 characters
        layerName.resize(31);
    }

    auto pathString = outputPath.generic_string();
    xl::WorkBook wb(pathString);
    auto* ws = workbook_add_worksheet(wb, layerName.c_str());
    if (!ws) {
        throw RuntimeError("Failed to add sheet to excel document");
    }

    auto qtColor = QColor(Qt::lightGray);

    auto* headerFormat = workbook_add_format(wb);
    format_set_bold(headerFormat);
    format_set_bg_color(headerFormat, qtColor.rgba());

    bool hasVerticalHeaders = model->headerData(0, Qt::Vertical).isValid() && !options.is_set(ExportModelOptions::NoVerticalHeaders);
    int32_t colOffset       = hasVerticalHeaders ? 1 : 0;
    int32_t rowOffset       = options.is_set(ExportModelOptions::NoHorizontalHeaders) ? 0 : 1;

    if (!options.is_set(ExportModelOptions::NoHorizontalHeaders)) {
        for (int j = 0; j < model->columnCount(); ++j) {
            auto data = model->headerData(j, Qt::Horizontal);
            worksheet_write_string(ws, 0, colOffset + j, data.toString().toUtf8(), headerFormat);
        }
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        const auto row = truncate<lxw_row_t>(i + rowOffset);
        if (hasVerticalHeaders) {
            worksheet_write_string(ws, row, 0, model->headerData(i, Qt::Vertical).toString().toUtf8(), headerFormat);
        }

        for (int j = 0; j < model->columnCount(); ++j) {
            const auto col         = truncate<lxw_col_t>(colOffset + j);
            auto data              = model->index(i, j).data();
            lxw_format* cellFormat = nullptr;
            auto color             = model->index(i, j).data(Qt::ForegroundRole);
            auto bgColor           = model->index(i, j).data(Qt::BackgroundRole);
            if (color.isValid() || bgColor.isValid()) {
                cellFormat = workbook_add_format(wb);
                if (color.isValid()) {
                    format_set_font_color(cellFormat, color.value<QBrush>().color().rgba());
                }

                if (bgColor.isValid()) {
                    format_set_bg_color(cellFormat, bgColor.value<QBrush>().color().rgba());
                }
            }

            auto fontVar = model->index(i, j).data(Qt::FontRole);
            if (fontVar.isValid()) {
                auto font = fontVar.value<QFont>();
                if (font.bold() || font.italic()) {
                    if (!cellFormat) {
                        cellFormat = workbook_add_format(wb);
                    }

                    if (font.bold()) {
                        format_set_bold(cellFormat);
                    }

                    if (font.italic()) {
                        format_set_italic(cellFormat);
                    }
                }
            }

            if (data.isValid()) {
                switch (data.type()) {
                case QVariant::Int:
                case QVariant::UInt:
                case QVariant::LongLong:
                case QVariant::ULongLong:
                case QVariant::Double:
                    worksheet_write_number(ws, row, col, data.toDouble(), cellFormat);
                    break;
                case QVariant::String:
                    worksheet_write_string(ws, row, col, data.toString().toUtf8(), cellFormat);
                    break;
                default:
                    if (data.canConvert(QVariant::String)) {
                        if (data.convert(QVariant::String)) {
                            worksheet_write_string(ws, row, col, data.toString().toUtf8(), cellFormat);
                        }
                    }
                    break;
                }
            } else if (bgColor.isValid()) {
                // empty cell with a background
                worksheet_write_string(ws, row, col, "", cellFormat);
            }
        }
    }
}
}
