#!/home/retrobsd/litebsd/contrib/mksh/mksh
LC_ALL=C PATH='/usr/local/bin:/usr/bin:/bin:/usr/local/games:/usr/games'; export LC_ALL PATH
test -n "$KSH_VERSION" || exit 1
set -A check_categories --  shell:legacy-no int:32
pflag='/home/retrobsd/litebsd/contrib/mksh/mksh'
sflag='./check.t'
usee=0 Pflag=0 Sflag=0 uset=0 vflag=1 xflag=0
while getopts "C:e:fPp:QSs:t:v" ch; do case $ch {
(C)	check_categories[${#check_categories[*]}]=$OPTARG ;;
(e)	usee=1; eflag=$OPTARG ;;
(f)	check_categories[${#check_categories[*]}]=fastbox ;;
(P)	Pflag=1 ;;
(+P)	Pflag=0 ;;
(p)	pflag=$OPTARG ;;
(Q)	vflag=0 ;;
(+Q)	vflag=1 ;;
(S)	Sflag=1 ;;
(+S)	Sflag=0 ;;
(s)	sflag=$OPTARG ;;
(t)	uset=1; tflag=$OPTARG ;;
(v)	vflag=1 ;;
(+v)	vflag=0 ;;
(*)	xflag=1 ;;
}
done
shift $((OPTIND - 1))
set -A args -- './check.pl' -p "$pflag"
x=
for y in "${check_categories[@]}"; do
x=$x,$y
done
if [[ -n $x ]]; then
args[${#args[*]}]=-C
args[${#args[*]}]=${x#,}
fi
if (( usee )); then
args[${#args[*]}]=-e
args[${#args[*]}]=$eflag
fi
(( Pflag )) && args[${#args[*]}]=-P
if (( uset )); then
args[${#args[*]}]=-t
args[${#args[*]}]=$tflag
fi
(( vflag )) && args[${#args[*]}]=-v
(( xflag )) && args[${#args[*]}]=-x	# force usage by synerr
if [[ -n $TMPDIR && -d $TMPDIR/. ]]; then
args[${#args[*]}]=-T
args[${#args[*]}]=$TMPDIR
fi
print Testing mksh for conformance:
grep -F -e Mir''OS: -e MIRBSD "$sflag"
print "This shell is actually:\n\t$KSH_VERSION"
print 'test.sh built for mksh R52 2015/12/12'
cstr='$os = defined $^O ? $^O : "unknown";'
cstr="$cstr"'print $os . ", Perl version " . $];'
for perli in $PERL perl5 perl no; do
if [[ $perli = no ]]; then
print Cannot find a working Perl interpreter, aborting.
exit 1
fi
print "Trying Perl interpreter '$perli'..."
perlos=$($perli -e "$cstr")
rv=$?
print "Errorlevel $rv, running on '$perlos'"
if (( rv )); then
print "=> not using"
continue
fi
if [[ -n $perlos ]]; then
print "=> using it"
break
fi
done
(( Sflag )) || echo + $perli "${args[@]}" -s "$sflag" "$@"
(( Sflag )) || exec $perli "${args[@]}" -s "$sflag" "$@"
# use of the -S option for check.t split into multiple chunks
rv=0
for s in "$sflag".*; do
echo + $perli "${args[@]}" -s "$s" "$@"
$perli "${args[@]}" -s "$s" "$@"
rc=$?
(( rv = rv ? rv : rc ))
done
exit $rv
