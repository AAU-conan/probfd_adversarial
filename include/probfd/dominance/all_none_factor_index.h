#ifndef DOMINANCE_ALL_NONE_FACTOR_INDEX_H
#define DOMINANCE_ALL_NONE_FACTOR_INDEX_H

#include "probfd/dominance/strong_types.h"
#include "probfd/dominance/label_outcome_map.h"

#include <cassert>
#include <string>

namespace probfd::dominance {
    // Represents whether l1 dominates l2 in all, all except "factor", or none
    class AllNoneFactorIndex {
        inline static const FactorIndex DOMINATES_IN_ALL = FactorIndex(-2);
        inline static const FactorIndex DOMINATES_IN_NONE = FactorIndex(-1);

        FactorIndex not_present_factor;

    public:
        explicit AllNoneFactorIndex (FactorIndex not_present_factor) :
            not_present_factor(not_present_factor) {
        }
        static AllNoneFactorIndex all_factors() {
            return AllNoneFactorIndex (DOMINATES_IN_ALL);
        }
        static AllNoneFactorIndex no_factors() {
            return AllNoneFactorIndex (DOMINATES_IN_NONE);
        }

        [[nodiscard]] bool is_none() const {
            return not_present_factor == DOMINATES_IN_NONE;
        }

        [[nodiscard]] bool is_all() const {
            return not_present_factor == DOMINATES_IN_ALL;
        }

        [[nodiscard]] bool is_factor() const {
            return not_present_factor >= FactorIndex(0);
        }

        [[nodiscard]] FactorIndex get_not_present_factor () const {
            assert(is_factor());
            return not_present_factor;
        }

        [[nodiscard]] bool contains (FactorIndex factor) const {
            return !is_none() && (is_all() || not_present_factor != factor);
        }

        bool remove (FactorIndex factor) {
            if (is_all()) {
                not_present_factor = factor;
                return true;
            } else if (!is_none() && not_present_factor != factor) {
                not_present_factor = DOMINATES_IN_NONE;
                return true;
            }
            return false;
        }

        bool contains_all_except (FactorIndex factor) const {
            return is_all() || factor == not_present_factor;
        }

        bool operator ==(const AllNoneFactorIndex & other) const {
            return not_present_factor == other.not_present_factor;
        }

        std::string to_string() const {
            if (is_all())
                return "all";
            if (is_none())
                return "none";
            return std::format("all_except_{}", get_not_present_factor().get());
        }
    };
}



#endif
