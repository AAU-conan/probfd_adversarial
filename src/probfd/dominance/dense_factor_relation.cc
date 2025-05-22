#include "probfd/dominance/dense_factor_relation.h"

#include "downward/cli/plugins/plugin.h"
#include "probfd/dominance/labelled_transition_system.h"

namespace probfd::dominance {
 bool DenseFactorRelation::apply_to_simulations_until(std::function<bool(State s, State t)>&& f) const {
        for (State s(0); s < relation.size(); ++s) {
            for (State t(0); t < relation.size(); ++t) {
                if (simulates(s, t) && f(s, t)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool DenseFactorRelation::remove_simulations_if(std::function<bool(State s, State t)>&& f) {
        bool any = false;
        for (State s(0); s < relation.size(); ++s) {
            for (State t(0); t < relation.size(); ++t) {
                if (s != t && simulates(s, t) && f(s, t)) {
                    relation[s.get()][t.get()] = false;
                    any = true;
                }
            }
        }
        return any;
    }


    DenseFactorRelation::DenseFactorRelation(const LabelledTransitionSystem& lts) : FactorDominanceRelation(lts.size()),
                                                                                         relation(lts.size(), std::vector<bool>(lts.size(), true)) {
        int num_states = lts.size();
        const std::vector<bool> &goal_states = lts.get_goal_states();
        const std::vector<int> &goal_distances = lts.get_goal_distances();
        relation.resize(num_states);
        for (int i = 0; i < num_states; i++) {
            relation[i].resize(num_states, true);
            if (!goal_states[i]) {
                for (int j = 0; j < num_states; j++) {
                    //TODO (efficiency): initialize with goal distances
                    if (goal_states[j] /*|| goal_distances[i] > goal_distances[j]*/) {
                        relation[i][j] = false;
                    }
                }
            }
        }
    }


    using DenseLocalStateRelationFactory = FactorDominanceRelationFactoryImpl<DenseFactorRelation>;
    class DenseLocalStateRelationFactoryFeature final : public downward::cli::plugins::TypedFeature<FactorDominanceRelationFactory, DenseLocalStateRelationFactory> {
    public:
        DenseLocalStateRelationFactoryFeature() : TypedFeature("dense_fdr") {
            document_title("Dense Factor Dominance Relation");
            document_synopsis("Stores the simulation relation between states in a dense matrix");
        }

        std::shared_ptr<DenseLocalStateRelationFactory> create_component(
            const downward::cli::plugins::Options &/*opts*/, const downward::utils::Context& context) const override {
            return downward::cli::plugins::make_shared_from_arg_tuples<DenseLocalStateRelationFactory>();
        }
    };
    static downward::cli::plugins::FeaturePlugin<DenseLocalStateRelationFactoryFeature> _dense_plugin;
}