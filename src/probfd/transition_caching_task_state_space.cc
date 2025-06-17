#include "probfd/transition_caching_task_state_space.h"


namespace probfd {

std::vector<
    StateSpace<downward::State, downward::OperatorID>::TransitionTailType>&
TransitionCachingTaskStateSpace::lookup(
    const downward::State& state,
    PruningType& pruning)
{
    if (!cache_.contains(state.get_id())) {
        if (cache_.size() >= max_cache_size) {
            // If the cache is full, we need to clear it
            cache_.clear();
        }
        TaskStateSpace::generate_all_transitions(
            state,
            pruning,
            cache_[state.get_id()]);
    }
    return cache_.at(state.get_id());
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