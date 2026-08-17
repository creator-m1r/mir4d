#include <cassert>
#include <cstdio>
#include <cstring>

extern "C" bool MirEngineRunCAECampaign(const char* definitionText, char* outJson, size_t outCapacity);

int main()
{
    const char* definition =
        "case hot\n"
        "  material temperature 350\n"
        "  initial flowRate 5\n"
        "  criterion temperature 0 400\n"
        "  criterion stress 0 1e9\n"
        "case soft\n"
        "  material youngModulus 1e11\n"
        "  initial flowRate 5\n"
        "  criterion stress 0 1e11\n";

    char buffer[1 << 16];
    const bool ok = MirEngineRunCAECampaign(definition, buffer, sizeof(buffer));
    assert(ok);
    assert(std::strlen(buffer) > 0);
    assert(std::strstr(buffer, "\"passed\"") != nullptr);
    assert(std::strstr(buffer, "\"cases\"") != nullptr);
    assert(std::strstr(buffer, "\"hot\"") != nullptr);

    printf("CAE export produced %zu bytes\n", std::strlen(buffer));
    return 0;
}
