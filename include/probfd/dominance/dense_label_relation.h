#ifndef DOMINANCE_DENSE_LABEL_RELATION_H
#define DOMINANCE_DENSE_LABEL_RELATION_H

#include "probfd/dominance/label_relation.h"
#include "probfd/dominance/strong_types.h"
#include "probfd/dominance/all_none_factor_index.h"

#include <vector>

namespace probfd::dominance {
    class LabelOutcomeMap;

    /*
     * The dense label relation stores a dense matrix of all label-outcome pairs.
     */
    class DenseLabelOutcomeRelation : public LabelOutcomeRelation {
        // Matrix factors where lo1 simulates lo2
        std::vector<std::vector<AllNoneFactorIndex> > dominates_in;

        //Indicates whether labels are dominated by noop or other irrelevant
        //variables in theta
        std::vector<std::vector<bool> > simulated_by_irrelevant;
        std::vector<std::vector<bool> > simulates_irrelevant;

        std::vector<AllNoneFactorIndex> dominated_by_noop_in;

        //Returns true if l1 simulates l2 in lts
        [[nodiscard]] bool simulates(LabelOutcome lo1, LabelOutcome lo2, FactorIndex factor) const;

        [[nodiscard]] bool noop_simulates(LabelOutcome lo, FactorIndex factor) const;


        inline void set_not_simulates(LabelOutcome lo1, LabelOutcome lo2, FactorIndex factor);

        inline bool set_not_simulated_by_irrelevant(LabelOutcome lo, FactorIndex factor);

    public:

        [[nodiscard]] bool label_dominates_label_in_all_other(FactorIndex factor, const FTSTask& fts_task, LabelOutcome lo1, LabelOutcome lo2) const override;

        [[nodiscard]] bool noop_dominates_label_in_all_other(FactorIndex factor, const FTSTask& fts_task, LabelOutcome lo) const override;

        bool update_factor(
            FactorIndex factor,
            const FTSTask& fts_task,
            const FactorDominanceRelation& sim,
            const LabelOutcomeMap& label_outcome_map) override;

        DenseLabelOutcomeRelation(
            const FTSTask& fts_task,
            const LabelOutcomeMap& label_outcome_map);

        inline size_t get_num_labels() const;

        void dump(std::ostream& os, const FTSTask& fts_task, const LabelOutcomeMap& label_outcome_map) const override;
    };
}



#endif
