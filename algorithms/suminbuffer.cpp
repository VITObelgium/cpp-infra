#include "gdx/algo/suminbuffer.h"

namespace gdx {

// summed circular area, using sliding window technique.  Much slower than integral image technique.
void computeCircleBorderOffsets(const int radius /*cells*/,
    std::vector<Cell>& plusRight,
    std::vector<Cell>& minLeft,
    std::vector<Cell>& plusDown,
    std::vector<Cell>& minTop)
{
    plusRight.clear();
    minLeft.clear();
    plusDown.clear();
    minTop.clear();
    const int rad2 = radius * radius;
    for (int dR = -radius; dR <= +radius; ++dR) {
        const int dR2 = dR * dR;
        for (int dC = -radius; dC <= +radius; ++dC) {
            if (dR2 + dC * dC <= rad2 && dR2 + (dC + 1) * (dC + 1) > rad2) {
                plusRight.emplace_back(dR, dC); // its a right border
            }
            if (dR2 + dC * dC <= rad2 && dR2 + (dC - 1) * (dC - 1) > rad2) {
                minLeft.emplace_back(dR, dC - 1); // its a left border
            }
            if (dR2 + dC * dC <= rad2 && (dR + 1) * (dR + 1) + dC * dC > rad2) {
                plusDown.emplace_back(dR, dC); // its bottom border
            }
            if (dR2 + dC * dC <= rad2 && (dR - 1) * (dR - 1) + dC * dC > rad2) {
                minTop.emplace_back(dR - 1, dC); // its top border
            }
        }
    }
}
}
