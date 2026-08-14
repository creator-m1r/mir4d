#include "MirEngine/Interaction/SelectionState.hpp"
#include <cassert>

int main()
{
    using mir::SelectionState;
    using mir4d::InvalidObjectId;
    using mir4d::ObjectId;

    SelectionState selection;
    assert(selection.empty());
    assert(selection.primary() == InvalidObjectId);

    const ObjectId first{1};
    const ObjectId second{2};

    selection.select(first);
    assert(selection.size() == 1 && selection.primary() == first && selection.contains(first));

    selection.select(second, true);
    assert(selection.size() == 2 && selection.contains(second));

    selection.select(second, true);
    assert(selection.size() == 2);

    selection.toggle(first);
    assert(selection.size() == 1 && !selection.contains(first) && selection.primary() == second);

    selection.deselect(second);
    assert(selection.empty());

    selection.select(first);
    selection.toggle(first);
    assert(selection.empty());

    selection.replace({first, first, InvalidObjectId, second});
    assert(selection.size() == 2 && selection.ids()[0] == first && selection.ids()[1] == second);

    selection.clear();
    assert(selection.empty());
    return 0;
}
