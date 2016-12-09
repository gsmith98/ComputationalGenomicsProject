#!/usr/bin/env python3
import sys

#Total time = 202.767s
#Reported 173672 pairwise alignments, 173916 HSSPs.
#14013 queries aligned.

if __name__ == "__main__":
    timeString = input()
    alignNumString = input()
    queryNumString = input()

    t = timeString.split(" ")[-1].split("s")[0]
    a = alignNumString.split(" ")[1]
    a2 = alignNumString.split(" ")[4]
    q = queryNumString.split(" ")[0]

    print("," + t + "," + a + "," + a2 + "," + q)


