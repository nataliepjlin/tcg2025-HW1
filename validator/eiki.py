# Chinese Dark Chess: Eiki
# ----------------------------------
# The Yama will judge your code.

import subprocess
import time
import os
import sys
import csv
import resource
import signal
from enum import Enum

class Judgement(Enum):
    PASS              = 1
    TIMER_MISMATCH    = 2
    TIMEOUT           = 3
    SUBOPTIMAL        = 4
    HIGHLY_SUBOPTIMAL = 5
    INVALID           = 6
    DEATH             = 7

# Runs make
def summon_wakasagi(target=None):
    cmd = ["make"]
    if target:
        cmd.append(target)
    subprocess.run(cmd, cwd="../wakasagihime", check=True)

def parse_testcases(f):
    testcases = []
    lines = [line.strip() for line in f if line.strip() and not line.startswith('#')]

    for i in range(0, len(lines), 3):
        name = lines[i]
        fen = lines[i + 1]
        size = int(lines[i + 2])
        testcases.append((name, fen, size))

    return testcases

def examiner(examinee, judger, problem, expected):
    def set_limit():
        LIMIT = 10 * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (LIMIT, LIMIT))

    proc = subprocess.Popen(
        [examinee],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        preexec_fn=set_limit,
        bufsize=0,
    )
    time.sleep(0.1) # wait for init

    start = time.perf_counter()
    try:
        output, err = proc.communicate(input=problem, timeout=10)
        end = time.perf_counter()
        elapsed = end - start
    except subprocess.TimeoutExpired:
        print("Times up!")
        proc.kill()
        return Judgement.TIMEOUT, 0

    print(f"Timed @ {elapsed:.04f}s")

    # status
    if proc.returncode < 0:
        print(f"Program terminated with {signal.Signals(-proc.returncode).name}")
        return Judgement.DEATH, 0

    answer = output.splitlines()
    if len(answer) == 0:
        return Judgement.INVALID, 0

    # time
    try:
        claimed_time = float(answer[0])
        if abs(elapsed - claimed_time) > 0.1:
            return Judgement.TIMER_MISMATCH, claimed_time
    except ValueError:
        return Judgement.INVALID, 0

    # move validity
    message = problem + '\n' + '\n'.join(answer[2:]) + '\n'
    try:
        validation = subprocess.run([judger], input = message, capture_output = True, text = True, timeout = 1)
    except subprocess.TimeoutExpired:
        # incomplete moveset
        validation.kill()
        return Judgement.INVALID, claimed_time

    if validation.returncode != 0:
        return Judgement.INVALID, claimed_time

    # move count
    try:
        claimed_move = int(answer[1])
    except ValueError:
        return Judgement.INVALID, 0

    if claimed_move != expected:
        if claimed_move == expected + 1:
            return Judgement.SUBOPTIMAL, claimed_time
        return Judgement.HIGHLY_SUBOPTIMAL, claimed_time

    return Judgement.PASS, claimed_time

def main():
    refresh = ("--refresh" in sys.argv)

    try:
        with open("testcases", 'r') as t:
            # Fetch testcases
            testcases = parse_testcases(t)

            # Make regular Wakasagi
            if refresh or not os.path.isfile("../wakasagihime/wakasagi"):
                summon_wakasagi()

            # Make validating Wakasagi
            if not os.path.isfile("../wakasagihime/valisagi"):
                summon_wakasagi(target = "validate")

            print("===== Judgement =====")
            memory_hint = False

            for name, fen, size in testcases:
                print(f"{name} - ", end = "")
                result, time = examiner("../wakasagihime/wakasagi", "../wakasagihime/valisagi", fen, size)
                match result:
                    case Judgement.DEATH:
                        print(f"❌ YOU DIED")
                        memory_hint = True
                    case Judgement.TIMER_MISMATCH:
                        print(f"❌ FAIL - Mistimed (Claimed {time}s)")
                    case Judgement.TIMEOUT:
                        print(f"❌ FAIL - Timeout")
                    case Judgement.SUBOPTIMAL:
                        print("❌ MEH - Suboptimal")
                    case Judgement.HIGHLY_SUBOPTIMAL:
                        print("❌ FAIL - Too many steps")
                    case Judgement.INVALID:
                        print("❌ FAIL - Invalid moves")
                    case Judgement.PASS:
                        print("⭐ Well done!")

            if memory_hint:
                print("Hint: Memory allocations will fail when the limit is exceeded. You might want to check the return values of your mallocs.")

    except IOError:
        print(f"Where are my testcases?")

main()
