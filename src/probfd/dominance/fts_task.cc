#include "probfd/dominance/fts_task.h"

#include <cassert>

#include "probfd/dominance/fact_names.h"
#include "probfd/dominance/labelled_transition_system.h"
#include "probfd/merge_and_shrink/factored_transition_system.h"
#include "probfd/merge_and_shrink/labels.h"
#include "probfd/dominance/label_map.h"

namespace probfd::dominance {

    FTSTask::FTSTask(const merge_and_shrink::FactoredTransitionSystem &fts, const std::optional<std::shared_ptr<ProbabilisticTask>>& parent)
    : fact_names(parent.has_value()? static_cast<std::shared_ptr<FactNames>>(std::make_shared<ProbabilisticTaskFactNames>(parent.value())): std::make_shared<NoFactNames>()) {

        LabelMap label_map (fts.get_labels());
        for (const auto & ts : fts) {
            transition_systems.push_back(std::make_unique<LabelledTransitionSystem>(fts.get_transition_system(ts), label_map, get_debug_or_release_fact_value_names(fact_names, ts)));
        }

        label_costs.resize(label_map.get_num_labels());
        label_outcomes.resize(label_map.get_num_labels());
        for (int i = 0; i < label_costs.size(); ++i) {
            label_costs[i] = fts.get_labels().get_label_cost(label_map.get_old_id(i));
            label_outcomes[i] = fts.get_labels().get_label_probabilities(label_map.get_old_id(i)).size();
        }
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

    FactPair FTSTask::get_operator_effect(int, int) const {
        ABORT("Accessing operator_effect of an FTSTask");
    }
}