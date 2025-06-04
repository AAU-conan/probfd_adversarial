#ifndef DOMINANCE_PRUNING_H
#define DOMINANCE_PRUNING_H

#include "probfd/pruning_method.h"
#include "probfd/transition_tail.h"
#include "probfd/task_pruning_factory.h"
#include "probfd/mdp.h"

#include "probfd/dominance/dominance_analysis.h"
#include "probfd/dominance/state_dominance_relation.h"

using namespace probfd::dominance;

namespace probfd::pruning {

class DominancePruning final : public PruningMethod<downward::State, downward::OperatorID> {
    std::shared_ptr<StateDominanceRelation> dominance_relation;

public:
    explicit DominancePruning(std::shared_ptr<StateDominanceRelation> dominance_relation)
        : dominance_relation(std::move(dominance_relation))
    {}

    bool can_prune_transition(
        StateSpace<downward::State, downward::OperatorID>& state_space,
        ParamType<downward::State> source_state,
        const TransitionTail<downward::OperatorID>& transition_tail) override
    {
        // We can prune a transition if any target state is dominated by the source state
        for (const auto& [target_state_id, _] : transition_tail.successor_dist.non_source_successor_dist) {
            auto target_state = state_space.get_state(target_state_id);
            if (dominance_relation->dominates(source_state, target_state)) {
                ++num_transitions_pruned;
                return true; // Prune this transition
            }
        }
        return false;
    }

    bool prune_distribution(
                StateSpace<downward::State, downward::OperatorID>& state_space,
                Distribution<StateID>& distribution) override
    {
        return distribution.remove_if([&](const ItemProbabilityPair<StateID>& target_state) {
            // We can prune an outcome if it is dominated by any other state in the distribution
            for (const auto& [other_state_id, prob] : distribution) {
                if (target_state.item != other_state_id && dominance_relation->dominates(state_space.get_state(other_state_id), state_space.get_state(target_state.item))) {
                    ++num_outcomes_pruned;
                    return true;
                }
            }
            return false;
        });
    }
};

class DominancePruningFactory : public TaskPruningFactory {
    std::shared_ptr<DominanceAnalysis> dominance_analysis;
public:
    explicit DominancePruningFactory(std::shared_ptr<DominanceAnalysis> dominance_analysis)
        : dominance_analysis(std::move(dominance_analysis))
    {}

    std::unique_ptr<FDRPruningMethod> create_pruning_method(std::shared_ptr<ProbabilisticTask> task) override;

};

}


#endif //DOMINANCE_PRUNING_H
