#include "probfd/dominance/labelled_transition_system.h"
#include "probfd/dominance/label_map.h"
#include "probfd/dominance/strong_types.h"

#include <utility>

#include "probfd/merge_and_shrink/transition_system.h"

#include <probfd/dominance/fact_names.h>

namespace downward {
    class AbstractTask;
}
using namespace std;

namespace probfd::dominance {
    LabelledTransitionSystem::LabelledTransitionSystem(const merge_and_shrink::TransitionSystem &ts,
                                                       const LabelMap &labelMap,
                                                       std::shared_ptr<FactValueNames> fact_value_names) :
            num_states(ts.get_size()), num_labels(labelMap.get_num_labels()), init_state(ts.get_init_state()), fact_value_names(std::move(fact_value_names)) {

        label_group_of_label.resize(num_labels, LabelGroup(-1));

        transitions_src.resize(num_states);

        for (const auto & local_label_info : ts.label_infos()) {
            const auto & abs_tr = local_label_info.get_transitions();

            if (!abs_tr.empty()) {
                LabelGroup new_label_group_id {(int)label_groups.size()};

                std::vector<Label> new_label_group;
                for (int label : local_label_info.get_label_group()) {
                    auto maybe_new_label_id = labelMap.get_id(label);
                    if(maybe_new_label_id.has_value()) {
                        int new_label_id = maybe_new_label_id.value();
                        new_label_group.push_back(Label(new_label_id));
                        label_group_of_label[new_label_id] = new_label_group_id;
                    }
                }
                assert(!new_label_group.empty());
                label_groups.push_back(new_label_group);
                transitions_label_group.emplace_back();
                for (const auto & tr : abs_tr) {
                    auto trs = std::views::transform(tr.targets, [](int s) { return State(s); }) | std::ranges::to<std::vector<State>>();
                    transitions_label_group[new_label_group_id.group].emplace_back(State(tr.src), trs);
                    transitions.emplace_back(State(tr.src), trs, new_label_group_id);
                    transitions_src[tr.src].emplace_back(State(tr.src), trs, new_label_group_id);
                }
            } else {
                // Dead labels should have been removed
                assert(false);
            }
        }

        label_group_is_relevant.resize(label_groups.size(), false);
        for (const auto& [lg_i, _] : std::views::enumerate(label_groups)) {
            if (const auto lg = LabelGroup(static_cast<int>(lg_i)); !irrelevant_label_group(lg)) {
                relevant_label_groups.push_back(lg);
                label_group_is_relevant[lg_i] = true;
            }
        }

        for (int s = 0; s < ts.get_size(); ++s) {
            goal_states.push_back(ts.is_goal_state(s));
        }
    }

    std::string LabelledTransitionSystem::state_name(State s) const {
        return fact_value_names->get_fact_value_name(s.get());
    }

    std::string LabelledTransitionSystem::label_name(Label label) const {
        return fact_value_names->get_operator_name(label.get());
    }

    std::string LabelledTransitionSystem::label_group_name(const LabelGroup& lg) const {
        return fact_value_names->get_common_operators_name(get_labels(lg));
    }

    void LabelledTransitionSystem::dump() const {
        for (int s = 0; s < size(); s++) {
            applyPostSrc(s, [&](const LTSTransition &trs) {
                cout << trs.src << " -> ";
                for (const auto &target: trs.targets) {
                    cout << target << " ";
                }
                cout << " (" << trs.label_group.group << ":";
                for (int tr_s_label: get_labels(trs.label_group)) {
                    cout << " " << tr_s_label;
                }
                cout << ")\n";
                return false;
            });
        }

    }

    /*
     * Returns true if the label group has a self-loop in every state and no other transitions
     */
    bool LabelledTransitionSystem::irrelevant_label_group(LabelGroup lg) const {
        const auto &trs = get_transitions_label_group(lg);
        return trs.size() == (size_t) num_states && is_self_loop_everywhere_label_group(lg);
    }

    /*
     * Returns true if the label group has a self loop in every state
     */
    bool LabelledTransitionSystem::is_self_loop_everywhere_label_group(LabelGroup lg) const {
        const auto &trs = get_transitions_label_group(lg);
        if (trs.size() < (size_t) num_states) return false;

        // This assumes that there is no repeated transition
        int num_self_loops = 0;
        for (const auto &tr: trs) {
            bool is_self_loop = true;
            for (const auto &target: tr.targets) {
                if (tr.src != target) {
                    is_self_loop = false;
                    break;
                }
            }
            if (is_self_loop) {
                num_self_loops++;
            }
        }

        assert(num_self_loops <= num_states);
        return num_self_loops == num_states;
    }

}


