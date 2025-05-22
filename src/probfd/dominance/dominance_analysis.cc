#include "probfd/dominance/dominance_analysis.h"

#include "downward/cli/plugins/plugin.h"

namespace probfd::dominance {
    static class DominanceAnalysisCategoryPlugin : public downward::cli::plugins::TypedCategoryPlugin<DominanceAnalysis> {
    public:
        DominanceAnalysisCategoryPlugin() : TypedCategoryPlugin("DominanceAnalysis") {
            document_synopsis(
                    "This page describes the various methods to compute a dominance relation on a factored task."
            );
        }
    } _category_plugin;

}