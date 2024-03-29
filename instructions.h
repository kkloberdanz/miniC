/*
 * Author: Kyle Kloberdanz
 * Project Start Date: 27 Nov 2018
 * License: GNU GPLv3 (see LICENSE.txt)
 *     This file is part of minic.
 *
 *     minic is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     minic is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with minic.  If not, see <https://www.gnu.org/licenses/>.
 * File: instructions.h
 */

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    NOP,
    PUSH,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
    NOT,
    PRINTI,
    PRINTC,
    READC,
    POP,
    LOAD,
    SAVE,
    J,
    JZ,
    JLEZ,
    JNZ,
    CALL,
    RET,
    POPC,
    HALT
} inst_t;

extern const char *inst_names[];

bool requires_immediate(inst_t inst);

bool is_jump(inst_t inst);

#endif /* INSTRUCTIONS_H */
