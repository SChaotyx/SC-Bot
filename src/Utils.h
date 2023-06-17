#pragma once

template <typename T, typename R>
T cast(R const v) { return reinterpret_cast<T>(v); }