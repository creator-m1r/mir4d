#include "../../Document/Document.hpp"

#include <cassert>

int main()
{
    mir4d::Document document("ObjectStore test");

    const auto first = document.allocateObjectId();
    const auto second = document.allocateObjectId();

    assert(mir4d::isValidObjectId(first));
    assert(mir4d::isValidObjectId(second));
    assert(first != second);
    assert(&document.objects().registry() == &document.objectRegistry());
    assert(document.objects().empty());
    assert(document.isValid());

    assert(document.objectRegistry().contains(first));
    assert(document.objectRegistry().contains(second));

    assert(document.releaseObjectId(second));
    assert(!document.objectRegistry().contains(second));
    assert(document.objects().isValid());

    return 0;
}
