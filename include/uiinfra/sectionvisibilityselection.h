#pragma once

#include <qglobal.h>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QHeaderView)

namespace inf::ui {

/*! Sets up a right click popup menu for the header that allows hiding sections
    section numbers in the fixedSections vector will not be included
*/
void setSectionVisibilitySelector(QHeaderView* headerView, const std::vector<int>& fixedSections = {0});
}
