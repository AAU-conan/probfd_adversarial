#ifndef DRAW_FTS_H
#define DRAW_FTS_H

#include <string>

namespace probfd::dominance {
    class FTSTask;
    void draw_fts(const std::string &filename, const FTSTask &fts);
}

#endif //DRAW_FTS_H
