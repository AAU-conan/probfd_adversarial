#ifndef DOMINANCE_LOCAL_STATE_RELATION_H
#define DOMINANCE_LOCAL_STATE_RELATION_H

#include "labelled_transition_system.h"
#include "probfd/dominance/strong_types.h"

#include <functional>
#include <memory>

namespace probfd::dominance {
    class LabelledTransitionSystem;

    /**
     * FactorDominanceRelation is abstract and represents the simulation relation between states in a single LTS.
     */
    class FactorDominanceRelation {
    protected:
        size_t num_states;
    public:
        explicit FactorDominanceRelation(int num_states);
        virtual ~FactorDominanceRelation() = default;

        [[nodiscard]] size_t get_num_states() const {
            return num_states;
        }

        [[nodiscard]] virtual inline bool simulates(State s, State t) const = 0;
        [[nodiscard]] virtual inline bool similar(State s, State t) const = 0;

        [[nodiscard]] virtual bool is_identity() const;

        virtual void dump(const LabelledTransitionSystem& lts) const;

        [[nodiscard]] virtual int num_equivalences() const;

        [[nodiscard]] virtual int num_simulations() const;

        [[nodiscard]] virtual int num_different_states() const;

        /**
         * Apply a function to simulations until the function returns true.
         * @return True if the function returns true for any pair of states, false otherwise.
         */
        virtual bool apply_to_simulations_until(std::function<bool(State s, State t)> &&f) const = 0;

        /**
         * Remove simulations for which the function returns true.
         * @return True if any simulation was removed, false otherwise.
         */
        virtual bool remove_simulations_if(std::function<bool(State s, State t)> &&f) = 0;
    };


    /**
     * Constructs the FactorDominanceRelation for a given LTS. Used for options for the plugins.
     */
    class FactorDominanceRelationFactory {
    public:
        virtual ~FactorDominanceRelationFactory() = default;
        virtual std::unique_ptr<FactorDominanceRelation> create(const LabelledTransitionSystem &lts) = 0;
    };

    /*
     * Generates instances of FactorDominanceRelationFactory for subtypes of FactorDominanceRelation.
     */
    template<class DominanceRelation>
    class FactorDominanceRelationFactoryImpl : public FactorDominanceRelationFactory {
        std::unique_ptr<FactorDominanceRelation> create(const LabelledTransitionSystem &lts) override {
            return std::make_unique<DominanceRelation>(lts);
        }
    };



}
#endif
