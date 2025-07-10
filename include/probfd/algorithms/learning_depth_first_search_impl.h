#ifndef GUARD_INCLUDE_PROBFD_ALGORITHMS_LEARNING_DEPTH_FIRST_SEARCH_H
#error "This file should only be included from learning_depth_first_search.h"
#endif

#include "downward/utils/countdown_timer.h"
#include "probfd/search_space_draw.h"
#include "probfd/utils/state_name.h"

#include <cassert>
#include <print>

#ifndef NDEBUG
#define SEARCH_SPACE_DRAWER if (this->search_space_drawer) this->search_space_drawer
#else
#define SEARCH_SPACE_DRAWER if (false) this->search_space_drawer
#endif


namespace probfd::algorithms::learning_depth_first_search {

namespace internal {

inline void Statistics::print(std::ostream& out) const
{
    out << "  Iterations: " << iterations << std::endl;
    out << "  Value iterations: " << convergence_value_iterations << std::endl;
    out << "  Bellman backups (forward): " << forward_updates << std::endl;
    out << "  Bellman backups (backtracking): " << backtracking_updates
        << std::endl;
    out << "  Bellman backups (convergence): " << convergence_updates
        << std::endl;
}

} // namespace internal

template <typename State, typename Action>
LearningDepthFirstSearch<State, Action>::LearningDepthFirstSearch(value_t epsilon, std::shared_ptr<PolicyPicker> policy_chooser, bool backtrack_update_upperbound, bool upperbound_update_to_qvalue, bool simple)
    : Base(epsilon, std::move(policy_chooser))
    , backtrack_update_upperbound_(backtrack_update_upperbound)
    , upperbound_update_to_qvalue_(upperbound_update_to_qvalue)
    , simple_(simple)
{
}

template <typename State, typename Action>
Interval LearningDepthFirstSearch<State, Action>::do_solve(
    MDP& mdp,
    HeuristicType& heuristic,
    PruningType& pruning,
    ParamType<State> state,
    ProgressReport& progress,
    double max_time)
{
    downward::utils::CountdownTimer timer(max_time);

    const StateID stateid = mdp.get_state_id(state);
    const StateInfo& state_info = this->state_infos_[stateid];

    progress.register_bound("v", [&state_info]() {
        return as_interval(state_info.value);
    });

    do {
        if (simple_) {
            exploration_simple_recursive(mdp, heuristic, pruning, stateid, state_info.get_bounds().lower, timer);
        } else {
            exploration_recursive(mdp, heuristic, pruning, stateid, state_info.get_bounds().lower, timer);
        }
        SEARCH_SPACE_DRAWER->draw_search_space();
        ++statistics_.iterations;
        progress.print();
    } while (state_info.get_bounds().lower < state_info.get_bounds().upper);

    return state_info.get_bounds();
}

template <typename State, typename Action>
void LearningDepthFirstSearch<State, Action>::
    print_additional_statistics(std::ostream& out) const
{
    statistics_.print(out);
}

template <typename State, typename Action>
bool LearningDepthFirstSearch<State, Action>::exploration_recursive(
    MDP& mdp,
    HeuristicType& heuristic,
    PruningType& pruning,
    StateID state,
    value_t bound,
    downward::utils::CountdownTimer& timer)
{
    StateInfo* sinfo;
    sinfo = &this->state_infos_[state];
    initialize(mdp, heuristic, pruning, state, *sinfo);
    SEARCH_SPACE_DRAWER->draw_search_space();

    // std::println("Exploring state: {}, V: [{}, {}], bound: {}", state_name(mdp.get_state(state)), sinfo->get_bounds().lower, sinfo->get_bounds().upper, bound);

    if (sinfo->is_goal_or_terminal() || sinfo->get_bounds().lower > bound || sinfo->get_bounds().upper <= bound) {
        if (sinfo->is_goal_or_terminal()) {
            this->update_value(*sinfo, Interval(mdp.get_termination_cost(mdp.get_state(state))), this->epsilon);
            SEARCH_SPACE_DRAWER->set_q_value(mdp.get_state(state), Interval(mdp.get_termination_cost(mdp.get_state(state))));
        }
        // std::println("State {} is already solved, V: [{}, {}], bound: {}", state_name(mdp.get_state(state)), sinfo->get_bounds().lower, sinfo->get_bounds().upper, bound);
        return true; // State is already solved
    }

    auto full_state = mdp.get_state(state);

    bool flag = false;

    std::vector<TransitionTail<Action>> transition_tails;
    this->generate_non_tip_transitions(mdp, pruning, full_state, transition_tails);
    auto tail_it = transition_tails.begin();
    for (; tail_it != transition_tails.end(); ++tail_it) {
        auto tail = *tail_it;
        if (this->compute_qvalue(tail, mdp).lower > bound) continue;
        flag = true;
        for (const auto& [succ_id, _] : tail.successor_dist.non_source_successor_dist) {
            flag = exploration_recursive(mdp, heuristic, pruning, succ_id, bound - mdp.get_action_cost(tail.action), timer);
            flag &= this->compute_qvalue(tail, mdp).lower <= bound;
            if (!flag) {
                break;
            }
        }
        if (flag) {
            break;
        }
    }

    if (flag) {
        // std::println("State {} solved within bound {}", state_name(full_state), bound);
        auto value = this->compute_qvalue(*tail_it, mdp);
        this->update_policy(*sinfo, *tail_it);
        if (upperbound_update_to_qvalue_) {
            this->update_value(*sinfo, value, this->epsilon);
        } else {
            this->update_value(*sinfo, Interval(sinfo->get_bounds().lower, bound), this->epsilon);
        }
    } else {
        AlgorithmValueType value;
        if (backtrack_update_upperbound_) {
            ClearGuard _(qvalues_);
            value = this->compute_bellman_and_greedy(full_state, transition_tails, mdp, qvalues_);
            qvalues_.clear();
            auto result = this->update_value(*sinfo, value, this->epsilon);
            if (result.converged) {
                // We found the true value, but outside the bound, update the policy
                auto transition = this->select_greedy_transition(mdp, sinfo->get_policy(), transition_tails);
                this->update_policy(*sinfo, transition);
            }
        } else {
            value = this->compute_bellman(full_state, transition_tails, mdp);
            this->update_value(*sinfo, Interval(value.lower, sinfo->get_bounds().upper), this->epsilon);
        }
        SEARCH_SPACE_DRAWER->set_q_value(mdp.get_state(state), value);
        // std::println("State {} not solved within bound {}, new V: [{},{}]", state_name(full_state), bound, sinfo->get_bounds().lower, sinfo->get_bounds().upper);
    }
    ++statistics_.backtracking_updates;
    return flag;
}



template <typename State, typename Action>
bool LearningDepthFirstSearch<State, Action>::exploration_simple_recursive(
    MDP& mdp,
    HeuristicType& heuristic,
    PruningType& pruning,
    StateID state,
    value_t bound,
    downward::utils::CountdownTimer& timer)
{
    StateInfo* sinfo;
    sinfo = &this->state_infos_[state];
    initialize(mdp, heuristic, pruning, state, *sinfo);
    // SEARCH_SPACE_DRAWER->draw_search_space();

    // std::println("Exploring state: {}, V: [{}, {}], bound: {}", state.id, sinfo->get_bounds().lower, sinfo->get_bounds().upper, bound);
    if (sinfo->is_goal_or_terminal()) {
        this->update_value(*sinfo, Interval(mdp.get_termination_cost(mdp.get_state(state))), this->epsilon);
        SEARCH_SPACE_DRAWER->set_q_value(mdp.get_state(state), sinfo->get_bounds());
        // std::println("State is terminal");
        return sinfo->is_goal_state(); // State is already solved
    } else if (sinfo->get_bounds().lower > bound) {
        // std::println("State already cannot be solved within the bound");
        return false; // State cannot be solved within the bound
    } else if (sinfo->get_bounds().upper <= bound) {
        // std::println("State is already solved within the bound");
        return true; // State is already solved within the bound
    }

    auto full_state = mdp.get_state(state);
    bool flag;
    do {
        SuccessorDistribution successors;
        flag = false;
        if (!sinfo->get_policy().has_value()) {
            ++statistics_.backtracking_updates;
            ClearGuard _(transitions_, qvalues_);
            this->generate_non_tip_transitions(mdp, pruning, full_state, transitions_);
            auto value = this->compute_bellman_and_greedy(full_state, transitions_, mdp, qvalues_);
            auto transition = this->select_greedy_transition( mdp, sinfo->get_policy(), transitions_);
            this->update_value(*sinfo, value, this->epsilon);
            this->update_policy(*sinfo, transition);

            // std::println("New greedy policy: V: [{}, {}], bound: {}", sinfo->get_bounds().lower, sinfo->get_bounds().upper, bound);
            if (value.lower > bound) {
                // If we are now above the bound, we cannot solve this state within the bound
                break;
            }
            successors = std::move(transition->successor_dist);
        } else {
            auto action = sinfo->get_policy();
            mdp.generate_action_transitions(full_state, *action, successors);
        }
        value_t action_cost = mdp.get_action_cost(sinfo->get_policy().value());
        for (const auto& [succ_id, _] : successors.non_source_successor_dist) {
            flag = exploration_simple_recursive(mdp, heuristic, pruning, succ_id, bound - action_cost, timer);
            if (!flag) {
                break; // Stop if any successor cannot be solved
            }
        }

        if (!flag) {
            // This state cannot be solved within the bound by the greedy policy
            // Remove the greedy policy
            this->update_policy(*sinfo, std::nullopt);
        } else {
            Interval new_value = sinfo->get_bounds();
            set_min(new_value, Interval(bound));
            this->update_value(*sinfo, new_value, this->epsilon);
            SEARCH_SPACE_DRAWER->set_q_value(mdp.get_state(state), sinfo->get_bounds());
        }
    } while (!flag);

    // std::println("State {}{} solved within bound {}, V: [{}, {}]", state.id, flag? "": " not", bound, sinfo->get_bounds().lower, sinfo->get_bounds().upper);
    return flag;
}


template <typename State, typename Action>
bool LearningDepthFirstSearch<State, Action>::initialize(
    MDP& mdp,
    HeuristicType& heuristic,
    PruningType& pruning,
    StateID state_id,
    StateInfo& sinfo)
{
    // Ignore labels if labelling option is turned off
    const auto state = mdp.get_state(state_id);

    const bool is_tip_state = sinfo.is_on_fringe();

    if (is_tip_state) {
        ClearGuard _(transitions_, qvalues_);
        this->expand_and_initialize(
            mdp,
            heuristic,
            pruning,
            state,
            sinfo,
            transitions_);
    }
    return true;
}

} // namespace probfd::algorithms::learning_depth_first_search
