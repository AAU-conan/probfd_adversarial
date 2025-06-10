#ifndef PRUNING_METHOD_H
#define PRUNING_METHOD_H

#include "distribution.h"
#include "probfd/aliases.h"
#include "state_space.h"

#include <print>

namespace probfd {

template <typename State, typename Action>
class MDP;
template <typename Action>
struct TransitionTail;

/**
 * @brief The interface representing a pruning method.
 *
 * @tparam State - The type of states for which the heuristic is evaluated.
 */
template <typename State, typename Action>
class PruningMethod {
protected:
    size_t num_transitions_pruned = 0;
    size_t num_outcomes_pruned = 0;
public:
    virtual ~PruningMethod() = default;

    /**
     * @brief Checks whether a transition can be pruned based on the source state,
     * action, and target state.
     */
     virtual void prune_transitions(
        StateSpace<State, Action>& state_space,
        ParamType<State> source_state,
        std::vector<TransitionTail<Action>>& transition_tails) = 0;

    /**
     * @brief Checks whether a target state can be pruned based on other target states.
     */
    virtual void prune_distribution(
         StateSpace<State, Action>& state_space,
         Distribution<StateID>& distribution) = 0;

    /**
     * @brief Prints statistics, e.g. the number of queries made to the
     * interface.
     */
    virtual void print_statistics() const
    {
        std::println("  Pruned transition(s): {}", num_transitions_pruned);
        std::println("  Pruned outcome(s): {}", num_outcomes_pruned);
    }
};

} // namespace probfd

#endif //PRUNING_METHOD_H
