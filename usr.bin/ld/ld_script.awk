# $Id$

BEGIN {
    split(ARGV[1], s, ".");
    printf "const char *%s = ", s[1];
}

{
    printf "\"";
    gsub("\"", "\\\"");
    printf "%s\\n\"\n", $0;
}

END {
    print ";";
}
