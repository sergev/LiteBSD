.\" @(#) kermit.1 9.0.302 2011/08/20 Columbia University
.TH KERMIT 1 "JULY 2011" "User Manuals"
.na
.SH NAME
kermit \-
.B C\(hyKermit 9.0:
transport\(hy and platform\(hyindependent
interactive and scriptable communications software.
.IP

This document is intended to give the beginner sufficient information to
make basic (if not advanced) use of C\(hyKermit 9.0.  Although it might be
rather long for a Unix manual page, it's still far shorter than the
C\(hyKermit manual, which should be consulted for advanced topics such as
customization, character\(hysets, scripting, etc. We also attempt to provide
a clear structural overview of C\(hyKermit's many capabilities, functional
areas, states, and modes and their interrelation, that should be helpful to
beginners and veterans alike, as well as to those upgrading to version 9.0
from earlier releases.
.PP
This document is also available as a Web page at:
.IP
http://www.columbia.edu/kermit/ckututor.html
.SH DESCRIPTION
C\(hyKermit is an all\(hypurpose communications software package from the Kermit
Project at Columbia University that:
.PP
.nf
\(bu	Is portable to many platforms, Unix and non\(hyUnix alike.
.br
\(bu	Can make both serial and network connections.
.br
\(bu	Can conduct interactive terminal sessions over its connection.
.br
\(bu	Can transfer text or binary files over the same connection.
.br
\(bu	Can convert character sets in the terminal session.
.br
\(bu	Can convert character sets during text\(hyfile file transfer.
.br
\(bu	Is customizable in every aspect of its operation.
.fi
.PP
C\(hyKermit is a modem program, a Telnet client, an Rlogin client, an FTP
client, an HTTP client, and on selected platforms, also an X.25 client. It
can make its own secure Internet connections using IETF\(hyapproved security
methods including Kerberos IV, Kerberos V, SSL/TLS, and SRP and it can also
make SSH connections through your external SSH client application. It can
be the far\(hyend file\(hytransfer or client/server partner of your desktop
Kermit client. It can also accept incoming dialed and network connections.
It can even be installed as an Internet service on its own standard TCP
socket, 1649 [RFC2839, RFC2840].
.PP
And perhaps most important, everything you can do "by hand" (interactively)
with C\(hyKermit, can be "scripted" (automated) using its built\(hyin
cross\(hyplatform transport\(hyindependent script programming language, which
happens to be identical to its interactive command language.
.PP
This manual page offers an overview of C\(hyKermit 9.0 for Unix ("Unix" is an
operating system family that includes AIX, DG/UX, FreeBSD, HP\(hyUX, IRIX,
Linux, Mac OS X, NetBSD, OpenBSD, Open Server, Open Unix, QNX, Solaris,
SunOS, System V R3, System V R4, Tru64 Unix, Unixware, Xenix, and many
others). For thorough coverage, please consult the published C\(hyKermit
manual and supplements (see DOCUMENTATION below). For further information
about C\(hyKermit, Kermit software for other platforms, and Kermit manuals,
visit the Kermit Project website:
.PP
  http://www.columbia.edu/kermit/
.PP
This is a longer\(hythan\(hyaverage manual page, and yet it barely scratches the
surface. Don't be daunted. C\(hyKermit is a large and complex package,
evolving over decades of practice and experience, but that doesn't mean
it's hard to learn or use. Its most commonly used functions are explained
here with pointers to additional information elsewhere.
.SH SYNOPSIS
.B kermit [
.I filename
.B ] [
.I options
.B ] [ {=,\-\-,+}
.I text
.B ] ]
.PP
or:
.PP
.B kermit 
.I URL
.PP
If the first command\(hyline argument is the name of a file, interactive\(hymode
commands are executed from the file. The '=' (or "\-\-") argument tells
Kermit not to parse the remainder of the command line, but to make the
words following '=' available as \e%1, \e%2, ... \e%9. The "+" argument is
like "=" but for use in "kerbang scripts" (explained below). A second
command\(hyline format allows the one and only argument to be a Telnet, FTP,
HTTP, or IKSD URL.
.PP
Order of execution:
.TP
 1.
The command file (if any).
.TP
.nf
 2.
The initialization file, if any, unless suppressed with \-Y.
.fi
.TP
 3.
The customization file (if it is executed by the initialization file).
.TP
 4.
The command\(hyline URL (if any, and if so, execution stops here).
.TP
 5.
Command\(hyline options (if any).
.TP
 6.
Interactive commands.
.PP
Some command\(hyline options can cause actions (such as \-s to send a file);
others just set parameters. If any action options are included on the
command line, Kermit exits when finished unless also given the \-S ("stay")
option. If no action options are given, no initialization or command files
contained an EXIT or QUIT command, and no fatal errors occurred, Kermit
issues its prompt and waits for you to type commands.
.IP
Bear in mind that C\(hyKermit can be built with selected features
disabled, and also that certain features are not available on all
platforms. For example, C\(hyKermit can't be built with TCP/IP
support on a platform that does not have TCP/IP header files and
libraries (and even if Kermit does include TCP/IP support, it
can't be used to make TCP/IP connections on a computer that does
not have a TCP/IP stack installed). If your version of 
C\(hyKermit lacks a feature mentioned here, use its SHOW FEATURES command to
see what might have been excluded.
.PP
C\(hyKermit has three kinds of commands: regular single\(hyletter command\(hyline
options, extended\(hyformat command\(hyline options, and interactive commands.
.PP
Like most Unix commands, C\(hyKermit can be be given options on the command
line. But C\(hyKermit also can be used interactively by giving it commands
composed of words, which are more intuitive than cryptic command\(hyline
options, and more flexible too. In other words, you don't have to use
C\(hyKermit's command\(hyline options, but they are available if you want to. (By
the same token, you don't have to use its interactive commands either \(hy\(hy
you can use either or both in any combination.)
.PP
C\(hyKermit is generally installed in the PATH as "kermit", and therefore is
invoked by typing the word "kermit" (lowercase) at the shell prompt, and
then pressing the Return or Enter key. If you wish to include command\(hyline
options, put them after the word "kermit" but before pressing Return or
Enter, separated by spaces, for example:
.PP
  $ kermit \-s ckermit.tar.gz
.PP
('$' is the shell prompt; "kermit \-s ckermit.tar.gz" is what you type,
followed by Return or Enter.)
.SH OPTIONS
Here is a list of C\(hyKermit's single\(hyletter command\(hyline options, which
start with a single dash (\-), in ASCII ("alphabetical") order. Alphabetic
case is significant (\-A is not the same as \-a).  Action options are 
tagged "ACTION".
.TP
\-0
(digit zero)  100% transparent Connect state for
"in\(hythe\(hymiddle" operation: 8 bits, no parity, no
escape character, everything passes through.
.TP
\-8
(digit eight)  Connection is 8\(hybit clean (this is the
default in C\(hyKermit 8.0 and later). Equivalent to the EIGHTBIT
command, which in turn is a shortcut for SET TERMINAL
BYTESIZE 8, SET COMMAND BYTESIZE 8, SET PARITY NONE.
.TP
\-9 arg
(digit nine)  Make a connection to an FTP server.
Equivalent to the FTP OPEN command.
Argument: IP\(hyaddress\(hyor\(hyhostname[:optional\(hyTCP\(hyport].
NOTE: C\(hyKermit also has a separate FTP command\(hyline
personality, with regular FTP\(hylike command\(hyline
syntax. More about this below.
.TP
\-A
Kermit is to be started as an Internet service (IKSD)
(only from inetd.conf).
.TP
\-B
Kermit is running in Batch or Background (no
controlling terminal). To be used in case Kermit
doesn't automatically sense its background status.
Equivalent to the SET BACKGROUND ON command.
.TP
\-C arg
Interactive\(hymode Commands to be executed.
Argument: Commands separated by commas, list in
doublequotes.
.TP
\-D arg
Delay before starting to send in Remote mode.
Equivalent to the SET DELAY command.
Argument: Number of seconds.
.TP
\-E
Exit automatically when connection closes. Equivalent
to SET EXIT ON\-DISCONNECT ON.
.TP
\-F arg
Use an open TCP connection.
Argument: Numeric file descriptor of open TCP
connection.
Also see: \-j, \-J.
.TP
\-G arg
(ACTION) Get file(s) from server, send contents to standard
output, which normally would be piped to another
process.
Argument: Remote file specification, in quotes if it
contains metacharacters.
Also see: \-g, \-k.
.TP
\-H
Suppress program startup Herald and greeting.
.TP
\-I
Tell Kermit it has a reliable connection, to force streaming to be used where
it normally would not be.  Equivalent to the SET RELIABLE ON command.
.TP
\-J arg
(ACTION) "Be like Telnet." Like \-j but implies \-E.  Argument: IP
hostname/address optionally followed by service.  NOTE: C\(hyKermit also has a
separate Telnet command\(hyline personality, with regular Telnet\(hylike
command\(hyline syntax. More about this below.
.TP
\-L
Recursive directory descent for files in \-s option.
.TP
\-M arg
My user name (for use with Telnet, Rlogin, FTP, etc).
Equivalent to the SET LOGIN USER command.
Argument: Username string.
.TP
\-O
(ACTION) (Uppercase letter O) Be a server for One command only.
Also see: \-x.
.TP
\-P
Don't convert file (Path) names of transferred files.
Equivalent to SET FILE NAMES LITERAL.
.TP
\-Q
Quick Kermit protocol settings. Equivalent to the FAST
command. This is the default in C\(hyKermit 7.0 and later.
.TP
\-R
Remote\(hyonly (this just makes IF REMOTE true).
.TP
\-S
Stay (enter command parser after action options).
.TP
\-T
Force Text mode for file transfer; implies \-V.
Equivalent to SET TRANSFER MODE MANUAL, SET FILE TYPE TEXT.
.TP
\-V
Disable automatic per\(hyfile text/binary switching.
Equivalent to SET TRANSFER MODE MANUAL.
.TP
\-Y
Skip (don't execute) the initialization file.
.TP
\-a arg
As\(hyname for file(s) in \-s, \-r, or \-g.
Argument: As\(hyname string (alternative filename). When
receiving files, this can be a directory name.
.TP
\-b arg
Speed for serial device. Equivalent to SET SPEED.
Argument: Numeric Bits per second for serial
connections.
.TP
\-c
(ACTION) Enter Connect state before transferring files.
.TP
\-d
Create a debug.log file with detailed debugging
information (a second \-d adds timestamps). Equivalent
to LOG DEBUG but takes effect sooner.
.TP
\-e arg
Maximum length for incoming Kermit file\(hytransfer
packets. Equivalent to SET RECEIVE PACKET\-LENGTH.
Argument: Length in bytes.
.TP
\-f
(ACTION) Send a FINISH command to a Kermit server.
.TP
\-g arg
Get file(s) from a Kermit server.
Argument: File specification on other computer, in
quotes if it contains metacharacters. Equivalent to
GET. Also see: \-a, \-G, \-r.
.TP
\-h
(ACTION) Print Help text for single\(hyletter command\(hyline options
(pipe thru 'more' to prevent scrolling).
.TP
\-i
Force binary (Image) mode for file transfer; implies
\-V. Equivalent to SET TRANSFER MODE MANUAL, SET FILE
TYPE BINARY.
.TP
\-j arg
Make a TCP/IP connection.
Argument: IP host name/address and optional service
name or number. Equivalent to the TELNET command.
Also see: \-J, \-F.
.TP
\-k
(ACTION) Receive file(s) to standard output, which normally 
would be piped to another process.
Also see: \-r, \-G.
.TP
\-l arg
(Lowercase letter L) Make a connection on the given
serial communications device. Equivalent to the SET
LINE (SET PORT) command.
Argument: Serial device name, e.g. /dev/ttyS0.
.TP
\-m arg
Modem type for use with the \-l device. Equivalent to
the SET MODEM TYPE command.
Argument: Modem name as in SET MODEM TYPE command,
e.g. "usrobotics".
.TP
\-n
(ACTION) Enter Connect state after transferring files (historical).
.TP
\-p arg
Parity. Equivalent to the SET PARITY command.
Argument: One of the following: e(ven), o(dd), m(ark),
n(one), s(pace).
.TP
\-q
Quiet (suppress most messages). Equivalent to SET QUIET ON.
.TP
\-r
(ACTION) Receive file(s). Equivalent to the RECEIVE command.
Argument: (none, but see \-a)
.TP
\-s arg
Send file(s).
Argument: One or more local file specifications.
Equivalent to the SEND command.
Also see: \-a.
.TP
\-t
(Historical) Xon (Ctrl\-Q) Turnaround character for
half\(hyduplex connections (used on serial linemode
connections to old mainframes). Equivalent to SET
DUPLEX HALF, SET HANDSHAKE XON.
.TP
\-v arg
Window size for Kermit protocol (ignored when
streaming). Equivalanet to SET WINDOW\-SIZE.
Argument: Number, 1 to 32.
.TP
\-w
Incoming files Write over existing files. Equivalent
to SET FILE COLLISION OVERWRITE.
.TP
\-x
(ACTION) Enter server mode. Equivalent to the SERVER command.
Also see: \-O.
.TP
\-y arg
Alternative initialization file.
Argument: Filename.
.TP
\-z
Force foreground behavior. To be used in case Kermit
doesn't automatically sense its foreground status.
Equivalent to the SET BACKGROUND OFF command.
.PP
Extended command\(hyline options (necessary because single\(hyletter ones are
about used up) start with two dashes (\-\-), with words rather than single
letters as option names. If an extended option takes an argument, it is
separated from the option word by a colon (:). Extended options include:

.TP
 \-\-bannerfile:filename
File to display upon startup or IKSD login.
.TP
 \-\-cdfile:filename
File to be sent for display to the client when
server changes directory (filename is relative to
the changed\(hyto directory).
.TP
 \-\-cdmessage:{on,off}
Enable/disable the server CD message feature.
.TP
 \-\-help
Prints usage message for extended options.
.TP
 \-\-helpfile:filename
Designates a file containing custom text to
replace the top\(hylevel HELP command.
.TP
 \-\-nointerrupts
Disables keyboard interrupts.
.TP
 \-\-noperms
Disables the Kermit protocol file Permissions
attribute, to prevent transmission of file
permissions (protection) from sender to receiver.
.TP
 \-\-version
(ACTION) C\(hyKermit prints its version number.
.PP
Plus several other IKSD\(hyOnly options described at:
.PP
  http://www.columbia.edu/kermit/iksd.html
.PP
See the file\(hytransfer section for examples of command\(hyline invocation.
.SH COMMAND LANGUAGE
C\(hyKermit's interactive command language is the subject of a 622\(hypage book
and another several hundred pages of updates, far too much for a manual
page. But it's not hard to get started. At the shell prompt, just type
"kermit" to get C\(hyKermit's interactive command prompt:
.PP
.nf
  $ kermit
  (/current/directory) C\-Kermit>
.fi
.PP
Begin by typing "help" (and then press the Return or Enter key) for a
top\(hylevel overview, read it, and go from there. Your second command should
probably be "intro" (introduction). Note the prompt shows your current
directory (unless you tell Kermit to prompt you with something else).
.PP
Interactive commands are composed mainly of regular English words, usually
in the form of imperative sentences, such as:
.PP
  send oofa.txt
.PP
which tells Kermit to send (transfer) the file whose name is oofa.txt, or:
.PP
  set transfer mode automatic
.PP
which sets Kermit's "transfer mode" to "automatic" (whatever that means).
.PP
While typing commands, you can abbreviate, ask for help (by pressing the
"?" key anywhere in a command), complete keywords or filenames (with the
Tab or Esc key), and edit your typing with Backspace or Delete, Ctrl\-W,
Ctrl\-U, etc. You can also recall previous commands, save your command
history, and who knows what else. Give the INTRO command for details.
.PP
C\(hyKermit has hundreds of commands, and they can be issued in infinite
variety and combinations, including commands for:
.nf
.PP
\(bu	Making connections (SET LINE, DIAL, TELNET, SSH, FTP, ...)
.br
\(bu	Breaking connections (HANGUP, CLOSE)
.br
\(bu	Transferring files (SEND, GET, RECEIVE, MOVE, RESEND, ...)
.br
\(bu	Establishing preferences (SET)
.br
\(bu	Displaying preferences (SHOW)
.br
\(bu	Managing local files (CD, DELETE, MKDIR, DIR, RENAME, TYPE, ...)
.br
\(bu	Managing remote files (RCD, RDEL, RMKDIR, RDIR, ...)
.br
\(bu	Using local files (FOPEN, FCLOSE, FREAD, FWRITE)
.br
\(bu	Programming (TAKE, DEFINE, IF, FOR, WHILE, SWITCH, DECLARE, ...)
.br
\(bu	Interacting with the user (ECHO, ASK, ...)
.br
\(bu	Interacting with a remote computer (INPUT, OUTPUT, ...)
.br
\(bu	Interacting with local programs (RUN, EXEC, PTY, ...)
.br
\(bu	Logging things (LOG SESSION, LOG PACKETS, LOG DEBUG, ...)
.PP
.fi
And of course QUIT or EXIT to get out and HELP to get help, and for
programmers: loops, decision making, variables, arrays, associative arrays,
integer and floating point arithmetic, macros, built\(hyin and user\(hydefined
functions, string manipulation, pattern matching, block structure, scoping,
recursion, and all the rest. To get a list of all C\(hyKermit's commands, type
a question mark (?) at the prompt. To get a description of any command,
type HELP followed by the name of the command, for example:
.PP
  help send
.PP
The command interruption character is Ctrl\-C (hold down the Ctrl key and
press the C key).
.PP
The command language "escape character", used to introduce variable names,
function invocations, and so on, is backslash (\). If you need to include a
literal backslash in a command, type two of them, e.g.:
.PP
  get c:\ek95\ek95custom.ini
.SS Command Files, Macros, and Scripts
A file containing Kermit commands is called a Kermit command file or Kermit
script. It can be executed with Kermit's TAKE command:
.PP
  (/current/dir) C\-Kermit> take commandfile
.PP
(where "commandfile" is the name of the command file). Please don't pipe a
command file into Kermit's standard input (which might or might not work);
if you have Kermit commands in a file, tell Kermit to TAKE the file.
.PP
In Unix only, a Kermit command file can also be executed directly by
including a "kerbang" line as the first line of the file:
.PP
  #!/usr/local/bin/kermit +
.PP
That is, a top line that starts with "#!", followed immediately by the full
path of the Kermit executable, and then, if the Kermit script is to be
given arguments on the command line, a space and a plus sign. The script
file must also have execute permission:
.PP
  chmod +x commandfile
.PP
Except for the " +" part, this is exactly the same as you would do for a
shell script, a Perl script, etc. Here's a simple but useless example
script that regurgitates its arguments (up to three of them):
.PP
  #!/usr/local/bin/kermit +
  if defined \e%1 echo "Argument 1: \e%1"
  if defined \e%2 echo "Argument 2: \e%2"
  if defined \e%3 echo "Argument 3: \e%3"
  if defined \e%4 echo "etc..."
  exit
.PP
If this file is stored in your current directory as "commandfile", then:
.PP
  ./commandfile one two three four five
.PP
prints:
.PP
  Argument 1: one
  Argument 2: two
  Argument 3: three
  etc...
.PP
This illustrates the basic structure of a standalone Kermit script: the
"kerbang line", then some commands. It should end with "exit" unless you
want the Kermit prompt to appear when it is finished. \e%1 is the first
argument, \e%2 the second, and so on.
.PP
You can also create your own commands by defining named macros composed of
other Kermit commands (or macros). For example:
.PP
.nf
  define mydelete {
      local trash
      assign trash \ev(home)trashcan/
      if not defined \e%1 end 1 "Delete what?"
      if wild \e%1 {
          end 1 "Deleting multiple files is too scary"
      }
      if not exist \e%1 end 1 "I can't find \e%1"
      if not directory \em(trash) {
          mkdir \em(trash)
          if fail end 1 "No trash can"
      }
      rename /list \e%1 \em(trash)
  }
  define myundelete {
      local trash
      assign trash \ev(home)trashcan/
      if not defined \e%1 end 1 "Undelete what?"
      if wild \e%1 {
          end 1 "Undeleting multiple files is too hard"
      }
      if not directory \em(trash) end 1 "No trash can"
      if not exist \em(trash)\e%1 {
          end 1 "I can't find \e%1 in trash can"
      }
      rename /list \em(trash)\e%1 .
  }
.PP
.fi
These sample macros are not exactly production quality (they don't handle
filenames that include path segments, they don't handle multiple files,
etc), but you get the idea: you can pass arguments to macros, and they can
check them and make other kinds of decisions. If you put the above lines
into your initialization or customization file (explained below), you'll
have MYDELETE and MYUNDELETE commands available every time you start
Kermit, at least as long as you don't suppress execution of the
initialization file. (Exercise for the reader: Make these macros generally
useful: remove limitations, add trashcan display, browsing, emptying, etc.)
.PP
Kerbang scripts execute without the initialization file. This to keep them
portable and also to make them start faster. If you want to write Kerbang
scripts that depend on the initialization file, include the command
.PP
  take \ev(home).kermrc
.PP
at the desired spot in the script. By the way, \ev(xxx) is a built\(hyin
variable (xxx is the variable name, "home" in this case). To see what
built\(hyin variables are available, type "show variables" at the C\(hyKermit
prompt. To see what else you can show, type "show ?". \em(xxx) is a user
defined variable (strictly speaking, it is a macro used as a variable).
.SS Command List
C\(hyKermit has more than 200 top\(hylevel commands, and some of these, such as
SET, branch off into hundreds of subcommands of their own, so it's not
practical to describe them all here. Instead, here's a concise list of the
most commonly used top\(hylevel commands, grouped by category. To learn about
each command, type "help" followed by the command name, e.g. "help set".
Terms such as Command state and Connect state are explained in subsequent
sections.
.PP
Optional fields are shown in [ brackets ].  "filename" means the
name of a single file. filespec means a file specification that is allowed
to contain wildcard characters like '*' to match groups of files. options
are (optional) switches like /PAGE, /NOPAGE, /QUIET, etc, listed in the
HELP text for each command. Example:
.PP
.nf
  send /recursive /larger:10000 /after:\-1week /except:*.txt *
.fi
.PP
which can be read as "send all the files in this directory and all the ones
underneath it that are larger than 10000 bytes, no more than one week old,
and whose names don't end with ".txt".
.SS
Basic Commands
.RS
.TP
HELP
Requests top\(hylevel help.
.TP
HELP command
Requests help about the given command.
.TP
INTRODUCTION
Requests a brief introduction to C\(hyKermit.
.TP
LICENSE
Displays the C\(hyKermit software copyright and license.
.TP
VERSION
Displays C\(hyKermit's version number.
.TP
EXIT [ number ]
Exits from Kermit with the given
status code. Synonyms: QUIT, E, Q.
.TP
TAKE filename [ parameters... ]
Executes commands from the given
.TP
LOG item [ filename ]
Keeps a log of the given item in the given file.
.TP
[ DO ] macro [ parameters... ]
Executes commands from the given macro.
.TP
SET parameter value
Sets the given parameter to the given value.
.TP
SHOW category
Shows settings in a given category.
.TP
STATUS
Tells whether previous command succeeded or failed.
.TP
DATE [ date\(hyand/or\(hytime ]
Shows current date\(hytime or interprets given date\(hytime.
.TP
RUN [ extern\(hycommand [ parameters... ]                 
Runs the given external command. Synonym: !.
.TP
EXEC [ extern\(hycommand [ params... ]
Kermit overlays itself with the given command.
.TP
SUSPEND
Stops Kermit and puts it in the background. Synonym: Z.
.RE
.SS
Local File Management
.RS
.TP
TYPE [ options ] filename
Displays the contents of the given file.
.TP
MORE [ options ] filename     
Equivalent to TYPE /PAGE (pause after each screenful).
.TP
CAT [ options ] filename      
Equivalent to TYPE /NOPAGE.
.TP
HEAD [ options ] filename     
Displays the first few lines of a given file.
.TP
TAIL [ options ] filename     
Displays the last few lines of a given file.
.TP
GREP [ options ] pattern filespec
Displays lines from files that match
the pattern. Synonym: FIND.
.TP
DIRECTORY [ options ] [filespec ]
Lists files (built\(hyin, many options).
.TP
LS [ options ] [ filespec ]   
Lists files (runs external "ls" command).
.TP
DELETE [ options ] [ filespec ]
Deletes files. Synonym: RM.
.TP
PURGE [ options ] [ filespec ]
Removes backup (*.~n~) files.
.TP
COPY [ options ] [ filespecs... ]
Copies files. Synonym: CP.
.TP
RENAME [ options ] [ filespecs... ]
Renames files. Synonym: MV.
.TP
CHMOD [ options ] [ filespecs... ]
Changes permissions of files.
.TP
TRANSLATE filename charsets [ filename ]
Converts file's character set. Synonym: XLATE.
.TP
CD
Changes your working directory to your home directory.
.TP
CD directory
Changes your working directory to the one given.
.TP
CDUP
Changes your working directory one level up.
.TP
PWD
Displays your working directory.
.TP
BACK
Returns to your previous working directory.
.TP
MKDIR [ directory ]
Creates a directory.
.TP
RMDIR [ directory ]
Removes a directory.
.RE
.SS
Making Connections
.RS
.TP
SET LINE [ options ] devicename                      
Opens the named serial port. Synonym: SET PORT.
.TP
OPEN LINE [ options ] devicename
Same as SET LINE. Synonym: OPEN PORT.
.TP
SET MODEM TYPE [ name ]
Tells Kermit what kind of modem is on the port.
.TP
DIAL [ number ]
Tells Kermit to dial the given phone number with the modem.
.TP
REDIAL
Redials the most recently dialed phone number.
.TP
ANSWER
Waits for and answers an incoming call on the modem.
.TP
AUTHENTICATE [ parameters... ]
Performs secure authentication on a TCP/IP connection.
.TP
SET NETWORK TYPE { TCP/IP, X.25, ... }
Selects network type for subsequent SET HOST commands.
.TP
SET HOST [ options ] host [ port ]                          
Opens a network connection to the given host and port.
.TP
SET HOST * port
Waits for an incoming TCP/IP connection on the given port.
.TP
TELNET [ options ] host
Opens a Telnet connection to the host and enters Connect state.
.TP
RLOGIN [ options ] host
Opens an Rlogin connection to the host and enters Connect state.
.TP
IKSD [ options ] host
Opens a connection to an Internet Kermit Service.
.TP
SSH [ options ] host
Opens an SSH connection to the host and enters Connect state.
.TP
FTP OPEN host [ options ]
Opens an FTP connection to the host.
.TP
HTTP [ options ] OPEN host
Opens an HTTP connection to the host.
.TP
PTY external\(hycommand
Runs the command on a pseudoterminal as if it were a connection.
.TP
PIPE external\(hycommand
Runs the command through a pipe as if it were a connection.
.RE
.SS
Using Connections
.RS
.TP
CONNECT [ options ]
Enters Connect (terminal) state.  Synonym: C.
.TP
REDIRECT command
Redirects the given external command over the connection.
.TP
TELOPT command
Sends a Telnet protocol command (Telnet connections only).
.TP
Ctrl\-\eC
"Escapes back" from Connect state to Command state.
.TP
Ctrl\-\eB
(In Connect state) Sends a BREAK signal (serial or Telnet).
.TP
Ctrl\-\e!
(In Connect state) Enters inferior shell; "exit" to return.
.TP
Ctrl\-\e?
(In Connect state) Shows a menu of other escape\(hylevel options.
.TP
Ctrl\-\eCtrl\-\e
(In Connect state) Type two
Ctrl\-Backslashes to send one of them.
.TP
SET ESCAPE [ character ]
Changes Kermit's Connect\(hystate escape character.
.RE
.SS
Closing Connections
.RS
.TP
HANGUP
Hangs up the currently open
serial\(hyport or network connection.
.TP
CLOSE
Closes the currently open
serial\(hyport or network connection.
.TP
SET LINE (with no devicename)
Closes the currently open
serial\(hyport or network connection.
.TP
SET HOST (with no hostname)
Closes the currently open serial\(hyport or network connection.
.TP
FTP CLOSE
Closes the currently open FTP connection.
.TP
HTTP CLOSE
Closes the currently open HTTP connection.
.TP
EXIT 
Also closes all connections. Synonym: QUIT.
.TP
SET EXIT WARNING OFF
Suppresses warning about open connections on exit or close.
.RE
.SS
File Transfer
.RS
.TP
SEND [ options ] filename [ as\(hyname ]
Sends the given file. Synonym: S.
.TP
SEND [ options ] filespec
Sends all files that match.
.TP
RESEND [ options ] filespec
Resumes an interrupted SEND from the point of failure.
.TP
RECEIVE [ options ] [ as\(hyname ]                    
Waits passively for files to arrive. Synonym: R.
.TP
LOG TRANSACTIONS [ filename ]
Keeps a record of file transfers.
.TP
FAST
Use fast file\(hytransfer settings (default).
.TP
CAUTIOUS
Use cautious and less fast file\(hytransfer settings.
.TP
ROBUST
Use ultra\(hyconservative and slow file\(hytransfer settings.
.TP
STATISTICS [ options ] 
Gives statistics about the most recent file transfer.
.TP
WHERE
After transfer: "Where did my files go?".
.TP
TRANSMIT [ options ] [ofilename ]
Sends file without protocol. Synonym: XMIT.
.TP
LOG SESSION [ filename ]
Captures remote text or files without protocol.
.TP
SET PROTOCOL [ name... ]
Tells Kermit to use an external file\(hytransfer protocol.
.TP
FTP { PUT, MPUT, GET, MGET, ... }
FTP client commands.
.TP
HTTP { PUT, GET, HEAD, POST, ... }
HTTP client commands.
.RE
.SS
Kermit Server
.RS
.TP
ENABLE, DISABLE
Controls which server features can be used by clients.
.TP
SET SERVER
Sets parameters prior to entering Server state.
.TP
SERVER
Enters Server state.
.RE
.SS
Client of Kermit or FTP Server
.RS
.TP
[ REMOTE ] LOGIN [ user password ]         
Logs in to a Kermit server or IKSD that requires it.
.TP
[ REMOTE ] LOGOUT
Logs out from a Kermit server or IKSD.
.TP
SEND [ options ] filename [ as\(hyname ]                   
Sends the given file to the server. Synonyms: S, PUT.
.TP
SEND [ options ] filespec
Sends all files that match.
.TP
RESEND [ options ] filespec
Resumes an interrupted SEND from the point of failure.
.TP
GET [ options ] remote\(hyfilespec
Asks the server to send the given files. Synonym: G.
.TP
REGET [ options ] remote\(hyfilespec
Resumes an interrupted GET from the point of failure.
.TP
REMOTE CD [ directory ]
Asks server to change its working
directory. Synonym: RCD.
.TP
REMOTE PWD [ directory ]
Asks server to display its working directory. Synonym: RPWD.
.TP
REMOTE DIRECTORY [ filespec... ]
Asks server to send a directory listing. Synonym: RDIR.
.TP
REMOTE DELETE [ filespec... ]
Asks server to delete files. Synonym: RDEL.
.TP
REMOTE [ command... ]
(Many other commands: "remote ?" for a list).
.TP
MAIL [ options ] filespec
Sends file(s) to be delivered as e\(hymail (Kermit only).
.TP
FINISH
Asks the server to exit server state (Kermit only).
.TP
BYE
Asks the server to log out and close the connection.
.RE
.SS
Script Programming
.PP
.RS
DEFINE, DECLARE, UNDEFINE, UNDECLARE, ASSIGN, EVALUATE, SEXPRESSION,
ARRAY, SORT, INPUT, OUTPUT, IF, FOR, WHILE, SWITCH, GOTO, ECHO, ASK,
GETC, GETOK, ASSERT, WAIT, SLEEP, FOPEN, FREAD, FWRITE, FCLOSE, STOP,
END, RETURN, LEARN, SHIFT, TRACE, VOID, INCREMENT, DECREMENT, ... For
these and many more you'll need to consult the manual and supplements,
and/or visit the Kermit Script Library, which also includes a brief
tutorial. Hint: HELP LEARN to find out how to get Kermit to write
simple scripts for you.
.RE
.PP
Many of Kermit's commands have synonyms, variants, relatives, and so on.
For example, MSEND is a version of SEND that accepts a list of file
specifications to be sent, rather than just one file specification, and
MPUT is a synonym of MSEND. MOVE means to SEND and then DELETE the source
file if successful. MMOVE is like MOVE, but accepts a list of filespecs,
and so on. These are described in the full documentation.
.PP
Use question mark to feel your way through an unfamiliar command, as in
this example:
.PP
.nf
  C\-Kermit> remote ? One of the following:
   assign     directory  kermit     print      rmdir
   cd         exit       login      pwd        set
   copy       help       logout     query      space
   delete     host       mkdir      rename     type
  C\-Kermit> remote set ? One of the following:
   attributes   file         retry        transfer
   block\-check  receive      server       window
  C\-Kermit> remote set file ? One of the following:
   character\-set  incomplete     record\-length
   collision      names          type
  C\-Kermit> remote set file names ? One of the following:
   converted  literal
  C\-Kermit> remote set file names literal
  C\-Kermit>
.PP
.fi
This is called menu on demand: you get a menu when you want one, but menus
are not forced on you even when know what you're doing. Note that you can
also abbreviate most keywords, and you can complete them with the Tab or
Esc key. Also note that ? works for filenames too, and that you can use it
in the middle of a keyword or filename, not just at the beginning. For
example, "send x?" lists all the files in the current directory whose names
start with 'x'.
.SH INITIALIZATION FILE
In its default configuration, C\(hyKermit executes commands from a file 
called .kermrc in your home directory when it starts, unless it is given the
\-Y or \-y command\(hyline option. Custom configurations might substitute a shared
system\(hywide initialization file. The SHOW FILE command tells what
initialization file, if any, was used. The standard initialization file
"chains" to an individual customization file, .mykermc, in the home directory,
in which each user can establish her/his own preferences, define macros, and
so on.
.PP
Since execution of the initialization file (at least the standard one)
makes C\(hyKermit take longer to start, it might be better not to have an
initialization file, especially now that Kermit's default startup
configuration is well attuned to modern computing and networking \(hy\(hy in
other words, you no longer have do anything special to make Kermit
transfers go fast. So instead of having an initialization file that is
executed every time Kermit starts, you might consider making one or more
kerbang scripts (with names other that .kermrc) that do NOT include an
"exit" command, and invoke those when you need the settings, macro
definitions, and/or scripted actions they contain, and invoke C\(hyKermit
directly when you don't.
.PP
To put it another way... We still distribute the standard initialization
file since it's featured in the manual and backwards compatibility is
important to us. But there's no harm in not using it if you don't need the
stuff that's in it (services directory, dialing directory, network
directory, and associated macro definitions). On the other hand, if there
are settings or macros you want in effect EVERY time you use Kermit, the
initialization file (or the customization file it chains to) is the place
to put them, because that's the only place Kermit looks for them
automatically each time you start it.
.SH MODES OF OPERATION
Kermit is said to be in Local mode if it has made a connection to another
computer, e.g. by dialing it or establishing a Telnet connection to it. The
other computer is remote, so if you start another copy of Kermit on the
remote computer, it is said to be in Remote mode (as long as it has not
made any connections of its own). The local Kermit communicates over the
communications device or network connection, acting as a conduit between
the the remote computer and your keyboard and screen. The remote Kermit is
the file\(hytransfer partner to the local Kermit and communicates only through
its standard input and output.
.PP
At any moment, a Kermit program can be in any of the following states. It's
important to know what they are and how to change from one to the other.
.TP
Command state
In this state, Kermit reads commands from:
.sp
\(bu	Your keyboard; or:
.br
\(bu	A file, or:
.br
\(bu	A macro definition.
.sp
You can exit from Command state back to Unix with the EXIT or QUIT
command (same thing). You can enter Connect state with any of various
commands (CONNECT, DIAL, TELNET, etc). You can enter file transfer
state with commands like SEND, RECEIVE, and GET. You can enter Server
state with the SERVER command. The TAKE command tells Kermit to read
and execute commands from a file. The (perhaps implied) DO command
tells Kermit to read and execute commands from a macro definition.
While in Command state, you can interrupt any command, macro, or
command file by typing Ctrl\-C (hold down the Ctrl key and press the C
key); this normally brings you back to the prompt.
.TP
Shell state
You can invoke an inferior shell or external command from the Kermit
command prompt by using the PUSH, RUN (!), EDIT, or BROWSE command.
While the inferior shell or command is active, Kermit is suspended and
does nothing. Return to Kermit Command state by exiting from the
inferior shell or application.
.TP
Connect state
In this state, which can be entered only when in Local mode (i.e. when
Kermit has made a connection to another computer), Kermit is acting as
a terminal to the remote computer. Your keystrokes are sent to the
remote computer and characters that arrive over the communication
connection are displayed on your screen. This state is entered when
you give a CONNECT, DIAL, TELNET, RLOGIN, or IKSD command. You can
return to command state by logging out of the remote computer, or by
typing:
.sp
  Ctrl\-\ec
.sp
That is: Hold down the Ctrl key and press the backslash key, then let
go of the Ctrl key and press the C key. This is called escaping back.
Certain other escape\(hylevel commands are also provided; type Ctrl\-\e?
for a list. For example, you can enter Shell state with:
.sp
  Ctrl\-\e!
.sp
To send a Ctrl\-\e to the host while in Connect state, type two of them
in a row. See HELP CONNECT and HELP SET ESCAPE for more info.
.TP
Local file\(hytransfer state
In this state, Kermit is sending packets back and forth with the other
computer in order to transfer a file or accomplish some other
file\(hyrelated task. And at the same time, it is displaying its progress
on your screen and watching your keyboard for interruptions. In this
state, the following single\(hykeystroke commands are accepted:
.sp
.RS
.TP
X
Interrupt the current file and go on to the next (if any).
.TP
Z
Interrupt the current file and skip all the rest.
.TP
E
Like Z but uses a "stronger" protocol (use if X or Z don't work).
.TP
Ctrl\-C
Interrupt file\(hytransfer mode (use if Z or E don't work).
.sp
.RE
Kermit returns to its previous state (Command or Connect) when the
transfer is complete or when interrupted successfully by X, Z, E, or
Ctrl\-C (hold down the Ctrl key and press the C key).
.TP
Remote file\(hytransfer state
In this state, Kermit is exchanging file\(hytransfer packets with its
local partner over its standard i/o. It leaves this state
automatically when the transfer is complete. In case you find your
local Kermit in Connect state and the remote one in File\(hytransfer
state (in which it seems to ignore your keystrokes), you can usually
return it to command state by typing three Ctrl\-C's in a row. If that
doesn't work, return your local Kermit to Command state (Ctrl\-\e C) and
type "e\(hypacket" and then press the Return or Enter key; this forces a
fatal Kermit protocol error.
.TP
Remote Server state
This is like Remote File\(hytransfer state, except it never returns
automatically to Command state. Rather, it awaits further instructions
from the client program; that is, from your Local Kermit program. You
can return the Remote Server to its previous state by issuing a
"finish" command to the client, or if you are in Connect state, by
typing three Ctrl\-C's in a row. You can tell the server job to log out
and break the connection by issuing a "bye" command to the client.
.TP
Local Server state
Like Remote\(hyServer state, but in local mode, and therefore with its
file\(hytransfer display showing, and listening for single\(hykey commands,
as in Local File\(hytransfer state. Usually this state is entered
automatically when a remote Kermit program gives a GET command.
.sp
C\(hyKermit, Kermit 95, and MS\(hyDOS Kermit all can switch automatically from
Connect state to Local File\(hytransfer state when you initiate a file
transfer from the remote computer by starting Kermit and telling it to send
or get a file, in which case, Connect state is automatically resumed after
the file transfer is finished.
.sp
Note that C\(hyKermit is not a terminal emulator. It is a communications
application that you run in a terminal window (e.g. console or Xterm). The
specific emulation, such as VT100, VT220, Linux Console, or Xterm, is
provided by the terminal window in which you are running C\(hyKermit. Kermit
95 and MS\(hyDOS Kermit, on the other hand, are true terminal emulators. Why
is C\(hyKermit not a terminal emulator? CLICK HERE to read about it.
.SH MAKING CONNECTIONS
Here is how to make different kinds of connections using interactive Kermit
commands (as noted above, you can also make connections with command\(hyline
options). Note that you don't have to make connections with Kermit. It can
also be used on the far end of a connection as the remote file transfer and
management partner of your local communications software.
.TP
Making a Telnet Connection
At the C\(hyKermit command prompt, simply type:
.sp
.nf
  telnet foo.bar.com
.fi
.sp
(substituting desired hostname or address).  
You can also include a port number:
.sp
.nf
  telnet xyzcorp.com 3000 ; 
.fi
.sp
If the connection is successful, Kermit automically enters Connect
state. When you logout from the remote host, Kermit automatically
returns to its prompt. More info: HELP TELNET, HELP SET TELNET, HELP
SET TELOPT. Also see the IKSD section below.
.TP
Making an Rlogin connection
This is just like Telnet, except you have to be root to do it because
Rlogin uses a privileged TCP port:
.sp
.nf
  rlogin foo.bar.com
.fi
.sp
More info: HELP RLOGIN.
.TP
Making an SSH Connection
Unlike Telnet and Rlogin, SSH connections are not built\(hyin, but
handled by running your external SSH client through a pseudoterminal.
Using C\(hyKermit to control the SSH client gives you all of Kermit's
features (file transfer, character\(hyset conversion, scripting, etc)
over SSH.
.sp
  ssh foo.bar.com
.sp
More info: HELP SSH, HELP SET SSH.
.TP
Dialing with a Modem
If it's an external modem, make sure it is connected to a usable
serial port on your computer with a regular (straight\(hythrough) modem
cable, and to the telephone jack with a telephone cable, and that it's
turned on. Then use these commands:
.sp
.nf
  set modem type usrobotics  ; Or other supported type
  set line /dev/ttyS0        ; Specify device name
  set speed 57600            ; Or other desired speed
  set flow rts/cts           ; Most modern modems support this
  set dial method tone       ; (or pulse)
  dial 7654321               ; Dial the desired number
.fi
.sp
Type "set modem type ?" for a list of supported modem types. If you
omit the SET MODEM TYPE command, the default type is
"generic\(hyhigh\(hyspeed", which should work for most modern AT\(hycommand\(hyset
modems. If the line is busy, Kermit redials automatically. If the call
does not succeed, use "set dial display on" and try it again to watch
what happens. If the call succeeds, Kermit enters Connect state
automatically and returns to its prompt automatically when you log out
from the remote computer or the connection is otherwise lost.
.sp
You can also dial from a modem that is accessible by Telnet, e.g. to a
reverse terminal server. In this case the command sequence is:
.sp
.nf
  set host ts.xxx.com 2000   ; Terminal\(hyserver and port
  set modem type usrobotics  ; Or other supported type
  set dial method tone       ; (or pulse)
  dial 7654321               ; Dial the desired number
.fi
.sp
If the terminal server supports the Telnet Com Port Option, RFC 2217,
you can also give serial\(hyport related commands such as SET SPEED, SET
PARITY, and so on, and Kermit relays them to the terminal server using
the protocol specified in the RFC.
.sp
More info: HELP SET MODEM, HELP SET LINE, HELP SET SPEED, HELP SET
FLOW, HELP DIAL, HELP SET DIAL, HELP SET MODEM, HELP SET 
CARRIER\-WATCH, SHOW COMMUNICATIONS, SHOW MODEM, SHOW DIAL.
.TP
Direct Serial Port
Connect the two computers, A and B, with a null modem cable (or two
modem cables interconnected with a null\(hymodem adapter or modem
eliminator). From Computer A:
.sp
.nf
  set modem type none   ; There is no modem
  set line /dev/ttyS0   ; Specify device name
  set carrier\-watch off ; If DTR CD are not cross\(hyconnected
  set speed 57600       ; Or other desired speed
  set flow rts/cts      ; If RTS and CTS are cross\(hyconnected
  set parity even       ; (or "mark" or "space", if necessary)
  set stop\-bits 2       ; (rarely necessary)
  set flow xon/xoff     ; If you can't use RTS/CTS
  connect               ; Enter Connect (terminal) state
.fi
.sp
This assumes Computer B is set up to let you log in. If it isn't, you 
can run a copy of Kermit on Computer B and follow approximately the
same directions. More info: As above plus HELP CONNECT.
.PP
With modems or direct serial connections, you might also have to "set
parity even" (or "mark" or "space") if it's a 7\(hybit connection.
.PP
Of the connection types listed above, only one can be open at a time.
However, any one of these can be open concurrently with an FTP or HTTP
session. Each connection type can be customized to any desired degree,
scripted, logged, you name it. See the manual.
.PP
NOTE: On selected platforms, C\(hyKermit also can make X.25 connections. See
the manual for details.
.SH TRANSFERRING FILES WITH KERMIT
There is a widespread and persistent belief that Kermit is a slow protocol.
This is because, until recently, it used conservative tuning by default to
make sure file transfers succeeded, rather than failing because they
overloaded the connection. Some extra commands (or command\(hyline options,
like \-Q) were needed to make it go fast, but nobody bothered to find out
about them. Also, it takes two to tango: most non\(hyKermit\(hyProject Kermit
protocol implementations really ARE slow. The best file\(hytransfer partners
for C\(hyKermit are: another copy of C\(hyKermit (7.0 or later) and Kermit 95.
These combinations work well and they work fast by default. MS\(hyDOS Kermit
is good too, but you have to tell it to go fast (by giving it the FAST
command).
.PP
Furthermore, all three of these Kermit programs support "autodownload" and
"autoupload", meaning that when they are in Connect state and a Kermit
packet comes in from the remote, they automatically switch into file
transfer mode.
.PP
And plus, C\(hyKermit and K95 also switch automatically between text and
binary mode for each file, so there is no need to "set file type binary" or
"set file type text", or to worry about files being corrupted because they
were transferred in the wrong mode.
.PP
What all of these words add up to is that now, when you use up\(hyto\(hydate
Kermit software from the Kermit Project, file transfer is not only fast,
it's ridiculously easy. You barely have to give any commands at all.
.TP
Downloading Files
Let's say you have Kermit 95, C\(hyKermit, or MS\(hyDOS Kermit on your
desktop computer, with a connection to a Unix computer that has
C\(hyKermit installed as "kermit". To download a file (send it from Unix
to your desktop computer), just type the following command at your
Unix shell prompt:
.sp
  kermit \-s oofa.txt
.sp
(where oofa.txt is the filename). If you want to send more than one
file, you can put as many filenames as you want on the command line,
and they can be any combination of text and binary:
.sp
  kermit \-s oofa.txt oofa.zip oofa.html oofa.tar.gz
.sp
and/or you can use wildcards to send groups of files:
.sp
  kermit \-s oofa.*
.sp
If you want to send a file under an assumed name, use:
.sp
  kermit \-s friday.txt \-a today.txt
.sp
This sends the file friday.txt but tells the receiving Kermit that its
name is today.txt. In all cases, as noted, when the file transfer is
finished, your desktop Kermit returns automatically to Connect state.
No worries about escaping back, re\(hyconnecting, text/binary mode
switching. Almost too easy, right?
.TP
Uploading Files
To upload files (send them from your desktop computer to the remote
Unix computer) do the same thing, but use the \-g (GET) option instead
of \-s:
.sp
  kermit \-g oofa.txt
.sp
This causes your local Kermit to enter server mode; then the remote
Kermit program requests the named file and the local Kermit sends it
and returns automatically to Connect state when done.
.sp
If you want to upload multiple files, you have have use shell quoting
rules, since these aren't local files:
.sp
.nf
  kermit \-g "oofa.txt oofa.zip oofa.html oofa.tar.gz"
  kermit \-g "oofa.*"
.fi
.sp
If you want to upload a file but store it under a different name, use:
.sp
  kermit \-g friday.txt \-a today.txt
.TP
Kermit Transfers the Old\(hyFashioned Way
If your desktop communications software does not support autoupload or
autodownload, or it does not include Kermit server mode, the procedure
requires more steps.
.sp
To download a file, type:
.sp
  kermit \-s filename
.sp
on the host as before, but if nothing happens automatically in
response to this command, you have to switch your desktop
communications software into Kermit Receive state. This might be done
by escaping back using keyboard characters or hot keys (Alt\-x is
typical) and/or with a command (like RECEIVE) or a menu. When the file
transfer is complete, you have to go back to Connect state, Terminal
emulation, or whatever terminology applies to your desktop
communications software.
.sp
To upload a file, type:
.sp
  kermit \-r
.sp
on the host (rather than "kermit \-g"). This tells C\(hyKermit to wait
passively for a file to start arriving. Then regain the attention of
your desktop software (Alt\-x or whatever) and instruct it to send the
desired file(s) with Kermit protocol. When the transfer is finished,
return to the Connect or Terminal screen.
.TP
If File Transfer Fails
Although every aspect of Kermit's operation can be finely tuned, there
are also three short and simple "omnibus tuning" commands you can use
for troubleshooting:
.RS
.TP
FAST
Use fast file\(hytransfer settings. This has been the default since
C\(hyKermit 7.0 now that most modern computers and connections
support it. If transfers fail with fast settings, try . . .
.TP
CAUTIOUS
Use cautious but not paranoid settings. File transfers, if they
work, will go at medium speed. If not, try . . .
.TP
ROBUST
Use the most robust, resilient, conservative, safe, and reliable
settings. File transfers will almost certainly work, but they
will be quite slow (of course this is a classic tradeoff; ROBUST
was C\(hyKermit's default tuning in versions 6.0 and earlier, which
made everybody think Kermit protocol was slow). If ROBUST doesn't
do the trick, try again with SET PARITY SPACE first in case it's
not an 8\(hybit connection.
.RE
.sp
Obviously the success and performance of a file transfer also depends
on C\(hyKermit's file transfer partner. Up\(hyto\(hydate, real Kermit Project
partners are recommended because they contain the best Kermit protocol
implementations and because we can support them in case of trouble.
.sp
If you still have trouble, consult Chapter 10 of Using C\(hyKermit, or
send email to kermit\(hysupport@columbia.edu.
.TP
Advanced Kermit File\(hyTransfer Features
Obviously there is a lot more to Kermit file transfer, including all
sorts of interactive commands, preferences, options, logging,
debugging, troubleshooting, and anything else you can imagine but
that's what the manual and updates are for. Here are a few topics you
can explore if you're interested by Typing HELP for the listed
commands:
.RS
.TP
Logging transfers:
LOG TRANSACTIONS (HELP LOG)
.TP
Automatic per\(hyfile text/binary mode switching:
SET TRANSFER MODE { AUTOMATIC, MANUAL } (HELP SET TRANSFER).
.TP
Cross\(hyplatform recursive directory tree transfer:
SEND /RECURSIVE, GET /RECURSIVE (HELP SEND, HELP GET).
.TP
File collision options:
SET FILE COLLISION { OVERWRITE, BACKUP, DISCARD, ... } (HELP SET FILE).
.TP
Update: Transfer only files that changed since last time:
SET FILE COLLISION UPDATE (HELP SET FILE).
.TP
Filename selection patterns:
(HELP WILDCARD).
.TP
Flexible file selection:
SEND (or GET) /BEFORE /AFTER /LARGER /SMALLER /TYPE /EXCEPT, ...
.TP
Character\(hyset conversion:
SET { FILE, TRANSFER } CHARACTER\-SET, ASSOCIATE, ...
.TP
File/Pathname control:
SET { SEND, RECEIVE } PATHNAMES, SET FILE NAMES.
.TP
Atomic file movement:
SEND (or GET) /DELETE /RENAME /MOVE\-TO
.TP
Transferring to/from standard i/o of other commands:
SEND (or GET) /COMMAND
.TP
Recovery of interrupted transfer from point of failure:
RESEND, REGET (HELP RESEND, HELP REGET).
.RE
.TP
Non\(hyKermit File Transfer
You can also use C\(hyKermit to transfer files with FTP or HTTP Internet
protocols; see below.
.sp
On a regular serial or Telnet connection where the other computer
doesn't support Kermit protocol at all, you have several options. For
example, if your desktop communications software supports Zmodem, use
"rz" and "sz" on the host rather than Kermit. But if Kermit is your
desktop software, and you are using it to make calls or network
connections to other computers that don't support Kermit protocol (or
that don't have a good implementation of it), then if your computer
also has external X, Y, or Zmodem programs that are redirectable,
Kermit can use them as external protocols. HELP SET PROTOCOL for
details.
.sp
You can also capture "raw" data streams from the other computer with
LOG SESSION (HELP LOG and HELP SET SESSION\-LOG for details), and you
can upload files without any protocol at all with TRANSMIT (HELP
TRANSMIT, HELP SET TRANSMIT).
.SH KERMIT'S BUILT\(hyIN FTP AND HTTP CLIENTS
Kermit's FTP client is like the regular Unix FTP client that you're used
to, but with some differences:
.TP
\(bu
It has lots more commands and features.
.TP
\(bu
Each FTP command must be prefixed with "ftp", for example "ftp open",
"ftp get", "ftp bye", etc (this is not strictly true, but until you're
more familiar with it, it's best to follow this rule).
.TP
\(bu
Commands like "cd", "directory", etc, execute locally, not on the
server. Use "ftp cd", "ftp dir", etc, to have them act on the server.
.TP
\(bu
You can have an FTP session and a regular Kermit serial or Telnet
session open at the same time.
.TP
\(bu
FTP sessions can be fully automated.
.PP
Pending publication of the next edition of the manual, the Kermit FTP
client is thoroughly documented at the Kermit Project website:
.sp
  http://www.columbia.edu/kermit/ftpclient.html
.sp
You also can use HELP FTP and HELP SET FTP to get descriptions of Kermit's
FTP\(hyrelated commands.
.PP
The HTTP client is similar to the FTP one, except you prefix each command
with HTTP instead of FTP: HTTP OPEN, HTTP GET, HTTP PUT, HTTP CLOSE, etc.
Type HELP HTTP for details, or visit the to view the manual supplements.
HTTP connections can be open at the same time as regular serial or Telnet
connections and FTP connections. So Kermit can manage up to three types
connections simultaneously.
.SH INTERNET KERMIT SERVICE
C\(hyKermit can be configured and run as an Internet service (called IKSD),
similar to an FTP server (FTPD) except you can (but need not) interact with
it directly, plus it does a lot more than an FTP server can do. The TCP
port for IKSD is 1649. It uses Telnet protocol. C\(hyKermit can be an Internet
Kermit Server, or it can be a client of an IKSD. You can make connections
from C\(hyKermit to an IKSD with any of the following commands:
.sp
.nf
  telnet foo.bar.edu 1649
  telnet foo.bar.edu kermit   ; if "kermit" is listed in /etc/services
  iksd foo.bar.edu
.fi
.sp
The IKSD command is equivalent to a TELNET command specifying port 1649.
For more information about making and using connections to an IKSD, see:
.sp
  http://www.columbia.edu/kermit/cuiksd.html
.sp
You can run an Internet Kermit Service on your own computer too (if you are
the system administrator). For instructions, see:
.sp
  http://www.columbia.edu/kermit/iksd.html
.SH SECURITY
All of C\(hyKermit's built\(hyin TCP/IP networking methods (Telnet, Rlogin, IKSD,
FTP, and HTTP) can be secured by one or more of the following IETF\(hyapproved
methods:
.PP
\(bu	MIT Kerberos IV
.br
\(bu	MIT Kerberos V
.br
\(bu	SSL/TLS
.br
\(bu	Stanford SRP
.PP
For complete instructions see:
.PP
  http://www.columbia.edu/kermit/security.html
.PP
And as noted previously, you can also make SSH connections with C\(hyKermit if
you already have an SSH client installed.
.SH ALTERNATIVE COMMAND\(hyLINE PERSONALITIES
When invoked as "kermit" or any other name besides "ftp" or "telnet",
C\(hyKermit has the command\(hyline options described above in the OPTIONS
section. However, if you invoke C\(hyKermit as "telnet" or "ftp", it changes
its command\(hyline personality to match. This can be done (among other ways)
with symbolic links (symlinks). For example, if you want C\(hyKermit to be
your regular Telnet client, or the Telnet helper of your Web browser, you
can create a link like the following in a directory that lies in your PATH
ahead of the regular telnet program:
.sp
  ln \-s /usr/local/bin/kermit telnet
.sp
Now when you give a "telnet" command, you are invoking Kermit instead, but
with its Telnet command\(hyline personality so, for example:
.sp
  telnet xyzcorp.com
.sp
Makes a Telnet connection to xyzcorp.com, and Kermit exits automatically
when the connection is closed (just like the regular Telnet client). Type
"telnet \-h" to get a list of Kermit's Telnet\(hypersonality command\(hyline
options, which are intended to be as compatible as possible with the
regular Telnet client.
.PP
Similarly for FTP:
.sp
  ln \-s /usr/local/bin/kermit ftp
.sp
And now type "ftp \-h" to see its command\(hyline options, and command lines
just like you would give your regular FTP client:
.sp
  ftp xyzcorp.com
.sp
but with additional options allowing an entire session to be specified on
the command line. Finally, if Kermit's
first command\(hyline option is a Telnet, FTP, IKSD, or HTTP URL, Kermit
automatically makes the appropriate kind of connection and, if indicated by
the URL, takes the desired action:
.TP
kermit telnet:xyzcorp.com
Opens a Telnet session
.TP
kermit telnet://olga@xyzcorp.com
Ditto for user olga
.TP
kermit ftp://olga@xyzcorp.com/public/oofa.zip
Downloads a file
.TP
kermit kermit://kermit.columbia.edu/kermit/f/READ.ME
Ditto for IKSD
.TP
kermit iksd://kermit.columbia.edu/kermit/f/READ.ME
(This works too)
.TP
kermit http://www.columbia.edu/kermit/index.html
Grabs a web page
.fi
.SH LICENSE
C\(hyKermit has an unusual license, but a fair and sensible one since the
Kermit Project must support itself out of revenue: it's not a BSD license,
not GPL, not Artistic, not commercial, not shareware, not freeware. It can
be summed up like this: if you want C\(hyKermit for your own use, you can
download and use it without cost or license (but we'd appreciate it if you
would purchase the manual). But if you want to sell C\(hyKermit or bundle it
with a product or otherwise distribute it in a commercial setting EXCEPT
WITH AN OPEN\(hySOURCE OPERATING SYSTEM DISTRIBUTION such as Linux, FreeBSD,
NetBSD, or OpenBSD, you must license it. To see the complete license, give
the LICENSE command at the prompt, or see the COPYING.TXT file distributed
with C\(hyKermit 7.0 or later, or download it from
.sp
  ftp://kermit.columbia.edu/kermit/c\-kermit/COPYING.TXT
.sp
Send licensing inquiries to kermit@columbia.edu.
.SH BUGS
See the following files for listings of known bugs, limitations,
workarounds, hints and tips:
.TP
ckcbwr.txt
General C\(hyKermit bugs, hints, tips.
.TP
ckubwr.txt
Unix\(hyspecific C\(hyKermit bugs, hints, tips.
.PP
Report bugs and problems by email to:
.sp
   kermit\-support@columbia.edu.
.sp
Before requesting technical support, please read the hints here:
.sp
  http://www.columbia.edu/kermit/support.html
.sp
and also read the C\(hyKermit Frequently Asked Questions:
.sp
  http://www.columbia.edu/kermit/ckfaq.html
.SH OTHER TOPICS
There's way more to C\(hyKermit than we've touched on here \(hy\(hy troubleshooting,
customization, character sets, dialing directories, sending pages, script
writing, and on and on, all of which are covered in the manual and updates
and supplements. For the most up\(hyto\(hydate information on documentation (or
updated documentation itself) visit the Kermit Project website:
.sp
  http://www.columbia.edu/kermit/
.PP
There you will also find Kermit software packages for other platforms:
different Unix varieties, Windows, DOS, VMS, IBM mainframes, and many
others: 20+ years' worth.
.SH DOCUMENTATION AND UPDATES
The manual for C\(hyKermit is:
.TP
.I
Using C\(hyKermit
Frank da Cruz and Christine M. Gianone,
Second Edition, Digital Press / Butterworth\(hyHeinemann, Woburn, MA, 1997, 622
pages, ISBN 1\-55558\-164\-1. This is a printed book. It covers C\(hyKermit 6.0.
.TP
The C\(hyKermit 7.0 Supplement
http://www.columbia.edu/kermit/ckermit70.html
.TP
The C\(hyKermit 8.0 Supplement
http://www.columbia.edu/kermit/ckermit80.html
.TP
The C\(hyKermit 9.0 Supplement
http://www.columbia.edu/kermit/ckermit90.html
.PP
Visit C\(hyKermit home page:
.sp
  http://www.columbia.edu/kermit/ckermit.html
.sp
to learn about new versions, Beta tests, and other news; to
read case studies and tutorials; to download source code, install packages,
and prebuilt binaries for many platforms. Also visit:
.TP
http://www.columbia.edu/kermit/scriptlib.html
The Kermit script library and tutorial
.TP
http://www.columbia.edu/kermit/newfaq.html
The Kermit FAQ (Frequently Asked Questions about Kermit)
.TP
http://www.columbia.edu/kermit/ckfaq.html
The C\(hyKermit FAQ (Frequently Asked Questions about C\(hyKermit)
.TP
http://www.columbia.edu/kermit/telnet.html
C\(hyKermit Telnet client documentation
.TP
http://www.columbia.edu/kermit/security.html
C\(hyKermit security documentation (Kerberos, SSL/TLS, etc)
.TP
http://www.columbia.edu/kermit/cuiksd.html
Internet Kermit Service user documentation
.TP
http://www.columbia.edu/kermit/iksd.html
Internet Kermit Service administrator documentation
.TP
http://www.columbia.edu/kermit/studies.html
Case studies.
.TP
http://www.columbia.edu/kermit/support.html
Technical support.
.TP
http://www.columbia.edu/kermit/k95tutorial.html
Kermit 95 tutorial.
.TP
comp.protocols.kermit.misc
The Kermit newsgroup (unmoderated).
.SH FILES
.TP
COPYING.TXT
C\(hyKermit license.
.TP
~/.kermrc
Initialization file.
.TP
~/.mykermrc
Customization file.
.TP
~/.kdd
Kermit dialing directory (see manual).
.TP
~/.knd
Kermit network directory (see manual).
.TP
~/.ksd
Kermit services directory (see manual).
.TP
ca_certs.pem
Certificate Authority certifcates used for SSL connections.
.TP
ckuins.txt
Installation instructions for Unix.  Also at
http://www.columbia.edu/kermit/ckuins.html.
.TP
ckcbwr.txt
General C\(hyKermit bugs, hints, tips.
.TP
ckubwr.txt
Unix\(hyspecific C\(hyKermit bugs, hints, tips.
.TP
ckcplm.txt
C\(hyKermit program logic manual.
.TP
ckccfg.txt
C\(hyKermit compile\(hytime configuration options.
.TP
ssh
(in your PATH) SSH connection helper.
.TP
rz, sz, etc.
(in your PATH) external protocols for XYZmodem.
.TP
/var/spool/locks (or whatever)
UUCP lockfile for dialing out (see installation instructions).
.SH AUTHORS
.TP
Software
Frank da Cruz and Jeffrey E Altman,
.br
1985\(hypresent, with contributions from hundreds of others all over the
world.
.TP
Documentation
Frank da Cruz
.TP
Address
.nf
The Kermit Project \(hy Columbia Univerity
612 West 115th Street
New York NY 10025\-7799
USA
.fi
.TP
E\(hyMail
kermit@columbia.edu
.TP
Web
http://www.columbia.edu/kermit/
.fi
.br
