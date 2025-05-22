#ifndef DOMINANCE_DOMINANCE_ANALYSIS_H
#define DOMINANCE_DOMINANCE_ANALYSIS_H

#include <memory>

namespace probfd::dominance {
    class StateDominanceRelation;
    class FTSTask;

    class DominanceAnalysis {
    public:
        virtual ~DominanceAnalysis() = default;

        virtual std::unique_ptr<StateDominanceRelation> compute_dominance_relation(const FTSTask &task) = 0;
    };

}

#endif

