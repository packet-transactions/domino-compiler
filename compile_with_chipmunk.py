#TODO: .c --- (canonocalizer) ---> canonicalizer_file.c --- (grouper)
#  ---> a_group_of_domino_files.cc --- (domino_to_chipmunk) ---> .sk files

import sys
import re
import subprocess


def main(argv):
    """Main program."""
    if len(argv) < 3:
        print("Usage: python3 " + argv[0] + " <program file> <group size>")
        exit(1)
    # program_file means the original domino program
    program_file = str(argv[1])
    group_size = str(argv[2])

    #TODO: run canonicalizer
    (ret_code,
     output) = subprocess.getstatusoutput("canonicalizer " + program_file)
    #TODO: output canonicalizer file to /tmp/file_name.cc
    canonicalizer_file = "/tmp/" + program_file[program_file.rfind('/') + 1:]
    with open(canonicalizer_file, 'w') as file:
        file.write(output)
    #TODO: run grouper to generate files with different group strategies output to /tmp folder
    # the name of group_files is /tmp/<file_name>_equivalent_(/d).c
    (ret_code,
     output) = subprocess.getstatusoutput("grouper " + canonicalizer_file +
                                          " " + group_size)
    # output restore the total number of group files
    for i in range(int(output)):
        group_file = "/tmp/" + program_file[program_file.rfind(
            '/') + 1:program_file.rfind('.')] + "_equivalent_" + str(i) + ".c"
        (ret_code, output) = subprocess.getstatusoutput("domino_to_chipmunk " +
                                                        group_file)
        chipmunk_file = group_file[:group_file.rfind('.')] + ".sk"
        with open(chipmunk_file, 'w') as file:
            file.write(output)


if __name__ == "__main__":
    main(sys.argv)
