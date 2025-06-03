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
    }

    [[nodiscard]]
    std::shared_ptr<DominancePruningFactory>
    create_component(const Options& opts, const Context&) const override
    {
        return std::make_shared<DominancePruningFactory>(
            opts.get<std::shared_ptr<DominanceAnalysis>>("dominance_analysis")
        );
    }
};

FeaturePlugin<DominancePruningFactoryFeature> _plugin;

} // namespace
