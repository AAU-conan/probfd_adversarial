#include "downward/cli/plugins/plugin.h"

#include "probfd/task_pruning_factory.h"

using namespace probfd;

using namespace downward::cli::plugins;

namespace {

class TaskEvaluatorFactoryCategoryPlugin
    : public TypedCategoryPlugin<TaskPruningFactory> {
public:
    TaskEvaluatorFactoryCategoryPlugin()
        : TypedCategoryPlugin("TaskPruningFactory")
    {
        allow_variable_binding();
    }
} _category_plugin;

} // namespace
