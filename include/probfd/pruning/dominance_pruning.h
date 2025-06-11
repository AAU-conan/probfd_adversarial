#ifndef DOMINANCE_PRUNING_H
#define DOMINANCE_PRUNING_H

#include "probfd/pruning_method.h"
#include "probfd/transition_tail.h"
#include "probfd/task_pruning_factory.h"
#include "probfd/mdp.h"

#include "probfd/dominance/dominance_analysis.h"
#include "probfd/dominance/state_dominance_relation.h"

#include <probfd/utils/state_name.h>

using namespace probfd::dominance;

namespace probfd::pruning {

class DominancePruning final : public PruningMethod<downward::State, downward::OperatorID> {
    std::shared_ptr<StateDominanceRelation> dominance_relation;

public:
    explicit DominancePruning(
        std::shared_ptr<StateDominanceRelation> dominance_relation)
        : dominance_relation(std::move(dominance_relation))
    {
    }

    void prune_transitions(
        StateSpace<downward::State, downward::OperatorID>& state_space,
        ParamType<downward::State> source_state,
        std::vector<TransitionTail<downward::OperatorID>>& transition_tails)
        override
    {
        // We can prune a transition if any target state is dominated by the
        // source state, or if there is another transition s.t. all targes of that
        // transitions dominate a target of the current transition.
        std::ranges::remove_if( transition_tails,
        [&](TransitionTail<downward::OperatorID>& tail) {
            if (std::ranges::any_of(
                    tail.successor_dist.non_source_successor_dist,
                    [&](const ItemProbabilityPair<StateID>& target) {
                        auto target_state = state_space.get_state(target.item);
                        return dominance_relation->dominates(source_state, target_state);
                    })) {
                // std::println("Pruned transition {} --{}--> because it has a target state that is dominated by the source state.", source_state.get_id().get_value(), action_name(source_state.get_task(), tail.action));
                ++num_transitions_pruned;
                return true; // Prune this transition
            }
            if (std::ranges::any_of(
            transition_tails,
            [&](const TransitionTail<downward::OperatorID>& other_tail) {
                if (other_tail.action == tail.action) {
                    return false; // Skip the current tail
                }
                // Dominates if it has at least one target and all targets dominate a target of tail
                // std::println("Checking other transition {} --{}-->", source_state.get_id().get_value(), action_name(source_state.get_task(), other_tail.action));
                bool dominates = !other_tail.successor_dist.non_source_successor_dist.empty() && std::ranges::all_of(
                    other_tail.successor_dist.non_source_successor_dist,
                    [&](const ItemProbabilityPair<StateID>& other_target_state_pair) {
                        const auto other_target_state = state_space.get_state(other_target_state_pair.item);

                        // std::println("Checking target {}", other_target_state.get_id().get_value());
                        return std::ranges::any_of(
                            tail.successor_dist.non_source_successor_dist,
                            [&](const ItemProbabilityPair<StateID>& this_target_state_pair) {
                                const auto this_targe_state = state_space.get_state(this_target_state_pair.item);

                                bool res = dominance_relation->dominates( other_target_state, this_targe_state);
                                // if (res) {
                                //     std::println("Target {} dominates {}", this_targe_state.get_id().get_value(), other_target_state.get_id().get_value());
                                // }
                                return res;
                            });
                    });
                // if (dominates) {
                //     std::println("Pruned transition {} --{}--> because it is dominated by {}", source_state.get_id().get_value(), action_name(source_state.get_task(), tail.action), action_name(source_state.get_task(), other_tail.action));
                // }
                return dominates;
            })) {
                ++num_transitions_pruned;
                return true; // Prune this transition
            }
            return false;
        });
    }

    void prune_distribution(
        StateSpace<downward::State, downward::OperatorID>& state_space,
        Distribution<StateID>& distribution) override
    {
        distribution.remove_if([&](const ItemProbabilityPair<StateID>& target_state) {
            // We can prune an outcome if it DOMINATES any other state in the distribution
            // The max-player will never choose this outcome, because the other is worse
            for (const auto& [other_state_id, _] : distribution) {
                if (target_state.item != other_state_id && dominance_relation->dominates(state_space.get_state(target_state.item), state_space.get_state(other_state_id))) {
                    // std::println("Pruned outcome {} because it dominates {}", target_state.item.id, other_state_id.id);
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
