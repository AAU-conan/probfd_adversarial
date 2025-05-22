#ifndef FTS_FACT_NAMES_H
#define FTS_FACT_NAMES_H

#include "probfd/dominance/strong_types.h"

#include <memory>
#include <string>
#include <boost/bimap.hpp>

namespace downward {
    class AbstractTask;
    struct FactPair;
}

using FactPair = downward::FactPair;

namespace probfd::dominance {

class FactNames {
public:
    virtual ~FactNames() = default;
    [[nodiscard]] virtual std::string get_operator_name(int index) const = 0;
    [[nodiscard]] virtual std::string get_variable_name(int var) const = 0;
    [[nodiscard]] virtual std::string get_fact_name(const FactPair& fact_pair) const = 0;
    [[nodiscard]] virtual std::unique_ptr<FactNames> clone() const = 0;
    [[nodiscard]] virtual int get_num_operators() const = 0;
    [[nodiscard]] virtual size_t get_num_variables() const = 0;
};

    class NoFactNames final : public FactNames {
    public:
        NoFactNames();

        [[nodiscard]] std::string get_operator_name(int index) const override;

        [[nodiscard]] std::string get_variable_name(int variable) const override;

        [[nodiscard]] std::string get_fact_name(const FactPair& fact_pair) const override;

        [[nodiscard]] std::unique_ptr<FactNames> clone() const override;

        [[nodiscard]] int get_num_operators() const override;

        [[nodiscard]] size_t get_num_variables() const override;
    };


    class AbstractTaskFactNames final : public FactNames {
        std::shared_ptr<downward::AbstractTask> abstract_task;

    public:
        AbstractTaskFactNames() = default;
        explicit AbstractTaskFactNames(const std::shared_ptr<downward::AbstractTask>& abstract_task)
            : abstract_task(abstract_task) {


        }

        [[nodiscard]] std::string get_operator_name(const int index) const override;

        [[nodiscard]] std::string get_variable_name(const int variable) const override;

        [[nodiscard]] std::string get_fact_name(const FactPair& fact_pair) const override;

        [[nodiscard]] std::unique_ptr<FactNames> clone() const override;

        [[nodiscard]] int get_num_operators() const override;

        [[nodiscard]] size_t get_num_variables() const override;
    };

    class FactValueNames {
    protected:
        std::shared_ptr<FactNames> fact_names;
        int variable;

    public:
        FactValueNames(const std::shared_ptr<FactNames>& fact_names, int variable);

        [[nodiscard]] std::string get_fact_value_name(const int value) const;

        [[nodiscard]] std::string get_operator_name(const int index) const;

        [[nodiscard]] std::string get_common_operators_name(const std::vector<Label>& ops) const;;
    };

    std::unique_ptr<FactValueNames> get_debug_or_release_fact_value_names(
        const std::shared_ptr<FactNames>& fact_names, int variable);

} // fts

#endif //FACT_NAMES_H
