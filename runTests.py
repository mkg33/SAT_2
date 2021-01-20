import os
import subprocess
import sys

class bcolors:
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    BOLD = '\033[1m'
    ENDC = '\033[0m'

def runForAll(exe, dir, res):
    out = []
    heuristics = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    files = sorted(os.listdir(dir))
    for heuristic in heuristics:
        print("HEURISTIC: ", heuristic)
        for filename in files:
            if filename.endswith(".cnf"):
                fOut    = []
                result = subprocess.run(["time", "./" + exe, dir + "/" + filename, str(heuristic)], capture_output=True, text=True)
                execTime = result.stderr.strip().split()[2]
                output   = result.stdout.strip().split()[0]
                if output != res:
                    fOut.append("[" + bcolors.FAIL + "FAIL" + bcolors.ENDC + "]")
                else:
                    fOut.append("[" + bcolors.OKGREEN + "OK" + bcolors.ENDC + "]")
                fOut.append(execTime)
                fOut.append(filename + ":")
                fOut.append(output)
                out.append(fOut)
        width = max(len(word) for row in out for word in row) + 2
        for row in out:
            print ("".join(word.ljust(width) for word in row))

def main(argv):
    if len(argv) < 4:
        print(argv[0], " <executable> <directory of cnf files> <expected result>")
        sys.exit(2)
    else:
        runForAll(argv[1], argv[2], argv[3])

if __name__ == "__main__":
    main(sys.argv)