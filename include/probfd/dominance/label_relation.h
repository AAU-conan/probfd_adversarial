#ifndef DOMINANCE_LABEL_RELATION_H
#define DOMINANCE_LABEL_RELATION_H

#include "probfd/dominance/strong_types.h"

#include <memory>

namespace probfd::dominance {
    class FTSTask;
    class LabelledTransitionSystem;
    class LabelMap;
    class FactorDominanceRelation;
    class LabelOutcomeMap;

    /*
     * Label relation represents the preorder relations on labels that
     * occur in a set of LTS
     */
    class LabelOutcomeRelation {
    protected:
        size_t num_label_outcomes;

    public:
        explicit LabelOutcomeRelation(int num_label_outcomes);
        virtual ~LabelOutcomeRelation() = default;

        [[nodiscard]] virtual bool label_dominates_label_in_all_other(FactorIndex factor, const FTSTask& fts_task, LabelOutcome lo1, LabelOutcome lo2) const = 0;

        [[nodiscard]] virtual bool noop_dominates_label_in_all_other(FactorIndex factor, const FTSTask& fts_task, LabelOutcome lo) const = 0;

        virtual bool update_factor(
            FactorIndex factor,
            const FTSTask& fts_task,
            const FactorDominanceRelation& sim,
            const LabelOutcomeMap& label_outcome_map) = 0;

        virtual void dump(
            std::ostream& os,
            const FTSTask& fts_task,
            const LabelOutcomeMap& label_outcome_map) const;
    };

    class LabelRelationFactory {
    public:
        virtual ~LabelRelationFactory() = default;
        virtual std::unique_ptr<LabelOutcomeRelation> create(const FTSTask& fts_task, const LabelOutcomeMap& label_outcome_map) = 0;
    };

    template<class LabelRelationType>
    class LabelRelationFactoryImpl final : public LabelRelationFactory {
        std::unique_ptr<LabelOutcomeRelation> create(const FTSTask& fts_task, const LabelOutcomeMap& label_outcome_map) override {
            return std::make_unique<LabelRelationType>(fts_task, label_outcome_map);
        }
    };
}
    

#endif
