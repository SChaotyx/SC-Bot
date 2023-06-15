#include "includes.h"
#include <matdash/boilerplate.hpp>

#ifdef SHOW_CONSOLE
    #include <matdash/console.hpp>
#endif

#include "Hooks.h"

void mod_main(HMODULE)
{
    #ifdef SHOW_CONSOLE
        matdash::create_console();
    #endif

    Hooks::Init();
}