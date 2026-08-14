#include "MirEngine/Document/Document.hpp"

int main()
{
    mir4d::Document document;
    return document.scene().isValid() && document.time().isValid() ? 0 : 1;
}
