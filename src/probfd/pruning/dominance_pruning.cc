
#include "probfd/pruning/dominance_pruning.h"

#include "probfd/dominance/fts_task.h"

#include "probfd/dominance/draw_fts.h"
#include "probfd/dominance/label_relation.h"
#include "probfd/merge_and_shrink/factored_transition_system.h"
#include "probfd/merge_and_shrink/fts_factory.h"
#include "probfd/task_proxy.h"

namespace probfd::pruning {
    std::unique_ptr<FDRPruningMethod> DominancePruningFactory::create_pruning_method(std::shared_ptr<ProbabilisticTask> task)
    {
        ProbabilisticTaskProxy task_proxy(*task);
        // Construct atomic transition systems
        merge_and_shrink::FactoredTransitionSystem fts = merge_and_shrink::create_factored_transition_system(task_proxy, downward::utils::g_log);

        // Construct FTS task
        FTSTask fts_task(fts, task);

#ifndef NDEBUG
        draw_fts("fts.dot", fts_task);
#endif

        // Do dominance analysis
        std::shared_ptr state_dominance_relation = dominance_analysis->compute_dominance_relation(fts_task);

        return std::make_unique<DominancePruning>(state_dominance_relation,
            compare_initial_state,
            compare_source_state,
            compare_other_transitions,
            compare_other_targets);
    }
}