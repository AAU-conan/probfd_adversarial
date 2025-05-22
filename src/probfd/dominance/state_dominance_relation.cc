#include "probfd/dominance/state_dominance_relation.h"

#include "probfd/dominance/factor_dominance_relation.h"
#include "probfd/dominance/label_relation.h"
#include "probfd/dominance/labelled_transition_system.h"
#include "downward/task_proxy.h"
#include "downward/utils/logging.h"
using namespace std;

namespace probfd::dominance {

    double StateDominanceRelation::get_percentage_simulations(bool ignore_equivalences) const {
        double percentage = 1;
        for (auto &sim: local_relations) {
            percentage *= static_cast<double>(sim->num_simulations()) / (sim->get_num_states() * sim->get_num_states());
        }
        if (ignore_equivalences) {
            percentage -= get_percentage_equivalences();
        } else {
            percentage -= get_percentage_equal();
        }
        return percentage;
    }

    double StateDominanceRelation::get_percentage_equal() const {
        double percentage = 1;
        for (auto &sim: local_relations) {
            percentage *= 1. / (sim->get_num_states() * sim->get_num_states());
        }
        return percentage;
    }

    bool StateDominanceRelation::dominates(const downward::State& t, const downward::State& s) const {
        t.unpack();
        s.unpack();
        for (auto [i, sim]: std::views::enumerate(local_relations)) {
            if (!sim->simulates(State(t.get_unpacked_values()[i]), State(s.get_unpacked_values()[i]))) {
                return false;
            }
        }
        return true;
    }

    double StateDominanceRelation::get_percentage_equivalences() const {
        double percentage = 1;
        for (auto &sim: local_relations) {
            int num_eq = 0;
            int num_states = sim->get_num_states();
            for (State i(0); i < num_states; ++i)
                for (State j(0); j < num_states; ++j)
                    if (sim->similar(i, j))
                        num_eq++;
            percentage *= num_eq / (static_cast<double>(num_states) * num_states);
        }
        return percentage;
    }


    int StateDominanceRelation::num_equivalences() const {
        int res = 0;
        for (size_t i = 0; i < local_relations.size(); i++) {
            res += local_relations[i]->num_equivalences();
        }
        return res;
    }

    int StateDominanceRelation::num_simulations() const {
        int res = 0;
        for (size_t i = 0; i < local_relations.size(); i++) {
            res += local_relations[i]->num_simulations() - local_relations[i]->get_num_states();
        }
        return res;
    }

    double StateDominanceRelation::num_st_pairs() const {
        double res = 1;
        for (size_t i = 0; i < local_relations.size(); i++) {
            res *= local_relations[i]->num_simulations() - local_relations[i]->get_num_states();
        }
        return res;
    }


    double StateDominanceRelation::num_states_problem() const {
        double res = 1;
        for (size_t i = 0; i < local_relations.size(); i++) {
            res *= local_relations[i]->get_num_states();
        }
        return res;
    }


    StateDominanceRelation::StateDominanceRelation(
        std::vector<std::unique_ptr<FactorDominanceRelation>>&& _local_relations,
        std::unique_ptr<LabelOutcomeRelation>& label_relation):
        local_relations (std::move(_local_relations)), label_relation(std::move(label_relation)) {
    }

    void StateDominanceRelation::dump_statistics() const {
        int num_equi = num_equivalences();
        int num_sims = num_simulations();

        int num_vars = 0;
        int num_vars_with_simulations = 0;
        for (size_t i = 0; i < local_relations.size(); i++) {
            if (!local_relations[i]->is_identity()) {
                num_vars_with_simulations++;
            }
            num_vars++;
        }

        std::cout << "Total Simulations: " << num_sims + num_equi * 2 << std::endl;
        std::cout << "Similarity equivalences: " << num_equi << std::endl;
        std::cout << "Only Simulations: " << num_sims << std::endl;
        std::cout << "Simulations Found in " << num_vars_with_simulations << " out of " << num_vars << " variables" << std::endl;
    }

}