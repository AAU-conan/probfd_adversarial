#ifndef PROBFD_ALGORITHMS_DEPTH_FIRST_HEURISTIC_SEARCH_H
#define PROBFD_ALGORITHMS_DEPTH_FIRST_HEURISTIC_SEARCH_H

#include "probfd/algorithms/heuristic_search_base.h"

#include <deque>
#include <iostream>
#include <probfd/search_space_draw.h>
#include <type_traits>
#include <vector>

// Forward Declarations
namespace downward::utils {
class CountdownTimer;
}

/// Namespace dedicated to Depth-First Heuristic Search.
namespace probfd::algorithms::learning_depth_first_search {

namespace internal {
struct Statistics {
    unsigned long long iterations = 0;
    unsigned long long forward_updates = 0;
    unsigned long long backtracking_updates = 0;
    unsigned long long convergence_updates = 0;
    unsigned long long convergence_value_iterations = 0;

    void print(std::ostream& out) const;
};

struct DFSState {
public:
    enum class Status {
        TIP_NOT_LAST,
        TIP_LAST,
        ACTIVE_NOT_LAST,
        ACTIVE_LAST,
    };
    StateID state_id;
    value_t bound{};
    DFSState* parent{};
    Status status;
    bool flag = false;
};
}

/**
 * @brief Implementation of the depth-first heuristic search algorithm family
 * \cite steinmetz:etal:icaps-16.
 *
 * @tparam State - The state type of the underlying MDP.
 * @tparam Action - The action type of the underlying MDP.
 */
template <typename State, typename Action>
class LearningDepthFirstSearch
    : public heuristic_search::HeuristicSearchAlgorithm<
          State,
          Action,
          heuristic_search::PerStateBaseInformation<Action, true, true>> {
    using Base =
        typename LearningDepthFirstSearch::HeuristicSearchAlgorithm;

public:
    using StateInfo = typename Base::StateInfo;
    using AlgorithmValueType = typename Base::AlgorithmValueType;

private:
    using MDP = typename Base::MDPType;
    using HeuristicType = typename Base::HeuristicType;
    using PruningType = typename Base::PruningType;

    using PolicyPicker = typename Base::PolicyPicker;

    using Statistics = internal::Statistics;

    static constexpr uint32_t NEW = std::numeric_limits<uint32_t>::max() - 1;
    static constexpr uint32_t CLOSED = std::numeric_limits<uint32_t>::max();

    // Algorithm parameters
    const bool backtrack_update_upperbound_;
    const bool upperbound_update_to_qvalue_;
    const bool simple_;

    // Algorithm state

    // Re-used buffer
    std::vector<TransitionTail<Action>> transitions_;
    std::vector<AlgorithmValueType> qvalues_;
    SuccessorDistribution successor_dist_;

    Statistics statistics_;

public:
    explicit LearningDepthFirstSearch(value_t epsilon, std::shared_ptr<PolicyPicker> policy_chooser, bool backtrack_update_upperbound, bool upperbound_update_to_qvalue, bool simple);

protected:
    Interval do_solve(
        MDP& mdp,
        HeuristicType& heuristic,
        PruningType& pruning,
        ParamType<State> state,
        ProgressReport& progress,
        double max_time) override;

    void print_additional_statistics(std::ostream& out) const override;

    struct StackFrame {
        StateID state_id;
        value_t bound;
        std::vector<TransitionTail<Action>> transition_tails;
        typename std::vector<TransitionTail<Action>>::iterator tail_it;
        typename std::vector<ItemProbabilityPair<StateID, value_t>>::const_iterator succ_it;

        bool flag{false};
        bool is_initialized{false};
        bool child_returned{false};

        StackFrame(StateID state_id, value_t bound)
            : state_id(state_id), bound(bound)
        {
        }
    };

private:
    bool exploration_recursive(
        MDP& mdp,
        HeuristicType& heuristic,
        PruningType& pruning,
        StateID state,
        value_t bound,
        downward::utils::CountdownTimer& timer);

    bool exploration_simple_recursive(
        MDP& mdp,
        HeuristicType& heuristic,
        PruningType& pruning,
        StateID state,
        value_t bound,
        downward::utils::CountdownTimer& timer);


    void exploration_iterative(
        MDP& mdp,
        HeuristicType& heuristic,
        PruningType& pruning,
        StateID initial_state,
        value_t initial_bound,
        downward::utils::CountdownTimer& timer);

    bool initialize(
        MDP& mdp,
        HeuristicType& heuristic,
        PruningType& pruning,
        StateID state_id,
        StateInfo& sinfo);
};

} // namespace probfd::algorithms::LearningDepthFirstSearch

#define GUARD_INCLUDE_PROBFD_ALGORITHMS_LEARNING_DEPTH_FIRST_SEARCH_H
#include "probfd/algorithms/learning_depth_first_search_impl.h"
#undef GUARD_INCLUDE_PROBFD_ALGORITHMS_LEARNING_DEPTH_FIRST_SEARCH_H

#endif // PROBFD_ALGORITHMS_DEPTH_FIRST_HEURISTIC_SEARCH_H
