#include "MirEngine/BRep/BRep.hpp"

int main()
{

    mir::BRepModel model;
    mir::BRepValidator validator;

    if (!model.empty())
        return 1;

    if (!validator.validate(model).ok())
        return 1;

    return 0;
}
