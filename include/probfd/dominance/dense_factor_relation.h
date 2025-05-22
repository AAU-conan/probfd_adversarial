#ifndef DOMINANCE_DENSE_FACTOR_RELATION_H
#define DOMINANCE_DENSE_FACTOR_RELATION_H

#include <vector>

#include "factor_dominance_relation.h"
#include "probfd/dominance/strong_types.h"

namespace probfd::dominance {
    /**
     * DenseLocalStateRelation represents the simulation relation between states in a single LTS. It is implemented as a
     * dense matrix. An N x N matrix for the N states in the LTS, representing when one state simulates another.
     *
     * The relation must be a superset of the identity relation.
     */
    class DenseFactorRelation final : public FactorDominanceRelation {
    protected:
        // Relations between states. relation[s.get()][t.get()] is true if s simulates t.
        std::vector<std::vector<bool> > relation;

    public:
        explicit DenseFactorRelation(const LabelledTransitionSystem& lts);

        void remove(State s, State t) {
            relation[s.get()][t.get()] = false;
        }

        [[nodiscard]] bool simulates(State s, State t) const override {
            return !relation.empty() ? relation[s.get()][t.get()] : s == t;
        }

        [[nodiscard]] bool similar(State s, State t) const override {
            return !relation.empty() ?
                   relation[s.get()][t.get()] && relation[t.get()][s.get()] :
                   s == t;
        }

        inline const std::vector<std::vector<bool> > &get_relation() {
            return relation;
        }

        bool apply_to_simulations_until(std::function<bool(State s, State t)> &&f) const override;
        bool remove_simulations_if(std::function<bool(State s, State t)> &&f) override;
    };
}

#endif
