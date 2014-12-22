#include "log.h"


using namespace mgl;
using namespace std;


ostream & Log::often()
{
    return std::cout;
}


ostream & Log::rarely()
{
    return std::cout;
}

ostream & Log::error()
{
    return cerr;
}

