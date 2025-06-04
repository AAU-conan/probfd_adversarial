#include "probfd/dominance/fts_task.h"

#include <cassert>

#include "probfd/dominance/fact_names.h"
#include "probfd/dominance/labelled_transition_system.h"
#include "probfd/merge_and_shrink/factored_transition_system.h"
#include "probfd/merge_and_shrink/labels.h"
#include "probfd/dominance/label_map.h"

namespace probfd::dominance {

    FTSTask::FTSTask(
    const merge_and_shrink::FactoredTransitionSystem& fts,
    const std::optional<std::shared_ptr<ProbabilisticTask>>& parent)
    : fact_names(
          parent.has_value() ? static_cast<std::shared_ptr<FactNames>>(
                                   std::make_shared<ProbabilisticTaskFactNames>(
                                       parent.value()))
                             : std::make_shared<NoFactNames>())
{

    LabelMap label_map(fts.get_labels());
    for (const auto& ts : fts) {
        transition_systems.push_back(
            std::make_unique<LabelledTransitionSystem>(
                fts.get_transition_system(ts),
                label_map,
                get_debug_or_release_fact_value_names(fact_names, ts)));
    }

    label_costs.resize(label_map.get_num_labels());
    label_outcomes.resize(label_map.get_num_labels());
    for (int i = 0; i < label_costs.size(); ++i) {
        label_costs[i] =
            fts.get_labels().get_label_cost(label_map.get_old_id(i));
        label_outcomes[i] =
            fts.get_labels()
                .get_label_probabilities(label_map.get_old_id(i))
                .size();
    }
}

    FTSTask::FTSTask(
        const std::vector<LabelledTransitionSystem>& ltss, std::vector<int> label_costs, std::vector<int> label_outcomes_,
        const std::optional<std::shared_ptr<ProbabilisticTask>>& parent)
        : transition_systems(), label_costs(std::move(label_costs)), label_outcomes(std::move(label_outcomes_)), fact_names(
              parent.has_value() ? static_cast<std::shared_ptr<FactNames>>(
                                       std::make_shared<ProbabilisticTaskFactNames>(
                                       parent.value()))
                                 : std::make_shared<NoFactNames>())
    {
        for (const auto& lts : ltss) {
            transition_systems.push_back(std::make_unique<LabelledTransitionSystem>(lts));
        }

#ifndef NDEBUG
        // Verify that all transitions have the correct number of outcomes
        for (const auto& lts : transition_systems) {
            for (const auto& tr : lts->get_transitions()) {
                for (const auto& label : lts->get_labels(tr.label_group)) {
                    assert(tr.targets.size() == label_outcomes[label]);
                }
            }
        }
#endif
    }

    int FTSTask::get_num_labels() const
    {
        return label_costs.size();
    }

    int FTSTask::get_num_label_outcomes(Label label) const
    {
        return label_outcomes.at(label);
    }

    int FTSTask::get_label_cost(Label label) const {
        assert ((size_t)label < label_costs.size());
        return label_costs[label];
    }

    const LabelledTransitionSystem &FTSTask::get_factor(FactorIndex factor) const {
        assert ((size_t)factor < transition_systems.size());
        return *(transition_systems[factor]);
    }

    int FTSTask::get_num_variables() const {
        return transition_systems.size();
    }

    std::string FTSTask::get_variable_name(int var) const {
        return fact_names->get_variable_name(var);
    }

    int FTSTask::get_variable_domain_size(int var) const {
        return transition_systems[var]->size();
    }

    int FTSTask::get_variable_axiom_layer(int ) const {
        return -1; // We do not consider derived variables on FTSTasks
    }

    int FTSTask::get_variable_default_axiom_value(int /*var*/) const {
        ABORT("Accessing axioms of an FTSTask");    }

    std::string FTSTask::get_fact_name(const FactPair &fact) const {
        return fact_names->get_fact_name(fact);
    }

    int FTSTask::get_num_operators() const {
        return label_costs.size();
    }

    int FTSTask::get_num_axioms() const {
        return 0;
    }

    int FTSTask::get_num_goals() const {
        ABORT("Accessing num_goals of an FTSTask");
    }

    FactPair FTSTask::get_goal_fact(int ) const {
        ABORT("Accessing goal of an FTSTask");
    }

