#pragma once

#include "utils.hpp"

namespace lispy::hashmap {

/**
 * @brief (hashmap-get . #"..." key) -> val
 */
LISPY_DEC_STATIC_MACRO(Get);

/**
 * @brief (hashmap-set . key1 val1 key2 #"!") -> .
 */
LISPY_DEC_STATIC_FUNCO(Set);

/**
 * @brief (hashmap-del . key1 key2) -> .
 */
LISPY_DEC_STATIC_FUNCO(Del);

/**
 * @brief (hashmap-size map1 map2) -> 大小之和
 */
LISPY_DEC_STATIC_FUNCO(Size);

/**
 * @brief (hashmap-iter map1 map2) -> 遍历所有表的迭代器
 */
LISPY_DEC_STATIC_FUNCO(Iter);

}
