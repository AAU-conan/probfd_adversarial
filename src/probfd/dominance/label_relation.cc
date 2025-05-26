#include "probfd/dominance/label_relation.h"

#include "probfd/dominance/factor_dominance_relation.h"
#include "probfd/dominance/labelled_transition_system.h"
#include "probfd/dominance/fts_task.h"
#include "probfd/dominance/label_map.h"
#include "downward/cli/plugins/plugin.h"

#include <format>

using namespace std;

namespace probfd::dominance {
    LabelOutcomeRelation::LabelOutcomeRelation(int num_label_outcomes) : num_label_outcomes(num_label_outcomes) { }

    void LabelOutcomeRelation::dump(std::ostream& os, const FTSTask& fts_task, const LabelOutcomeMap& label_outcome_map) const {


    }


    static class LabelOutcomeRelationFactoryPlugin final : public downward::cli::plugins::TypedCategoryPlugin<LabelRelationFactory> {
    public:
        LabelOutcomeRelationFactoryPlugin() : TypedCategoryPlugin("LabelOutcomeRelationFactory") {
            document_synopsis( "A LabelOutcomeRelationFactory creates LabelOutcomeRelations for a given FTS task.");
        }
    }
    _category_plugin;
}