#pragma once

#include "utils.hpp"

namespace lispy::vector {

LISPY_DEC_STATIC_MACRO(Get);

LISPY_DEC_STATIC_FUNCO(Set);

LISPY_DEC_STATIC_FUNCO(Size);

/**
 * @brief 向某一位置插入多个连续元素
 *
 * 用法：
 * (vector-insert [] 0 "a" "b" "c") 得到 ["a" "b" "c"]
 */
LISPY_DEC_STATIC_FUNCO(Insert);

/**
 * @brief 指定多个下标，删除元素
 */
LISPY_DEC_STATIC_FUNCO(Remove);

}
