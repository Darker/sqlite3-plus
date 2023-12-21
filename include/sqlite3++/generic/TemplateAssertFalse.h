#pragma once
#include <type_traits>

namespace sqlitepp
{
// Use this to force a false assertion only when template is actually instantiated
template<typename T>
struct TemplateAssertFalse : std::false_type
{};

}