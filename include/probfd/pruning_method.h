#ifndef PRUNING_METHOD_H
#define PRUNING_METHOD_H

#include "probfd/aliases.h"
#include "state_space.h"

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
public:
    virtual ~PruningMethod() = default;

    /**
     * @brief Checks whether a transition can be pruned based on the source state,
     * action, and target state.
     */
     virtual bool can_prune_transition(
                StateSpace<State, Action>& state_space,
                ParamType<State> source_state,
                const TransitionTail<Action>& transition_tail) const = 0;

    /**
     * @brief Prints statistics, e.g. the number of queries made to the
     * interface.
     */
    virtual void print_statistics() const {}
};

} // namespace probfd

#endif //PRUNING_METHOD_H
