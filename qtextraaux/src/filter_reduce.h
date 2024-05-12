#pragma once
#include <algorithm>

namespace Qt5Extra
{
    template<class _FwdIt, class _Pred, class _Fn>
    _Fn filter_reduce(_FwdIt first, _FwdIt eos, _Pred pred, _Fn action)
    {
        while(first != eos)
        {
            auto last = std::find_if(first, eos, pred);
            if (first == last)
            {
                first = ++last;
                continue;
            }

            action(first, last);

            if (last == eos)
                break;

            first = ++last;
        }
        return action;
    }
} // end namespace Qt5Extra
