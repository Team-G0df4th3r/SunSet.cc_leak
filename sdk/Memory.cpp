// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Memory.h"
//#include "..\utils\.h"

template <typename T>
static constexpr auto relativeToAbsolute(int* address) noexcept
{
    return reinterpret_cast<T>(reinterpret_cast<char*>(address + 1) + *address);
}

#define FIND_PATTERN(type, ...) \
reinterpret_cast<type>(findPattern(__VA_ARGS__))

void Memory::initialize() noexcept
{
    auto temp = FIND_PATTERN(std::uintptr_t*, ("client"), ("\xB9????\xE8????\x8B\x5D\x08"), 1);
    hud = *temp;
    findHudElement = relativeToAbsolute<decltype(findHudElement)>(reinterpret_cast<int*>(reinterpret_cast<char*>(temp) + 5));
    clearHudWeapon = FIND_PATTERN(decltype(clearHudWeapon), ("client.dll"), ("55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 34"));
    itemSchema = relativeToAbsolute<decltype(itemSchema)>(FIND_PATTERN(int*, ("client"), ("\xE8????\x0F\xB7\x0F"), 1));
    equipWearable = FIND_PATTERN(decltype(equipWearable), ("client"), ("\x55\x8B\xEC\x83\xEC\x10\x53\x8B\x5D\x08\x57\x8B\xF9"));
}