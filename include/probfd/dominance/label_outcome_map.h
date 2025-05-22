#ifndef LABEL_OUTCOME_MAP_H
#define LABEL_OUTCOME_MAP_H

#include "fts_task.h"

#include <NamedType/named_type.hpp>
namespace probfd::dominance {
    /*
     * This class is used to map labels to label-outcomes in a labelled transition system.
     */
    class LabelOutcomeMap {
        std::vector<LabelOutcome> label_to_first_label_outcome;
        std::vector<size_t> label_to_num_outcomes;
    public:
        explicit LabelOutcomeMap(const FTSTask& fts_task)
        {
            label_to_first_label_outcome.resize(fts_task.get_num_labels());
            label_to_num_outcomes.resize(fts_task.get_num_labels());

            LabelOutcome next_label_outcome(0);
            for (Label label(0); label < fts_task.get_num_labels(); ++label) {
                label_to_first_label_outcome[label] = next_label_outcome;
                label_to_num_outcomes[label] = fts_task.get_num_label_outcomes(label);
                next_label_outcome = LabelOutcome(next_label_outcome + label_to_num_outcomes[label]);
            }
        }

        [[nodiscard]] LabelOutcome get_label_outcome(Label label, size_t outcome) const {
            assert(outcome < label_to_num_outcomes.size());
            return LabelOutcome(static_cast<int>(label_to_first_label_outcome[label] + outcome));
        }

        [[nodiscard]] size_t get_num_label_outcomes(Label label) const {
            return label_to_num_outcomes[label];
        }

        [[nodiscard]] auto get_label_outcomes(Label label) const {
            return std::views::iota(0ul, label_to_num_outcomes[label]) | std::views::transform([this, label](size_t i) {
                return get_label_outcome(label, i);
            });
        }
    };
}
#endif //LABEL_OUTCOME_MAP_H
