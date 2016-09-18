#!/usr/bin/env python3

import numpy as np
import pylab as plot
import sys
import argparse

parser = argparse.ArgumentParser(description="Generate a performance graph")
parser.add_argument("--base_data", required=True,
                    help="The path to the base data")
parser.add_argument("--base_title", required=True,
                    help="The title of the base data plot")

parser.add_argument("--changed_data", required=True,
                    help="The path to the changed data")
parser.add_argument("--changed_title", required=True,
                    help="The title of the changed data plot")

parser.add_argument("--fps", action="store_true",
                    help="Draw plot as FPS, default is frametime")
parser.add_argument("--save_img", help="Saves the generated graph to the specified file path.")
args = parser.parse_args()

averageTime = 0.2

MICROSECONDS_PER_SECOND = 1000000.

def average(time, frametime):
    startIndex = 0
    currentTime = time[startIndex]

    outTime = []
    outFPS = []

    for i in range(startIndex + 1, len(time)):
        if time[i] - currentTime >= averageTime:
            val = np.mean(frametime[startIndex:i])

            outTime.append(time[startIndex])
            outFPS.append(val)

            startIndex = i
            currentTime = time[i]

    return outTime, outFPS


def adjustData(data):
    times = data[2:, 0] / MICROSECONDS_PER_SECOND
    frametimes = data[2:, 1] / MICROSECONDS_PER_SECOND

    times = times - times[0]

    if args.fps:
        frametimes = 1 / frametimes

    return average(times, frametimes)


normalData = np.genfromtxt(args.base_data, delimiter=';')
changedData = np.genfromtxt(args.changed_data, delimiter=';')

nT, nF = adjustData(normalData)
cT, cF = adjustData(changedData)

plot.plot(nT, nF, label=args.base_title)
plot.plot(cT, cF, label=args.changed_title)

plot.xlabel("Mission time")
if args.fps:
    plot.ylabel("FPS")
else:
    plot.ylabel("Frametime")

plot.legend(loc=1)

if args.save_img:
    plot.savefig(args.save_img)
else:
    plot.show()
