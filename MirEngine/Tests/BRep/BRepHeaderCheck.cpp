#include "MirEngine/BRep/BRep.hpp"

int main()
{
    // Compile-time contract for the canonical public B-Rep API.
    // The test intentionally performs no modeling work: its purpose is to
    // ensure the complete public umbrella remains self-consistent.
    mir::BRepModel model;
    mir::BRepValidator validator;

    if (!model.empty())
        return 1;

    if (!validator.validate(model).ok())
        return 1;

    return 0;
}
