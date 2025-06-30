#include "downward/cli/plugins/plugin.h"

#include "probfd/pruning/dominance_pruning.h"

using namespace downward;
using namespace utils;

using namespace probfd;
using namespace probfd::pruning;

using namespace downward::cli::plugins;

namespace probfd::pruning {

class DominancePruningFactoryFeature
    : public TypedFeature<TaskPruningFactory, DominancePruningFactory> {
public:
    DominancePruningFactoryFeature() : TypedFeature("dominance_pruning")
    {
        document_title("Dominance Pruning");
        document_synopsis("Prune with dominance analysis.");

        add_option<std::shared_ptr<DominanceAnalysis>>(
            "dominance_analysis",
            "The dominance analysis to be used for pruning.",
            "ld_simulation()");

        add_option<bool>(
            "_init",
            "Prune transitions where the initial state dominates a target state.",
            "true");
        add_option<bool>(
            "source",
            "Prune transitions where the source state dominates a target state.",
            "true");
        add_option<bool>(
            "other",
            "Prune transitions where all target states of another transition dominate a target state.",
            "true");
        add_option<bool>(
            "outcomes",
            "Prune target state that dominate another target state.",
            "true");
    }

    [[nodiscard]]
    std::shared_ptr<DominancePruningFactory>
    create_component(const Options& opts, const Context&) const override
    {
return std::make_shared<DominancePruningFactory>(
            opts.get<std::shared_ptr<DominanceAnalysis>>("dominance_analysis"),
                opts.get<bool>("_init"),
                opts.get<bool>("source"),
                opts.get<bool>("other"),
                opts.get<bool>("outcomes")
        );
    }
};

FeaturePlugin<DominancePruningFactoryFeature> _plugin;

} // namespace
