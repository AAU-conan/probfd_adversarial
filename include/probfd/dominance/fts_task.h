#ifndef FTS_FTS_TASK_H
#define FTS_FTS_TASK_H

#include <vector>
#include <memory>

#include "downward/abstract_task.h"
#include "labelled_transition_system.h"
#include "probfd/probabilistic_task.h"

namespace probfd {
class ProbabilisticTask;
}

namespace probfd::merge_and_shrink {
    class FactoredTransitionSystem;
}

using FactPair = downward::FactPair;

namespace probfd::dominance {
    class FactNames;
    // This is very similar to the merge_and_shrink::FactoredTransitionSystem
    // However, the idea is that here, we have a "clean" unmutable version of the representation.
    // Specifically, we do not have "non-active" labels, or transition_systems, so all IDs are set from 0 to n-1.
    // Also, we do not have dead labels, unreachable or dead end states
    //
    // Also, we use a representation of transition systems with some redundant access to the transitions.
    // This adds some memory overhead as well as some overhead when copying the transition systems, but
    // it allows for faster access to the transitions from/to certain states.
    // Preliminary experiments showed that this can pay off for the computation of dominance relations

    class FTSTask final : public downward::AbstractTask {
        // The abstract task that was used to generate this task. This is optional.
        // For now just used to preserve fact and action names, whenever they match
        std::shared_ptr<FactNames> fact_names;

        std::vector<int> label_costs;
        std::vector<int> label_outcomes;
        std::vector<std::unique_ptr<LabelledTransitionSystem>> transition_systems;


    public:
        explicit FTSTask(
            const merge_and_shrink::FactoredTransitionSystem& fts,
            const std::optional<std::shared_ptr<ProbabilisticTask>>& parent =
                std::nullopt);
        explicit FTSTask(
            const std::vector<LabelledTransitionSystem>& ltss,
            std::vector<int> label_costs,
            std::vector<int> label_outcomes_,
            const std::optional<std::shared_ptr<ProbabilisticTask>>& parent);


        int get_num_labels() const;

        int get_num_label_outcomes(Label label) const;

        int get_label_cost(Label label) const;

        const LabelledTransitionSystem &get_factor(FactorIndex i) const;

        int get_num_variables() const override;

        std::string get_variable_name(int var) const override;

        int get_variable_domain_size(int var) const override;

        int get_variable_axiom_layer(int var) const override;

        int get_variable_default_axiom_value(int var) const override;

        std::string get_fact_name(const FactPair &fact) const override;

        int get_num_operators() const override;

        int get_num_axioms() const override;

        int get_num_goals() const override;

        FactPair get_goal_fact(int index) const override;

        std::vector<int> get_initial_state_values() const override;

        const std::vector<std::unique_ptr<LabelledTransitionSystem>>& get_factors() const;
        std::string get_axiom_name(int index) const override;
        int get_num_axiom_preconditions(int index) const override;
        FactPair
        get_axiom_precondition(int op_index, int fact_index) const override;
        int get_num_axiom_effects(int op_index) const override;
        int get_num_axiom_effect_conditions(int op_index, int eff_index)
            const override;
        FactPair
        get_axiom_effect_condition(int op_index, int eff_index, int cond_index)
            const override;
        FactPair get_axiom_effect(int op_index, int eff_index) const override;
        std::string get_operator_name(int index) const override;
        int get_num_operator_preconditions(int index) const override;
        FactPair
        get_operator_precondition(int op_index, int fact_index) const override;
        int get_operator_cost(int index) const override;
        int get_num_operator_effects(int op_index) const override;
        int get_num_operator_effect_conditions(int op_index, int eff_index)
            const override;
        FactPair get_operator_effect_condition(
            int op_index,
            int eff_index,
            int cond_index) const override;
        FactPair
        get_operator_effect(int op_index, int eff_index) const override;
    };


    // A wrapper for the FTSTask that implements the ProbabilisticTask interface.
    class FTSTaskWrapper final : public ProbabilisticTask {
    private:
        std::shared_ptr<FTSTask> fts_task;

        struct LabelOperator {
            std::vector<FactPair> preconditions;
            std::vector<std::vector<FactPair>> outcomes;
        };

        std::vector<LabelOperator> label_operators;
        std::vector<FactPair> goals;

    public:
        explicit FTSTaskWrapper(std::shared_ptr<FTSTask> fts_task);

        int get_num_variables() const override;
        std::string get_variable_name(int var) const override;
        int get_variable_domain_size(int var) const override;
        int get_variable_axiom_layer(int var) const override;
        int get_variable_default_axiom_value(int var) const override;
        std::string get_fact_name(const FactPair& fact) const override;
        int get_num_axioms() const override;
        std::string get_axiom_name(int index) const override;
        int get_num_axiom_preconditions(int index) const override;
        FactPair
        get_axiom_precondition(int op_index, int fact_index) const override;
        int get_num_axiom_effects(int op_index) const override;
        int get_num_axiom_effect_conditions(int op_index, int eff_index)
            const override;
        FactPair
        get_axiom_effect_condition(int op_index, int eff_index, int cond_index)
            const override;
        FactPair get_axiom_effect(int op_index, int eff_index) const override;
        std::string get_operator_name(int index) const override;
        int get_num_operators() const override;
        int get_num_operator_preconditions(int index) const override;
        FactPair
        get_operator_precondition(int op_index, int fact_index) const override;
        int get_num_goals() const override;
        FactPair get_goal_fact(int index) const override;
        std::vector<int> get_initial_state_values() const override;
        value_t get_goal_termination_cost() const override;
        value_t get_non_goal_termination_cost() const override;
        value_t get_operator_cost(int op_index) const override;
        int get_num_operator_outcomes(int op_index) const override;
        value_t
        get_operator_outcome_probability(int op_index, int outcome_index)
            const override;
        int
        get_operator_outcome_id(int op_index, int outcome_index) const override;
        int get_num_operator_outcome_effects(int op_index, int outcome_index)
            const override;
        downward::FactPair get_operator_outcome_effect(
            int op_index,
            int outcome_index,
            int eff_index) const override;
        int get_num_operator_outcome_effect_conditions(
            int op_index,
            int outcome_index,
            int eff_index) const override;
        downward::FactPair get_operator_outcome_effect_condition(
            int op_index,
            int outcome_index,
            int eff_index,
            int cond_index) const override;
    };
}

#endif
