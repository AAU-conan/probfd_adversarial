#include "probfd/dominance/draw_fts.h"

#include "probfd/dominance/fact_names.h"
#include "probfd/graphviz.h"
#include "probfd/dominance/fts_task.h"
#include "probfd/dominance/strong_types.h"
#include "probfd/dominance/labelled_transition_system.h"

namespace probfd::dominance {
    void draw_fts(const std::string &filename, const FTSTask &fts) {
        graphviz::Graph graph(true);
        for (const auto& [i, lts] : std::views::enumerate(fts.get_factors())) {
            std::unordered_map<State, size_t> state_to_node;
            for (State j(0); j < lts->size(); ++j) {
                state_to_node[j] = graph.add_node(fts.get_fact_name(FactPair(i, j)), lts->is_goal(j) ? "peripheries=2" : "");
            }

            for (State s(0); s < lts->size(); ++s) {
                for (const LTSTransition &t : lts->get_transitions(s)) {
                    if (lts->is_relevant_label_group(t.label_group)) {
                        auto op_names = lts->fact_value_names->get_common_operators_name(lts->get_labels(t.label_group));
                        if (t.targets.size() == 1) {
                            graph.add_edge(state_to_node[s], state_to_node[t.targets.at(0)], op_names);
                        } else {
                            size_t op_node = graph.add_node("","shape=point");
                            graph.add_edge(state_to_node[s], op_node, op_names, "arrowhead=none");
                            for (const auto& tgt : t.targets) {
                                graph.add_edge(op_node, state_to_node[tgt], "");
                            }
                        }
                    }
                }
            }
        }
        graph.output_graph(filename);
    }
}