    std::vector<int> FTSTask::get_initial_state_values() const {
        std::vector<int> initial_state_values;
        for (const auto & factor : transition_systems) {
            initial_state_values.push_back(factor->get_initial_state());
        }
        return initial_state_values;
    }

    const std::vector<std::unique_ptr<LabelledTransitionSystem>> &FTSTask::get_factors() const {
        return transition_systems;
    }

    std::string FTSTask::get_axiom_name(int) const {
        ABORT("Accessing axiom_name of an FTSTask");
    }

    int FTSTask::get_num_axiom_preconditions(int) const {
        ABORT("Accessing num_axiom_preconditions of an FTSTask");
    }

    FactPair FTSTask::get_axiom_precondition(int, int) const {
        ABORT("Accessing axiom_precondition of an FTSTask");
    }

    int FTSTask::get_num_axiom_effects(int) const {
        ABORT("Accessing num_axiom_effects of an FTSTask");
    }

    int FTSTask::get_num_axiom_effect_conditions(int, int) const {
        ABORT("Accessing num_axiom_effect_conditions of an FTSTask");
    }

    FactPair FTSTask::get_axiom_effect_condition(int, int, int) const {
        ABORT("Accessing axiom_effect_condition of an FTSTask");
    }

    FactPair FTSTask::get_axiom_effect(int, int) const {
        ABORT("Accessing axiom_effect of an FTSTask");
    }

    std::string FTSTask::get_operator_name(int) const {
        ABORT("Accessing operator_name of an FTSTask");
    }

    int FTSTask::get_num_operator_preconditions(int) const {
        ABORT("Accessing num_operator_preconditions of an FTSTask");
    }

    FactPair FTSTask::get_operator_precondition(int, int) const {
        ABORT("Accessing operator_precondition of an FTSTask");
    }

    int FTSTask::get_operator_cost(int) const {
        ABORT("Accessing operator_cost of an FTSTask");
    }

    int FTSTask::get_num_operator_effects(int) const {
        ABORT("Accessing num_operator_effects of an FTSTask");
    }

    int FTSTask::get_num_operator_effect_conditions(int, int) const {
        ABORT("Accessing num_operator_effect_conditions of an FTSTask");
    }

    FactPair FTSTask::get_operator_effect_condition(int, int, int) const {
        ABORT("Accessing operator_effect_condition of an FTSTask");
    }

    FactPair FTSTask::get_operator_effect(int, int) const
    {
        ABORT("Accessing operator_effect of an FTSTask");
    }

    FTSTaskWrapper::FTSTaskWrapper(std::shared_ptr<FTSTask> fts_task_)
        : fts_task(std::move(fts_task_))
    {
        for (Label label(0); label < fts_task->get_num_labels(); ++label) {
            LabelOperator label_operator;
            label_operator.outcomes.resize(fts_task->get_num_label_outcomes(label));
            for (FactorIndex i(0); i < fts_task->get_num_variables(); ++i) {
                const LabelledTransitionSystem& lts = fts_task->get_factor(i);
                if (lts.is_relevant_label(label)) {
                    auto trs = lts.get_transitions_label(label);
                    assert(trs.size() == 1);
                    label_operator.preconditions.emplace_back(i, trs[0].src);
                    for (const auto& [o, tgt] : std::views::enumerate(trs[0].targets)) {
                        label_operator.outcomes[o].emplace_back(i, tgt);
                    }
                }
            }
            label_operators.push_back(label_operator);
        }

        // Collect goals
        for (FactorIndex i(0); i < fts_task->get_factors().size(); ++i) {
            bool found_goal = false;
            for (State s(0); s < fts_task->get_factor(i).size(); ++s) {
                if (fts_task->get_factor(i).is_goal(s)) {
                    assert(!found_goal);
                    goals.emplace_back(FactPair(i, s));
                    found_goal = true;
                }
            }
        }
    }

    int FTSTaskWrapper::get_num_variables() const
    {
        return fts_task->get_num_variables();
    }

    std::string FTSTaskWrapper::get_variable_name(int var) const {
        return fts_task->get_variable_name(var);
    }

    int FTSTaskWrapper::get_variable_domain_size(int var) const {
        return fts_task->get_variable_domain_size(var);
    }

