#include "downward/cli/plugins/plugin.h"

#include "probfd/heuristics/determinization_cost_heuristic.h"

#include "downward/tasks/root_task.h"
#include "probfd/dominance/draw_fts.h"
#include "probfd/dominance/fts_task.h"
#include "probfd/merge_and_shrink/factored_transition_system.h"
#include "probfd/merge_and_shrink/fts_factory.h"
#include "probfd/task_proxy.h"
#include "probfd/tasks/determinization_task.h"
#include "probfd/tasks/probabilization_task.h"

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
    }

    std::shared_ptr<DeterminizationCostHeuristicFactory>
    create_component(const Options& options, const Context&) const override
    {
        if (auto det_root_task = dynamic_cast<probfd::tasks::DeterminizationTask*>(downward::tasks::g_root_task.get()); det_root_task) {
        #ifndef NDEBUG
                auto prob_det_task = std::make_shared<probfd::tasks::ProbabilizationTask>(downward::tasks::g_root_task);

                ProbabilisticTaskProxy task_proxy(*prob_det_task);
                // Construct atomic transition systems
                merge_and_shrink::FactoredTransitionSystem fts = merge_and_shrink::create_factored_transition_system(task_proxy, downward::utils::g_log);

                // Construct FTS task
                probfd::dominance::FTSTask fts_task(fts, prob_det_task);

                draw_fts("fts_det.dot", fts_task);
        #endif
        } else {
            throw std::runtime_error("DeterminizationHeuristicFactoryFeature requires the root task to be a DeterminizationTask.");
        }
        return std::make_shared<DeterminizationCostHeuristicFactory>(
            options.get<std::shared_ptr<::Evaluator>>("evaluator"));
    }
};

FeaturePlugin<DeterminizationHeuristicFactoryFeature> _plugin;

} // namespace
