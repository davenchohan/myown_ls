# My Own ls Command

Created my own ls command that runs on Unix based machines such as Linux

Added the choice for some options and to ls multiple directories at once.

Output is sorted in lexicographical order.

## Format

Format is ./myls [options] [file list]

[options]: Optional options from the list below. May be specified in any order or grouping, such as “”, “-i”, “-i -l -R”, “-iRl” “-R -li”, “-lR”

    -i: Print the index number of each file

    -l: Use a long listing format

    -R: List subdirectories recursively. (Make sure that recursion must not cause any infinite loop)

[file list]: Optional space separated list of paths to files or directories to display.