    int FTSTaskWrapper::get_variable_axiom_layer(int var) const {
        return fts_task->get_variable_axiom_layer(var);
    }

    int FTSTaskWrapper::get_variable_default_axiom_value(int var) const {
        return fts_task->get_variable_default_axiom_value(var);
    }

    std::string FTSTaskWrapper::get_fact_name(const FactPair& fact) const {
        return fts_task->get_fact_name(fact);
    }

    int FTSTaskWrapper::get_num_axioms() const {
        return fts_task->get_num_axioms();
    }

    std::string FTSTaskWrapper::get_axiom_name(int index) const {
        return fts_task->get_axiom_name(index);
    }

    int FTSTaskWrapper::get_num_axiom_preconditions(int index) const {
        return fts_task->get_num_axiom_preconditions(index);
    }

    FactPair FTSTaskWrapper::get_axiom_precondition(int op_index, int fact_index) const {
        return fts_task->get_axiom_precondition(op_index, fact_index);
    }

    int FTSTaskWrapper::get_num_axiom_effects(int op_index) const {
        return fts_task->get_num_axiom_effects(op_index);
    }

    int FTSTaskWrapper::get_num_axiom_effect_conditions(int op_index, int eff_index) const {
        return fts_task->get_num_axiom_effect_conditions(op_index, eff_index);
    }

    FactPair FTSTaskWrapper::get_axiom_effect_condition(int op_index, int eff_index, int cond_index) const {
        return fts_task->get_axiom_effect_condition(op_index, eff_index, cond_index);
    }

    FactPair FTSTaskWrapper::get_axiom_effect(int op_index, int eff_index) const {
        return fts_task->get_axiom_effect(op_index, eff_index);
    }

    std::string FTSTaskWrapper::get_operator_name(int index) const {
        return fts_task->get_operator_name(index);
    }

    int FTSTaskWrapper::get_num_operators() const {
        return fts_task->get_num_operators();
    }

    int FTSTaskWrapper::get_num_operator_preconditions(int index) const {
        return label_operators[index].preconditions.size();
    }

    FactPair FTSTaskWrapper::get_operator_precondition(int op_index, int fact_index) const {
        return label_operators[op_index].preconditions[fact_index];
    }

    int FTSTaskWrapper::get_num_goals() const {
        return goals.size();
    }

    FactPair FTSTaskWrapper::get_goal_fact(int index) const {
        return goals.at(index);
    }

    std::vector<int> FTSTaskWrapper::get_initial_state_values() const {
        return fts_task->get_initial_state_values();
    }

    value_t FTSTaskWrapper::get_goal_termination_cost() const {
        return 0.0;
    }

    value_t FTSTaskWrapper::get_non_goal_termination_cost() const {
        return INFINITE_VALUE;
    }

    value_t FTSTaskWrapper::get_operator_cost(int op_index) const {
        return fts_task->get_label_cost(Label(op_index));
    }

    int FTSTaskWrapper::get_num_operator_outcomes(int op_index) const {
        return fts_task->get_num_label_outcomes(Label(op_index));
    }

    value_t FTSTaskWrapper::get_operator_outcome_probability(int op_index, int outcome_index) const {
        return -0xFACE; // No probabilities in FTSTASK
    }

    int FTSTaskWrapper::get_operator_outcome_id(int op_index, int outcome_index) const {
        ABORT("Accessing operator_outcome_id of an FTSTaskWrapper");
    }

    int FTSTaskWrapper::get_num_operator_outcome_effects(int op_index, int outcome_index) const {
        return label_operators[op_index].outcomes[outcome_index].size();
    }

    downward::FactPair FTSTaskWrapper::get_operator_outcome_effect(int op_index, int outcome_index, int eff_index) const {
        return label_operators[op_index].outcomes[outcome_index][eff_index];
    }

    int FTSTaskWrapper::get_num_operator_outcome_effect_conditions(int op_index, int outcome_index, int eff_index) const {
        return 0; // No conditions in FTSTask
    }

    downward::FactPair FTSTaskWrapper::get_operator_outcome_effect_condition(int op_index, int outcome_index, int eff_index, int cond_index) const {
        ABORT("Accessing operator_outcome_effect_condition of an FTSTaskWrapper");
    }

} // namespace probfd::dominance
