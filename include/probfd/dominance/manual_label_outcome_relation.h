#ifndef MANUAL_LABEL_OUTCOME_RELATION_H
#define MANUAL_LABEL_OUTCOME_RELATION_H

#include "probfd/dominance/label_relation.h"
#include "probfd/dominance/strong_types.h"
#include "probfd/dominance/all_none_factor_index.h"

#include <vector>

namespace probfd::dominance {
    class LabelOutcomeMap;

    class ManualLabelOutcomeRelation final : public LabelOutcomeRelation {
        std::vector<std::vector<AllNoneFactorIndex>> dominates_in;
        std::vector<AllNoneFactorIndex> dominated_by_noop_in;
    public:
        ManualLabelOutcomeRelation(int num_label_outcomes)
            : LabelOutcomeRelation(num_label_outcomes)
        {
            dominates_in.resize( num_label_outcomes, std::vector( num_label_outcomes, AllNoneFactorIndex::no_factors()));
            dominated_by_noop_in.resize( num_label_outcomes, AllNoneFactorIndex::no_factors());
        }

        [[nodiscard]] bool label_dominates_label_in_all_other( FactorIndex factor, const FTSTask& fts_task, LabelOutcome lo1, LabelOutcome lo2) const override {
            return dominates_in[lo1][lo2].contains_all_except(factor);
        }

        [[nodiscard]] bool noop_dominates_label_in_all_other( FactorIndex factor, const FTSTask& fts_task, LabelOutcome lo) const override {
            return dominated_by_noop_in[lo].contains_all_except(factor);
        }

        bool update_factor( FactorIndex factor, const FTSTask& fts_task, const FactorDominanceRelation& sim, const LabelOutcomeMap& label_outcome_map) override
        {
            throw std::runtime_error("ManualLabelOutcomeRelation does not support update_factor");
        }


        void set_identity() {
            for (LabelOutcome lo1(0); lo1 < dominates_in.size(); ++lo1) {
                for (LabelOutcome lo2(0); lo2 < dominates_in[lo1].size(); ++lo2) {
                    if (lo1 == lo2) {
                        dominates_in[lo1][lo2] = AllNoneFactorIndex::all_factors();
                    } else {
                        dominates_in[lo1][lo2] = AllNoneFactorIndex::no_factors();
                    }
                }
            }
        }

        void set_dominates(LabelOutcome lo1, LabelOutcome lo2, AllNoneFactorIndex factors) {
            dominates_in[lo1][lo2] = factors;
        }

        void set_noop_dominates(LabelOutcome lo, AllNoneFactorIndex factors) {
            dominated_by_noop_in[lo] = factors;
        }
    };
}
#endif //MANUAL_LABEL_OUTCOME_RELATION_H
