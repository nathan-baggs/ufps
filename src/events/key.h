#pragma once

#include <string>

#include <windows.h>

namespace ufps
{

enum class Key
{
    ESC = 0x1b,
    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4a,
    K = 0x4b,
    L = 0x4c,
    M = 0x4d,
    N = 0x4e,
    O = 0x4f,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5a,
    SPACE = VK_SPACE,
    F1 = VK_F1
};

enum class KeyState
{
    UP,
    DOWN
};

inline auto to_string(Key obj) -> std::string
{
    switch (obj)
    {
        using enum ufps::Key;
        case ESC: return "ESC";
        case A: return "A";
        case B: return "B";
        case C: return "C";
        case D: return "D";
        case E: return "E";
        case F: return "F";
        case G: return "G";
        case H: return "H";
        case I: return "I";
        case J: return "J";
        case K: return "K";
        case L: return "L";
        case M: return "M";
        case N: return "N";
        case O: return "O";
        case P: return "P";
        case Q: return "Q";
        case R: return "R";
        case S: return "S";
        case T: return "T";
        case U: return "U";
        case V: return "V";
        case W: return "W";
        case X: return "X";
        case Y: return "Y";
        case Z: return "Z";
        case F1: return "F1";
        case SPACE: return "SPACE";
    }

    return "?";
}

inline auto to_string(KeyState obj) -> std::string
{
    switch (obj)
    {
        using enum ufps::KeyState;
        case UP: return "UP";
        case DOWN: return "DOWN";
        default: return "UNKNOWN STATE";
    }
}

}
