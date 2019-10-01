#!/bin/python3

# Created by naya.vu, 2019
# WTFPL

import matplotlib.pyplot as plt

# Reads raw IR pulses, parses data as numbers
# Returns a hash of lists containing data for different controller states in format [ state => [row1, row2]]
def read_data(filename):
    data = {}
    state = ()

    with open(filename) as f:
        for l in f:
            if l.startswith('# '):
                state = l.strip()
                continue
            if not state or not l.startswith('Raw'):
                continue
            if not state in data:
                data[state] = []

            values = l.split(':')[1]
            row = [int(i.strip()) for i in values.strip().split(' ')]

            data[state].append(row)
    return data

def measure_unit_of_time(row):
    # Unit of time T is a time interval to calculate bits:
    # 0 is 1T pulse followed by 1T silece
    # 1 is 1T pulse followed by 3T silence
    # see http://users.telenet.be/davshomepage/panacode.htm for details
    epsilon = 0.05  # 5%
    distribution = {}

    for i in range(1, len(row)):
        t = row[i]
        distr = None
        for k in distribution.keys():
            if abs(t - k) / t <= epsilon:
                distr = k
                break
        if distr:
            distribution[distr] = (distribution[distr][0] + t, distribution[distr][1] + 1)
        else:
            distribution[t] = (t, 1)
            
    avg_distribution = []
    for diff in distribution.items():
        (s, cnt) = diff[1]
        avg_distribution.append((s/cnt, cnt))

    avg_distribution = sorted(avg_distribution, key = lambda x: x[1], reverse=True)

    # first two times are minimal ones representing IR pulse and silence
    # IR pulse may differ from silence because of IR receiver I used to collect the data

    t = (avg_distribution[0][0] + avg_distribution[1][0])/2
    print("Pulse time: ", avg_distribution[0][0])
    print("Silence time: ", avg_distribution[1][0])
    print("Unit of time: ", t)
    print("Pulse or silence times:", avg_distribution)
    return t


def plot(*rows):
    for p in rows:
        plt.plot(p[0], p[1], drawstyle='steps-pre')
    plt.show()

# For a given timing sequence, generate a list of (time, 1 or 0) considering that every next impulse means change state (0 => 1 and 1 => 0)
def make_square_signal(row):
    x = []
    y = []
    i = 0
    t = 0 # timings in row are relative, to display them correctly we need absolute values
    for p in row:
        x.append(t + p)
        t += p
        y.append(i)
        i = 0 if i == 1 else 1
    return x, y


def plot_signal(*rows):
    squares = [make_square_signal(row) for row in rows]
    plot(*squares)

def measure_pulses(row, unit_of_time):
    pulses = []
    lvl = 1
    for i in range(1, len(row)):
        t = row[i]
        pulses_count = round(t / unit_of_time)
        pulses.append((pulses_count, lvl))
        lvl = 0 if lvl == 1 else 1
    return pulses


def analyze_bits(row):
    # ignore first 8 pulses and 4 silences, they are a signature of the beginning of transmission
    # see http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152
    if rows[0] != (8,1) and row[1] != (4, 0):
        raise Exception('Unknown starting signature')

    # all next pulses should come in a format:
    # - 1 pulse followed by 1 silence for 0
    # - 1 pulse followed by 3 silences for 1

    # ignore the last pulse (it's a stop pulse)
    bits = []
    for i in range(2, len(row)-1, 2):

        if row[i] != (1, 1):
            if row[i] == (2, 1):
                # most likely it's 1T pulse which somehow was measured as 2T
                row[i] = (1, 1)
            else:
                raise Exception('Could not parse bit for pulse ' + str(i) + ' - ' + str(row[i]))

        if row[i + 1][1] != 0:
            raise Exception('Could not parse bit for pulse ' + str(i + 1) + ' - ' + str(row[i + 1]))

        pulses_count = row[i + 1][0]
        if pulses_count == 1:
            bits.append(0)
        elif pulses_count == 3:
            bits.append(1)
        elif pulses_count > 3:
            # too many silences (23 silences) - it's end of header, a stop pulse. ignore it

            bits.append('-')

            # recursively try to parse
            bits += analyze_bits(row[i+2:])

            break

        else:
            raise Exception('Unexpected pulse ' + str(i + 1) + ' - ' + str(row[i + 1]))
    return bits


def bits_to_str(bits):
    i = 0
    chars = []
    for c in bits:
        if c == '-':
            chars.append('-')
            chars.append(' ')
            continue

        chars.append('0' if c == 0 else '1')
        i += 1
        if i == 8:
            i = 0
            chars.append(' ')
    return ''.join(chars)

#################################################

# 1. read and parse data
data = read_data("raw_ir_data.txt")

# 2. pick up some row for analysis
first_row = next(iter(data.values()))[0]

# 3. plot a row to see how it looks
plot_signal(first_row)

# 4. measure unit of time of the signal
unit_of_time = measure_unit_of_time(first_row)

# 5. measure pulses
pulses = {}
for state, rows in data.items():
    p = measure_pulses(rows[0], unit_of_time)
    for i in range(1, len(rows)):
        p2 = measure_pulses(rows[i], unit_of_time)
        if p != p2:
            raise Exception('Pulses for the same state do not match: ' + state)
    pulses[state] = p

#print(pulses)

# 6. analyze pulses, convert them to bits
bits = {}
for state, pulse in pulses.items():
    bits[state] = analyze_bits(pulse)

    print(state)
    print(bits_to_str(bits[state]))


# the protocol:
# 8xON, 4xOFF, 8 bytes of header (0X4004072000000060), 1xON, 23xOFF, 8xON, 4xOFF, 19 bytes data, 1xON
