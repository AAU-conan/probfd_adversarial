#ifndef DOMINANCE_LD_SIMULATION_H
#define DOMINANCE_LD_SIMULATION_H

#include "dominance_analysis.h"
#include "probfd/dominance/strong_types.h"

#include <vector>


namespace probfd::dominance {
    class LabelOutcomeMap;
    class FTSTask;
    class LabelledTransitionSystem;
    class FactorDominanceRelationFactory;
    class FactorDominanceRelation;
    class LabelOutcomeRelation;
    class LabelRelationFactory;

    bool update_local_relation(FactorIndex factor, const FTSTask& task, const LabelOutcomeRelation& label_dominance,
                               FactorDominanceRelation& local_relation, const LabelOutcomeMap& label_outcome_map);
    bool update_label_relation(LabelOutcomeRelation& label_relation, const FTSTask & task, const std::vector<std::unique_ptr<FactorDominanceRelation>> &sim, const LabelOutcomeMap& label_outcome_map);

    class LDSimulation : public DominanceAnalysis {
        std::shared_ptr<FactorDominanceRelationFactory> factor_dominance_relation_factory;
        std::shared_ptr<LabelRelationFactory> label_relation_factory;

        std::unique_ptr<StateDominanceRelation> compute_ld_simulation(const FTSTask & task);
    public:
        explicit LDSimulation(std::shared_ptr<FactorDominanceRelationFactory> factor_dominance_relation_factory, std::shared_ptr<LabelRelationFactory> label_relation_factory);

        virtual ~LDSimulation() = default;
        std::unique_ptr<StateDominanceRelation> compute_dominance_relation(const FTSTask &task) override;
    };
}

#endif
