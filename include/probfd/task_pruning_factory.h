#ifndef TASK_PRUNING_FACTORY_H
#define TASK_PRUNING_FACTORY_H

#include "probfd/fdr_types.h"

#include <memory>

namespace probfd {
class ProbabilisticTask;
}

namespace probfd {

class TaskPruningFactory {
public:
    virtual ~TaskPruningFactory() = default;

    virtual std::unique_ptr<FDRPruningMethod> create_pruning_method(
        std::shared_ptr<ProbabilisticTask> task) = 0;
};

} // namespace probfd

#endif //TASK_PRUNING_FACTORY_H
