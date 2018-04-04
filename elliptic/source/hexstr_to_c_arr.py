#!/usr/bin/env python3

import sys

def as_c_arr(s):
     out = "{ "
     s_list = list(s)
     if len(s_list) % 2 != 0:
         s_list.insert(0, '0')
     for i in range(0, len(s_list), 2):
         out += "0x" + (s_list[i] + s_list[i+1]).upper()
         out += ", "
     out = out[:-2]
     out += " };"
     return out


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("You forgot to give me string, man!")
        print("Usage: {} <hexstring1> [hexstring2, ...]".format(sys.argv[0]))
    else:
        for i in sys.argv[1:]:
            print(as_c_arr(i))

