#ifndef PROBABILIZATION_TASK_H
#define PROBABILIZATION_TASK_H

#include "probfd/probabilistic_task.h"
#include <memory>
#include <vector>

namespace probfd::tasks {

class ProbabilizationTask final : public probfd::ProbabilisticTask {
    std::shared_ptr<downward::AbstractTask> deterministic_task_;

public:
    explicit ProbabilizationTask(std::shared_ptr<downward::AbstractTask> deterministic_task)
        : deterministic_task_(std::move(deterministic_task)) {}

    // Each deterministic operator becomes a probabilistic operator with a single outcome
    value_t get_goal_termination_cost() const override {
        // No extra cost for goal termination
        return 0;
    }

    value_t get_non_goal_termination_cost() const override {
        // No extra cost for non-goal termination
        return 0;
    }

    value_t get_operator_cost(int op_index) const override {
        return deterministic_task_->get_operator_cost(op_index);
    }

    int get_num_operator_outcomes(int op_index) const override {
        // Each operator has exactly one outcome
        return 1;
    }

    value_t get_operator_outcome_probability(int op_index, int outcome_index) const override {
        // Only one outcome, with probability 1.0
        return 1.0;
    }

    int get_operator_outcome_id(int op_index, int outcome_index) const override {
        // Use deterministic operator index as outcome id
        return op_index;
    }

    int get_num_operator_outcome_effects(int op_index, int outcome_index) const override {
        // All effects of the deterministic operator
        return deterministic_task_->get_num_operator_effects(op_index);
    }

    downward::FactPair get_operator_outcome_effect(int op_index, int outcome_index, int eff_index) const override {
        return deterministic_task_->get_operator_effect(op_index, eff_index);
    }

    int get_num_operator_outcome_effect_conditions(int op_index, int outcome_index, int eff_index) const override {
        return deterministic_task_->get_num_operator_effect_conditions(op_index, eff_index);
    }

    downward::FactPair get_operator_outcome_effect_condition(int op_index, int outcome_index, int eff_index, int cond_index) const override {
        return deterministic_task_->get_operator_effect_condition(op_index, eff_index, cond_index);
    }

    // Forward all PlanningTask methods to deterministic_task_
    int get_num_variables() const override {
        return deterministic_task_->get_num_variables();
    }

    std::string get_variable_name(int var) const override {
        return deterministic_task_->get_variable_name(var);
    }

    int get_variable_domain_size(int var) const override {
        return deterministic_task_->get_variable_domain_size(var);
    }

    int get_variable_axiom_layer(int var) const override {
        return deterministic_task_->get_variable_axiom_layer(var);
    }

    int get_variable_default_axiom_value(int var) const override {
        return deterministic_task_->get_variable_default_axiom_value(var);
    }

    std::string get_fact_name(const downward::FactPair& fact) const override {
        return deterministic_task_->get_fact_name(fact);
    }

    int get_num_axioms() const override {
        return deterministic_task_->get_num_axioms();
    }

    std::string get_axiom_name(int index) const override {
        return deterministic_task_->get_axiom_name(index);
    }

    int get_num_axiom_preconditions(int index) const override {
        return deterministic_task_->get_num_axiom_preconditions(index);
    }

    downward::FactPair get_axiom_precondition(int op_index, int fact_index) const override {
        return deterministic_task_->get_axiom_precondition(op_index, fact_index);
    }

    int get_num_axiom_effects(int op_index) const override {
        return deterministic_task_->get_num_axiom_effects(op_index);
    }

    int get_num_axiom_effect_conditions(int op_index, int eff_index) const override {
        return deterministic_task_->get_num_axiom_effect_conditions(op_index, eff_index);
    }

    downward::FactPair get_axiom_effect_condition(int op_index, int eff_index, int cond_index) const override {
        return deterministic_task_->get_axiom_effect_condition(op_index, eff_index, cond_index);
    }

    downward::FactPair get_axiom_effect(int op_index, int eff_index) const override {
        return deterministic_task_->get_axiom_effect(op_index, eff_index);
    }

    std::string get_operator_name(int index) const override {
        return deterministic_task_->get_operator_name(index);
    }

    int get_num_operators() const override {
        return deterministic_task_->get_num_operators();
    }

    int get_num_operator_preconditions(int index) const override {
        return deterministic_task_->get_num_operator_preconditions(index);
    }

    downward::FactPair get_operator_precondition(int op_index, int fact_index) const override {
        return deterministic_task_->get_operator_precondition(op_index, fact_index);
    }

    int get_num_goals() const override {
        return deterministic_task_->get_num_goals();
    }

    downward::FactPair get_goal_fact(int index) const override {
        return deterministic_task_->get_goal_fact(index);
    }

    std::vector<int> get_initial_state_values() const override {
        return deterministic_task_->get_initial_state_values();
    }
};

} // namespace probfd::tasks

#endif //PROBABILIZATION_TASK_H