#include "downward/cli/plugins/plugin.h"

#include "probfd/heuristics/determinization_cost_heuristic.h"

#include "downward/tasks/root_task.h"
#include "probfd/tasks/determinization_task.h"

using namespace downward;
using namespace utils;

using namespace probfd;
using namespace probfd::heuristics;

using namespace downward::cli::plugins;

namespace {

class DeterminizationHeuristicFactoryFeature
    : public TypedFeature<
          TaskHeuristicFactory,
          DeterminizationCostHeuristicFactory> {
public:
    DeterminizationHeuristicFactoryFeature()
        : TypedFeature("det")
    {
        document_title("Determinization-based Heuristic");
        document_synopsis("This heuristic returns the estimate of a classical "
                          "planning heuristic evaluated on the all-outcomes "
                          "determinization of the planning task.");

        add_option<std::shared_ptr<::Evaluator>>(
            "evaluator",
            "The classical planning heuristic.");

        add_option<probfd::tasks::DeterminizationTask::OutcomeType>(
            "outcomes",
            "Which outcomes to have in the determinization task.",
            "last_outcome");
    }

    std::shared_ptr<DeterminizationCostHeuristicFactory>
    create_component(const Options& options, const Context&) const override
    {
        auto desired_outcomes = options.get<probfd::tasks::DeterminizationTask::OutcomeType>("outcomes");
        std::shared_ptr<AbstractTask> task = nullptr;
        if (auto det_root_task = dynamic_cast<probfd::tasks::DeterminizationTask*>(downward::tasks::g_root_task.get()); det_root_task) {
            if (det_root_task->outcome_determinization != desired_outcomes) {
                downward::tasks::g_root_task = std::make_shared<probfd::tasks::DeterminizationTask>(det_root_task->get_parent_task(), desired_outcomes);
            }
        } else {
            throw std::runtime_error("DeterminizationHeuristicFactoryFeature requires the root task to be a DeterminizationTask.");
        }
        return std::make_shared<DeterminizationCostHeuristicFactory>(
            options.get<std::shared_ptr<::Evaluator>>("evaluator"));
    }
};

FeaturePlugin<DeterminizationHeuristicFactoryFeature> _plugin;

} // namespace
