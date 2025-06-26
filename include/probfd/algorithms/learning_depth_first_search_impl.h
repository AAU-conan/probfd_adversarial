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
LearningDepthFirstSearch<State, Action>::LearningDepthFirstSearch(value_t epsilon, std::shared_ptr<PolicyPicker> policy_chooser, bool backtrack_update_upperbound, bool upperbound_update_to_qvalue)
    : Base(epsilon, std::move(policy_chooser))
    , backtrack_update_upperbound_(backtrack_update_upperbound)
    , upperbound_update_to_qvalue_(upperbound_update_to_qvalue)
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
        exploration_recursive(mdp, heuristic, pruning, stateid, state_info.get_bounds().lower, timer);
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



// template <typename State, typename Action>
// bool LearningDepthFirstSearch<State, Action>::policy_exploration(
//     MDP& mdp,
//     HeuristicType& heuristic,
//     PruningType& pruning,
//     StateID state,
//     downward::utils::CountdownTimer& timer)
// {
//     StateInfo* sinfo;
//     sinfo = &this->state_infos_[state];
//     initialize(mdp, heuristic, pruning, state, *sinfo);
//
//     push( state, sinfo->get_bounds().lower, nullptr, internal::DFSState::Status::NEW); // Put initial state on the top of the stack
//
//     do {
//         auto& [current, bound, parent, status, flag] = dfs_stack_.back();
//         sinfo = &this->state_infos_[current];
//
//         using Status = internal::DFSState::Status;
//         if (status == Status::TIP_LAST || status == Status::TIP_NOT_LAST) {
//             // This state is a tip, i.e. not handled, make it active
//             status = (status == Status::TIP_LAST) ? Status::ACTIVE_LAST : Status::ACTIVE_NOT_LAST;
//
//             // Initialize the exploration state, if it is not already initialized
//             initialize(mdp, heuristic, pruning, current, *sinfo);
//
//             if (sinfo->is_goal_or_terminal() || sinfo->bounds_approximately_equal(0)) {
//                 assert(sinfo->get_bounds().bounds_approximately_equal(0));
//                 // State is already solved
//                 flag = true;
//             } else {
//                 flag = false;
//
//                 ClearGuard _(transitions_, qvalues_);
//                 this->generate_non_tip_transitions(mdp, pruning, state, transitions_);
//                 this->compute_q_values(transitions_, mdp, qvalues_);
//
//                 for (int i = 0; i < transitions_.size(); ++i) {
//                     if (qvalues_[i] > bound) continue;
//                     bool first = true;
//                     for (const auto& [succ_id, _] : transitions_[i].successor_dist.non_source_successor_dist) {
//                         // If this is the first successor, it is the last tip
//                         push (succ_id, bound - mdp.get_action_cost(transitions_[i].action), &dfs_stack_.back(), first ? Status::TIP_LAST : Status::TIP_NOT_LAST);
//                         first = false;
//                     }
//                 }
//             }
//         } else {
//             // All children of this state have been processed
//
//             auto value = this->compute_bellman_and_greedy(state, transitions_, mdp, qvalues_);
//             flag &= value <= bound; // Check if we solved this state within bound
//
//             // Update
//             if (flag) {
//                 auto transition = this->select_greedy_transition(mdp, sinfo->get_policy(), transitions_);
//                 this->update_policy(*sinfo, transition);
//             }
//             this->update_value(*sinfo, value, this->epsilon);
//
//             parent->flag &= flag; // Propagate the solved flag to the parent
//
//             if (status == Status::ACTIVE_LAST) {
//                 // This is the last child of the parent, so we need to check if we solved
//                 if (flag) {
//                     // We solved, discard all other children. This invalidates out own DFSState variables!
//                     do {
//                         dfs_stack_.pop_back();
//                     } while (&dfs_stack_.back() != parent);
//                 } else {
//                     // We did not solve, so we reset the parent flag to true
//                 }
//             }
//
//         }
//     } while (!dfs_stack_.empty());
//
//
//
//
// }


template <typename State, typename Action>
void LearningDepthFirstSearch<State, Action>::push(StateID stateid, value_t bound, internal::DFSState* parent, internal::DFSState::Status status)
{
    dfs_stack_.push_back({stateid, bound, parent, status});
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

template <typename State, typename Action>
bool LearningDepthFirstSearch<State, Action>::value_iteration(
    MDP& mdp,
    PruningType& pruning,
    const std::ranges::input_range auto& range,
    downward::utils::CountdownTimer& timer)
{
    ++statistics_.convergence_value_iterations;

    for (;;) {
        auto [value_changed, policy_changed] =
            vi_step(mdp, pruning, range, timer);

        if (policy_changed) return false;
        if (!value_changed) break;
    }

    return true;
}

template <typename State, typename Action>
std::pair<bool, bool>
LearningDepthFirstSearch<State, Action>::vi_step(
    MDP& mdp,
    PruningType& pruning,
    const std::ranges::input_range auto& range,
    downward::utils::CountdownTimer& timer)
{
    bool values_not_conv = false;
    bool policy_not_conv = false;

    for (const StateID id : range) {
        timer.throw_if_expired();

        StateInfo& state_info = this->state_infos_[id];

        const auto state = mdp.get_state(id);

        ClearGuard _(transitions_, qvalues_);

        this->generate_non_tip_transitions(mdp, pruning, state, transitions_);

        const auto value = this->compute_bellman_and_greedy(
            state,
            transitions_,
            mdp,
            qvalues_);

        ++statistics_.convergence_updates;

        auto transition = this->select_greedy_transition(
            mdp,
            state_info.get_policy(),
            transitions_);

        auto val_upd = this->update_value(state_info, value, this->epsilon);
        bool policy_changed = this->update_policy(state_info, transition);
        values_not_conv = values_not_conv || !val_upd.converged;
        policy_not_conv = policy_not_conv || policy_changed;
    }

    return std::make_pair(values_not_conv, policy_not_conv);
}

} // namespace probfd::algorithms::learning_depth_first_search
