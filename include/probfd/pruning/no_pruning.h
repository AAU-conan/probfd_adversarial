#ifndef NO_PRUNING_H
#define NO_PRUNING_H

#include "probfd/pruning_method.h"
#include "probfd/transition_tail.h"
#include "probfd/task_pruning_factory.h"

namespace probfd::pruning {

template <typename State, typename Action>
class NoPruningMethod final : public PruningMethod<State, Action> {
public:
    bool can_prune_transition(
        StateSpace<State, Action>& state_space,
        ParamType<State> source_state,
        const TransitionTail<Action>& transition_tail) const override
    {
        return false;
    }
};

class NoPruningFactory : public TaskPruningFactory {
public:
    std::unique_ptr<FDRPruningMethod> create_pruning_method(std::shared_ptr<ProbabilisticTask> task) override
    {
        return std::make_unique<NoPruningMethod<downward::State, downward::OperatorID>>();
    }
};

}


#endif //NO_PRUNING_H
