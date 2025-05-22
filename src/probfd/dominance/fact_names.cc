#include "probfd/dominance/fact_names.h"

#include "downward/abstract_task.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <format>
#include <ranges>

namespace probfd::dominance {
    NoFactNames::NoFactNames() = default;

    std::string NoFactNames::get_operator_name(int index) const {
        return std::format("Operator-{}", index);
    }

    std::string NoFactNames::get_variable_name(int variable) const {
        return std::format("Var-{}", variable);
    }

    std::string NoFactNames::get_fact_name(const FactPair& fact_pair) const {
        return std::format("Fact-{}-{}", fact_pair.var, fact_pair.value);
    }

    std::unique_ptr<FactNames> NoFactNames::clone() const {
        return std::make_unique<NoFactNames>(*this);
    }

    int NoFactNames::get_num_operators() const {
        throw std::logic_error("NoFactNames cannot be used to get the number of operators");
    }

    size_t NoFactNames::get_num_variables() const {
        throw std::logic_error("NoFactNames cannot be used to get the number of variables");
    }

    std::string AbstractTaskFactNames::get_operator_name(const int index) const {
        return abstract_task->get_operator_name(index);
    }

    std::string AbstractTaskFactNames::get_variable_name(const int variable) const {
        return abstract_task->get_variable_name(variable);
    }

    std::string AbstractTaskFactNames::get_fact_name(const FactPair& fact_pair) const {
        return abstract_task->get_fact_name(fact_pair);
    }

    std::unique_ptr<FactNames> AbstractTaskFactNames::clone() const {
        return std::make_unique<AbstractTaskFactNames>(*this);
    }

    int AbstractTaskFactNames::get_num_operators() const {
        return abstract_task->get_num_operators();
    }

    size_t AbstractTaskFactNames::get_num_variables() const {
        return abstract_task->get_num_variables();
    }

    FactValueNames::FactValueNames(const std::shared_ptr<FactNames>& fact_names, int variable): fact_names(fact_names), variable(variable) { }

    std::string FactValueNames::get_fact_value_name(const int value) const {
        return fact_names->get_fact_name(FactPair(variable, value));
    }

    std::string FactValueNames::get_operator_name(const int index) const {
        return fact_names->get_operator_name(index);
    }

    std::string FactValueNames::get_common_operators_name(const std::vector<Label>& ops) const {
        return boost::join(std::ranges::to<std::vector<std::string>>(std::views::transform(ops, [&](int op) { return get_operator_name(op); })), " | ");
    }

    std::unique_ptr<FactValueNames> get_debug_or_release_fact_value_names(const std::shared_ptr<FactNames>& fact_names, int variable) {
        return std::make_unique<FactValueNames>(fact_names, variable);
    }

} // fts