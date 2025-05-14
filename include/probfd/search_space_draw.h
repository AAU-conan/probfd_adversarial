#ifndef UTILS_SEARCH_SPACE_DRAW_H
#define UTILS_SEARCH_SPACE_DRAW_H

#include "algorithms/acyclic_value_iteration.h"
#include "algorithms/heuristic_search_state_information.h"
#include "task_proxy.h"

#include <unordered_map>
#include <vector>

#include <boost/container_hash/hash.hpp>

namespace probfd {
class ProbabilisticTaskProxy;
}

namespace downward {
class State;
}

namespace probfd {
    template <typename Container>
    struct container_hash {
        std::size_t operator()(Container const& c) const {
            return boost::hash_range(c.begin(), c.end());
        }
    };

    class SearchSpaceDrawer {
        struct Edge {
            int from_id;
            std::vector<int> to_ids;
            probfd::ProbabilisticOperatorProxy op;
        };

        int state_lookup(const downward::State& state);
        std::string state_name(const std::vector<int>& state_vector);

        ProbabilisticTaskProxy task_proxy;
        std::vector<Edge> edges;
        std::unordered_map<int, downward::State> id_to_state;
        std::unordered_map<int, value_t> q_values;

        std::string output_path;

    public:
        explicit SearchSpaceDrawer(std::string output_path, ProbabilisticTaskProxy task_proxy);

        void set_initial_state(const downward::State &initial_state);

        template <typename State, typename Action>
        void add_successor(const State &state, const Action& op, const std::vector<State> &successor_states)
        {
            throw std::runtime_error("Not implemented");
        }
        void add_successor(const downward::State &state, const downward::OperatorID &op, const std::vector<downward::State> &successor_states);

        template <typename State, typename Value>
        void set_q_value(const State &state, Value value)
        {
            throw std::runtime_error("Not implemented");
        }
        void set_q_value(const downward::State &state, value_t value);

        template <typename State, typename Action>
        void draw_search_space(const Policy<State, Action>* policy = nullptr)
        {
            throw std::runtime_error("Not implemented");
        }
        void draw_search_space(const Policy<downward::State, downward::OperatorID>* policy = nullptr);

    };
}

#endif //SEARCH_SPACE_DRAW_H
