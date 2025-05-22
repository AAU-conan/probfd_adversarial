#include "probfd/dominance/factor_dominance_relation.h"

#include "downward/cli/plugins/plugin.h"
#include "probfd/dominance/labelled_transition_system.h"

#include <iostream>

using namespace std;

namespace probfd::dominance {
    FactorDominanceRelation::FactorDominanceRelation(int num_states) : num_states(num_states) {}

    [[nodiscard]] bool FactorDominanceRelation::is_identity() const {
        for (State i(0); i < num_states; ++i) {
            for (State j(i.get() + 1); j < num_states; ++j) {
                if (simulates(i, j) || simulates(j, i)) {
                    return false;
                }
            }
        }
        return true;
    }

    void FactorDominanceRelation::dump(const LabelledTransitionSystem& lts) const {
        std::cout << "SIMREL:" << std::endl;
        for (State j(0); j < num_states; ++j) {
            for (State i(0); i < num_states; ++i) {
                if (simulates(j, i) && i != j) {
                    if (simulates(i, j)) {
                        if (j < i) {
                            std::cout<< lts.state_name(i) << " <=> " << lts.state_name(j) << std::endl;
                        }
                    } else {
                        std::cout << lts.state_name(i) << " <= " << lts.state_name(j) << std::endl;
                    }
                }
            }
        }
    }

    int FactorDominanceRelation::num_equivalences() const {
        int num = 0;
        for (State i(0); i < num_states; ++i) {
            for (State j (i.get() + 1); j < num_states; ++j) {
                if (similar(i, j)) {
                    num++;
                }
            }
        }
        return num;
    }

    int FactorDominanceRelation::num_simulations() const {
        int res = 0;
        std::vector<bool> counted(num_states, false);
        for (State i(0); i < num_states; ++i) {
            if (!counted[i.get()]) {
                for (State j (i.get() + 1); j < num_states; ++j) {
                    if (similar(i, j)) {
                        counted[j.get()] = true;
                    }
                }
            }
        }
        for (State i(0); i < num_states; ++i) {
            if (!counted[i.get()]) {
                for (State j (i.get() + 1); j < num_states; ++j) {
                    if (!counted[j.get()]) {
                        if (!similar(i, j) && (simulates(i, j) || simulates(j, i))) {
                            res++;
                        }
                    }
                }
            }
        }
        return res;
    }

    int FactorDominanceRelation::num_different_states() const {
        int num = 0;
        std::vector<bool> counted(num_states, false);
        for (State i(0); i < counted.size(); ++i) {
            if (!counted[i.get()]) {
                num++;
                for (State j (i.get() + 1); j < num_states; ++j) {
                    if (similar(i, j)) {
                        counted[j.get()] = true;
                    }
                }
            }
        }
        return num;
    }



    static class FactorDominanceRelationFactoryPlugin final : public downward::cli::plugins::TypedCategoryPlugin<FactorDominanceRelationFactory> {
    public:
        FactorDominanceRelationFactoryPlugin() : TypedCategoryPlugin("FactorDominanceRelationFactory") {
            document_synopsis( "A FactorDominanceRelationFactory creates FactorDominanceRelations for a given LTS.");
        }
    }
    _category_plugin;



}
