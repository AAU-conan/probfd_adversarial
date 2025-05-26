#ifndef DOMINANCE_STRONG_TYPES_H
#define DOMINANCE_STRONG_TYPES_H

#include <NamedType/named_type.hpp>

namespace fluent {
    template <typename T>
    struct PrePostIncrementable : crtp<T, PreIncrementable>
    {
        IGNORE_SHOULD_RETURN_REFERENCE_TO_THIS_BEGIN

        FLUENT_CONSTEXPR17 T& operator++()
        {
            ++this->underlying().get();
            return this->underlying();
        }

        FLUENT_CONSTEXPR17 T operator++(int)
        {
            return T(this->underlying().get()++);
        }

        IGNORE_SHOULD_RETURN_REFERENCE_TO_THIS_END
    };
}

#define BASE_FUNCTIONALITIES \
    fluent::Comparable, fluent::PrePostIncrementable, fluent::Printable, fluent::ImplicitlyConvertibleTo<size_t>::templ, fluent::Hashable

namespace probfd::dominance {
    using Label = fluent::NamedType<int, struct LabelTag, BASE_FUNCTIONALITIES>;
    using LabelOutcome = fluent::NamedType<int, struct LabelOutcomeTag, BASE_FUNCTIONALITIES>;
    using State = fluent::NamedType<int, struct StateTag, BASE_FUNCTIONALITIES>;
    using FactorIndex = fluent::NamedType<int, struct FactorIndexTag, BASE_FUNCTIONALITIES>;
}

#endif //DOMINANCE_STRONG_TYPES_H
