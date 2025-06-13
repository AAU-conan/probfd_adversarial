#include "probfd/transition_caching_task_state_space.h"


namespace probfd {

std::vector<
    StateSpace<downward::State, downward::OperatorID>::TransitionTailType>&
TransitionCachingTaskStateSpace::lookup(
    const downward::State& state,
    PruningType& pruning)
{
    CacheEntry& entry = cache_[state];
    if (!entry.is_initialized) {
        TaskStateSpace::generate_all_transitions(
            state,
            pruning,
            entry.transition_tails);
        entry.is_initialized = true;
    }
    return entry.transition_tails;
}

void TransitionCachingTaskStateSpace::generate_all_transitions(
    const downward::State& state,
    PruningType& pruning,
    std::vector<TransitionTailType>& transitions)
{
    std::vector<TransitionTailType>& cached_transitions = lookup(state, pruning);
    std::ranges::copy(cached_transitions, std::back_inserter(transitions));
}
}