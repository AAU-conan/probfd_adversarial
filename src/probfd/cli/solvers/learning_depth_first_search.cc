#include "downward/cli/plugins/plugin.h"

#include "probfd/cli/multi_feature_plugin.h"
#include "probfd/cli/naming_conventions.h"

#include "probfd/cli/solvers/mdp_heuristic_search.h"
#include "probfd/cli/solvers/mdp_solver.h"

#include "probfd/algorithms/learning_depth_first_search.h"

#include <memory>
#include <string>
#include <utility>

using namespace downward;
using namespace probfd;
using namespace probfd::algorithms;
using namespace probfd::algorithms::learning_depth_first_search;
using namespace probfd::solvers;

using namespace probfd::cli;
using namespace probfd::cli::solvers;

using namespace downward::cli::plugins;

namespace {

class LDFSSolver : public MDPHeuristicSearch<false, false> {
    const bool backtrack_update_upperbound_;
    const bool upperbound_update_to_qvalue_;
    const bool simple_;
    const std::string name_;
public:
    template <typename... Args>
    LDFSSolver(
        std::string variant_name,
        bool backtrack_update_upperbound,
        bool upperbound_update_to_qvalue_,
        bool simple,
        Args&&... args)
        : MDPHeuristicSearch<false, false>(std::forward<Args>(args)...)
        , name_(std::move(variant_name))
        , backtrack_update_upperbound_(backtrack_update_upperbound)
        , upperbound_update_to_qvalue_(upperbound_update_to_qvalue_)
        , simple_(simple)
    {
    }

    std::string get_heuristic_search_name() const override { return name_; }

    std::unique_ptr<StatisticalMDPAlgorithm> create_algorithm(
        const std::shared_ptr<ProbabilisticTask>& task,
        const std::shared_ptr<FDRCostFunction>& task_cost_function) override
    {
        return std::make_unique<AlgorithmAdaptor>(
            std::make_unique<LearningDepthFirstSearch<downward::State, downward::OperatorID>>(this->convergence_epsilon_, this->tiebreaker_, backtrack_update_upperbound_, upperbound_update_to_qvalue_, simple_));
    }
};

class LDFSSolverFeature : public TypedFeature<TaskSolverFactory, MDPSolver> {
public:
    LDFSSolverFeature()
        : LDFSSolverFeature::TypedFeature("ldfs")
    {
        this->document_title("Learning Depth-First Search");

        this->add_option<bool>("backtrack_upperbound",
            "Whether to update the upper bound during backtracking.",
            "true");

        this->add_option<bool>("upperbound_tightening",
            "Whether to update the upper bound to the Q-value of the greedy policy.",
            "true");

        this->add_option<bool>("simple",
            "Whether to use the simplified version of the algorithm ",
            "false");

        add_base_solver_options_except_algorithm_to_feature(*this);
        add_mdp_hs_options_to_feature<false, false>(*this);
    }

protected:
    std::shared_ptr<MDPSolver>
    create_component(const Options& options, const utils::Context& context)
        const override
    {
        return make_shared_from_arg_tuples<MDPSolver>(
            make_shared_from_arg_tuples<LDFSSolver>(
                "ldfs",
                options.get<bool>("backtrack_upperbound"),
                options.get<bool>("upperbound_tightening"),
                options.get<bool>("simple"),
                get_mdp_hs_args_from_options<false, false>(options)),
            get_base_solver_args_no_algorithm_from_options(options));
    }
};

FeaturePlugin<LDFSSolverFeature> _plugin_ldfs;

} // namespace
