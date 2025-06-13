#ifndef TRANSITION_CACHING_TASK_STATE_SPACE_H
#define TRANSITION_CACHING_TASK_STATE_SPACE_H

#include "downward/per_state_information.h"
#include "probfd/task_state_space.h"
#include "transition_tail.h"

namespace probfd {

class TransitionCachingTaskStateSpace final : public TaskStateSpace {

    struct CacheEntry {
        bool is_initialized = false;
        std::vector<TransitionTailType> transition_tails;
    };

    downward::PerStateInformation<CacheEntry> cache_;

    std::vector<TransitionTailType>&
    lookup(const downward::State& state, PruningType& pruning);

public:
    TransitionCachingTaskStateSpace(
        const std::shared_ptr<ProbabilisticTask>& task,
        const std::vector<std::shared_ptr<downward::Evaluator>>&
            path_dependent_evaluators)
        : TaskStateSpace(std::move(task), std::move(path_dependent_evaluators))
    {
    }

    void generate_all_transitions(
        const downward::State& state,
        PruningType& pruning,
        std::vector<TransitionTailType>& transitions) override;
};

}
#endif //TRANSITION_CACHING_TASK_STATE_SPACE_H
