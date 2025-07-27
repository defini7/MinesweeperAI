#include "../Include/App.hpp"

int main()
{
    App app;

    if (app.Construct(600, 400, 1, 1, false, true))
        app.Run();

    return 0;
}
