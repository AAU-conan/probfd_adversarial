#include "downward/cli/plugins/plugin.h"

#include "probfd/pruning/no_pruning.h"

using namespace downward;
using namespace utils;

using namespace probfd;
using namespace probfd::pruning;

using namespace downward::cli::plugins;

namespace {

class NoPruningFactoryFeature
    : public TypedFeature<TaskPruningFactory, NoPruningFactory> {
public:
    NoPruningFactoryFeature() : TypedFeature("no_pruning")
    {
        document_title("No pruning");
        document_synopsis("Always returns false on queries for pruning.");
    }

    [[nodiscard]]
    std::shared_ptr<NoPruningFactory>
    create_component(const Options&, const Context&) const override
    {
        return std::make_shared<NoPruningFactory>();
    }
};

FeaturePlugin<NoPruningFactoryFeature> _plugin;

} // namespace
