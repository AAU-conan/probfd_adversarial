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

    void LabelOutcomeRelation::dump(const FTSTask& fts_task ) const {
        for (int i = 0; i < static_cast<int>(fts_task.get_factors().size()); ++i) {
            cout << std::format("Factor {}", i) << std::endl;
            for (int l1 = 0; l1 < num_label_outcomes; ++l1) {
//                for (int l2 = 0; l2 < num_label_outcomes; ++l2) {
//                    if (l1 != l2 && label_dominates_label_in_all_other(i, fts_task, l2, l1)) {
//                        cout << std::format("Label {} dominates {} in all other", fts_task.get_factor(i).label_name(l2), fts_task.get_factor(i).label_name(l1)) << std::endl;
//                    }
//                }
//                if (noop_dominates_label_in_all_other(i, fts_task, l1)) {
//                    cout << std::format("NOOP dominates {} in all other", fts_task.get_factor(i).label_name(l1)) << std::endl;
//                }
            }
        }
    }


    static class LabelOutcomeRelationFactoryPlugin final : public downward::cli::plugins::TypedCategoryPlugin<LabelRelationFactory> {
    public:
        LabelOutcomeRelationFactoryPlugin() : TypedCategoryPlugin("LabelOutcomeRelationFactory") {
            document_synopsis( "A LabelOutcomeRelationFactory creates LabelOutcomeRelations for a given FTS task.");
        }
    }
    _category_plugin;
}