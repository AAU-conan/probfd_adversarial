#include "probfd/search_space_draw.h"
#include "probfd/graphviz.h"

#include "probfd/task_proxy.h"

#include <probfd/algorithms/heuristic_search_base.h>

namespace probfd {
    int SearchSpaceDrawer::state_lookup(const downward::State& state) {
        state.unpack();
        const auto& state_vec = state.get_unpacked_values();
        const int id = state.get_id().get_value();
        if (!id_to_state.contains(id)) {
            id_to_state.insert({id, state});
        }
        return state.get_id().get_value();
    }

    SearchSpaceDrawer::SearchSpaceDrawer(
        std::string output_path,
        ProbabilisticTaskProxy task_proxy)
        : output_path(std::move(output_path))
        , task_proxy(task_proxy)
        , id_to_state()
    {
    }

    void SearchSpaceDrawer::set_initial_state(const downward::State& initial_state) {
    }

    void SearchSpaceDrawer::add_successor(const downward::State& state, const downward::OperatorID& op, const std::vector<downward::State>& successor_states) {
        auto successor_state_ids = std::views::transform(successor_states, [&](const auto& s) {
            return state_lookup(s);
        }) | std::ranges::to<std::vector<int>>();
        edges.push_back({state_lookup(state), successor_state_ids, task_proxy.get_operators()[op]});
    }

    void SearchSpaceDrawer::set_q_value(const downward::State& state, value_t value) {
        int id = state_lookup(state);
        q_values[id] = {value};
    }

    std::string SearchSpaceDrawer::state_name(const std::vector<int>& state_vector) {
        std::string result;
        for (int i = 0; i < static_cast<int>(state_vector.size()); ++i) {
            if (i != 0) {
                result += ", ";
            }
            result += task_proxy.get_variables()[i].get_fact(state_vector[i]).get_name();
        }
        return result;
    }

    void SearchSpaceDrawer::draw_search_space(const Policy<downward::State, downward::OperatorID>& policy) {
        graphviz::Graph graph;

        std::unordered_map<int, size_t> state_id_to_node;
        std::unordered_map<int, downward::OperatorID> state_id_to_policy_op;

        for (const auto& [id, state] : id_to_state) {
            state.unpack();
            std::string label = state_name(state.get_unpacked_values());

            auto decision = policy.get_decision(state);
            if (decision.has_value()) {
                state_id_to_policy_op.insert({id, decision.value().action});
            }

            std::string node_attrs = 0 == id? "shape=cds": "shape=box";
            bool is_goal = true;
            for (auto goal : task_proxy.get_goals()) {
                downward::FactPair fp = goal.get_pair();
                is_goal &= state.get_unpacked_values()[fp.var] == fp.value;
            }
            if (is_goal) {
                node_attrs += ",peripheries=2";
            }

            if (q_values.contains(id)) {
                auto q_value = q_values[id];
                state_id_to_node[id] = graph.add_node(label, std::format("xlabel=\"q={}\"", q_value == INFINITE_VALUE ? "∞": std::format("{}", q_value)));
            } else {
                state_id_to_node[id] = graph.add_node(label,  node_attrs);
            }
        }

        for (const auto& [from_id, to_ids, op] : edges) {
            std::string op_attrs;
            if (state_id_to_policy_op.contains(from_id) && state_id_to_policy_op.at(from_id).get_index() == op.get_id()) {
                op_attrs += "style=dashed,";
            }
            if (to_ids.size() == 1) {
                graph.add_edge(state_id_to_node[from_id], state_id_to_node[to_ids.at(0)], op.get_name(), op_attrs);
            } else {
                size_t op_node = graph.add_node("","shape=point");
                op_attrs += "arrowhead=none,";
                graph.add_edge(state_id_to_node[from_id], op_node, op.get_name(), op_attrs);
                for (const auto& to_id : to_ids) {
                    graph.add_edge(op_node, state_id_to_node[to_id], "");
                }
            }
        }

        graph.output_graph(output_path);
    }
}
