#ifndef MANUAL_FACTOR_DOMINANCE_RELATION_H
#define MANUAL_FACTOR_DOMINANCE_RELATION_H


#include "probfd/dominance/factor_dominance_relation.h"
#include "probfd/dominance/strong_types.h"
#include "probfd/dominance/all_none_factor_index.h"

#include <vector>

namespace probfd::dominance {
    class LabelOutcomeMap;

    class ManualFactorDominanceRelation final : public FactorDominanceRelation {
        std::vector<std::vector<bool>> relation;
    public:
        explicit ManualFactorDominanceRelation(int num_states)
            : FactorDominanceRelation(num_states)
        {
            relation.resize(num_states, std::vector<bool>(num_states, false));
        }

        [[nodiscard]]
        inline bool simulates(State s, State t) const override
        {
            return relation[s][t];
        }

        [[nodiscard]]
        inline bool similar(State s, State t) const override
        {
            return simulates(s, t) && simulates(t, s);
        }

        bool apply_to_simulations_until(
            std::function<bool(State s, State t)>&& f) const override
        {
            throw std::runtime_error(
                "ManualFactorDominanceRelation does not support "
                "apply_to_simulations_until");
        }

        bool remove_simulations_if(
            std::function<bool(State s, State t)>&& f) override
        {
            throw std::runtime_error(
                "ManualFactorDominanceRelation does not support "
                "remove_simulations_if");
        }

        void set_simulates(State s, State t)
        {
            relation[s][t] = true;
        }

        void set_identity()
        {
            for (State i(0); i < num_states; ++i) {
                for (State j(0); j < num_states; ++j) {
                    if (i == j) {
                            relation[i][j] = true;
                    } else {
                            relation[i][j] = false;
                    }
                }
            }
        }
    };
}
#endif //MANUAL_FACTOR_DOMINANCE_RELATION_H
