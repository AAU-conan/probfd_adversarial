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
    ProbabilisticTaskProxy task_proxy;

    bool compare_initial_state;
    bool compare_source_state;
    bool compare_other_transitions;
    bool compare_other_targets;

    int num_transitions_pruned_by_initial_state = 0;
    int num_transitions_pruned_by_source_state = 0;
    int num_transitions_pruned_by_other_transitions = 0;
    int num_outcomes_pruned_by_other_targes = 0;

    std::optional<downward::State> initial_state = std::nullopt;

public:
    explicit DominancePruning(ProbabilisticTaskProxy task_proxy,
        std::shared_ptr<StateDominanceRelation> dominance_relation, bool compare_initial_state, bool compare_source_state, bool compare_other_transitions, bool compare_other_targets)
        : task_proxy(std::move(task_proxy)),
          dominance_relation(std::move(dominance_relation)),
          compare_initial_state(compare_initial_state),
          compare_source_state(compare_source_state),
          compare_other_transitions(compare_other_transitions),
          compare_other_targets(compare_other_targets)
    {
    }

    void prune_transitions(
        StateSpace<downward::State, downward::OperatorID>& state_space,
        ParamType<downward::State> source_state,
        std::vector<TransitionTail<downward::OperatorID>>& transition_tails)
        override
    {
        if (compare_initial_state || compare_source_state || compare_other_transitions) {
            // We can prune a transition if any target state is dominated by the
            // source state, or if there is another transition s.t. all targes of that
            // transitions dominate a target of the current transition.
            std::ranges::remove_if( transition_tails,
            [&](TransitionTail<downward::OperatorID>& tail) {
                if (compare_initial_state) {
                    if (!initial_state.has_value()) {
                        initial_state = source_state.get_task().get_initial_state();
                    }
                    if (std::ranges::any_of(
                            tail.successor_dist.non_source_successor_dist,
                            [&](const ItemProbabilityPair<StateID>& target) {
                                auto target_state = state_space.get_state(target.item);
                                return dominance_relation->dominates(*initial_state, target_state);
                            })) {
                        // std::println("Pruned transition {} --{}--> because it has a target state that is dominated by the initial state.", source_state.get_id().get_value(), action_name(source_state.get_task(), tail.action));
                        ++num_transitions_pruned;
                        ++num_transitions_pruned_by_initial_state;
                        return true; // Prune this transition
                    }
                }
                if (compare_source_state && std::ranges::any_of(
                        tail.successor_dist.non_source_successor_dist,
                        [&](const ItemProbabilityPair<StateID>& target) {
                            auto target_state = state_space.get_state(target.item);
                            return dominance_relation->dominates(source_state, target_state);
                        })) {
                    // std::println("Pruned transition {} --{}--> because it has a target state that is dominated by the source state.", source_state.get_id().get_value(), action_name(source_state.get_task(), tail.action));
                    ++num_transitions_pruned;
                    ++num_transitions_pruned_by_source_state;
                    return true; // Prune this transition
                }
                if (compare_other_transitions && std::ranges::any_of(
                transition_tails,
                [&](const TransitionTail<downward::OperatorID>& other_tail) {
                    if (other_tail.action == tail.action) {
                        return false; // Skip the current tail
                    }
                    // Dominates if it has at least one target and all targets dominate a target of tail
                    // std::println("Checking other transition {} --{}-->", source_state.get_id().get_value(), action_name(source_state.get_task(), other_tail.action));
                    bool dominates = task_proxy.get_operators()[other_tail.action].get_cost() <= task_proxy.get_operators()[tail.action].get_cost() && !other_tail.successor_dist.non_source_successor_dist.empty() && std::ranges::all_of(
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
                    ++num_transitions_pruned_by_other_transitions;
                    return true; // Prune this transition
                }
                return false;
            });
        }
    }

    void prune_distribution(
        StateSpace<downward::State, downward::OperatorID>& state_space,
        Distribution<StateID>& distribution) override
    {
        if (compare_other_targets) {
            distribution.remove_if([&](const ItemProbabilityPair<StateID>& target_state) {
                // We can prune an outcome if it DOMINATES any other state in the distribution
                // The max-player will never choose this outcome, because the other is worse
                for (const auto& [other_state_id, _] : distribution) {
                    if (target_state.item != other_state_id && dominance_relation->dominates(state_space.get_state(target_state.item), state_space.get_state(other_state_id))) {
                        // std::println("Pruned outcome {} because it dominates {}", target_state.item.id, other_state_id.id);
                        ++num_outcomes_pruned;
                        ++num_outcomes_pruned_by_other_targes;
                        return true;
                    }
                }
                return false;
            });
        }
    }


    void print_statistics() const override
    {
        PruningMethod<downward::State, downward::OperatorID>::print_statistics();
        std::println("  Pruned transition(s) by initial state: {}", num_transitions_pruned_by_initial_state);
        std::println("  Pruned transition(s) by source state: {}", num_transitions_pruned_by_source_state);
        std::println("  Pruned transition(s) by other transitions: {}", num_transitions_pruned_by_other_transitions);
        std::println("  Pruned outcome(s) by other targets: {}", num_outcomes_pruned_by_other_targes);
    }
};

class DominancePruningFactory : public TaskPruningFactory {
    std::shared_ptr<DominanceAnalysis> dominance_analysis;
    bool compare_initial_state;
    bool compare_source_state;
    bool compare_other_transitions;
    bool compare_other_targets;
public:
    explicit DominancePruningFactory(std::shared_ptr<DominanceAnalysis> dominance_analysis, bool compare_initial_state, bool compare_source_state, bool compare_other_transitions, bool compare_other_targets)
        : dominance_analysis(std::move(dominance_analysis)),
          compare_initial_state(compare_initial_state),
          compare_source_state(compare_source_state),
          compare_other_transitions(compare_other_transitions),
          compare_other_targets(compare_other_targets)
    {}

    std::unique_ptr<FDRPruningMethod> create_pruning_method(std::shared_ptr<ProbabilisticTask> task) override;

};

}


#endif //DOMINANCE_PRUNING_H
