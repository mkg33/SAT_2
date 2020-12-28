import os
import subprocess
import sys

class bcolors:
    OKGREEN = '\033[92m'
    FAIL = '\033[91m'
    BOLD = '\033[1m'
    ENDC = '\033[0m'

def runForAll(dir, res):
    out = []
    for filename in os.listdir(dir):
        if filename.endswith(".cnf"):
            fOut    = []
            result = subprocess.run(["./solver.out", dir + filename], capture_output=True, text=True)
            if result.stderr != "" or result.stdout.strip() != res:
                fOut.append("[" + bcolors.FAIL + "FAIL" + bcolors.ENDC + "]")
            else:
                fOut.append("[" + bcolors.OKGREEN + "OK" + bcolors.ENDC + "]")
            fOut.append(filename + ":")
            if (result.stderr != ""):
                fOut.append(result.stderr.strip())
            else:
                fOut.append(result.stdout.strip())
            out.append(fOut)
    width = max(len(word) for row in out for word in row) + 2
    for row in out:
        print ("".join(word.ljust(width) for word in row))

def main(argv):
    if len(argv) < 3 or not argv[1].endswith("/"):
        print(argv[0], " <directory of cnf files including the ending / (slash)> <expected result>")
        sys.exit(2)
    else:
        runForAll(argv[1], argv[2])

if __name__ == "__main__":
    main(sys.argv)