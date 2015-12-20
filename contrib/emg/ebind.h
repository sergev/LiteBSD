/* This file is in the public domain. */

/*
 * This table is *roughly* in ASCII order, left to right across the
 * characters of the command. This expains the funny location of the
 * control-X commands.
 */

KEYTAB keytab[] = {
  {CTRL | 'A', gotobol},
  {CTRL | 'B', backchar},
  {CTRL | 'D', forwdel},
  {CTRL | 'E', gotoeol},
  {CTRL | 'F', forwchar},
  {CTRL | 'G', ctrlg},
  {CTRL | 'H', backdel},
  {CTRL | 'I', tab},
  {CTRL | 'K', killtext},
  {CTRL | 'L', refresh},
  {CTRL | 'M', newline},
  {CTRL | 'N', forwline},
  {CTRL | 'O', openline},
  {CTRL | 'P', backline},
  {CTRL | 'Q', quote},
  {CTRL | 'R', backsearch},
  {CTRL | 'S', forwsearch},
  {CTRL | 'T', twiddle},
  {CTRL | 'V', pagedown},
  {CTRL | 'W', killregion},
  {CTRL | 'Y', yank},
  {CTLX | '(', ctlxlp},
  {CTLX | ')', ctlxrp},
  {CTLX | '1', onlywind},
  {CTLX | '2', splitwind},
  {CTLX | 'B', usebuffer},
  {CTLX | 'E', ctlxe},
  {CTLX | 'F', setfillcol},
  {CTLX | 'K', killbuffer},
  {CTLX | 'N', filename},
  {CTLX | 'O', nextwind},
  {CTLX | 'S', filesave},		/* non-standard */
  {CTLX | 'Q', quote},			/* non-standard */
  {CTLX | 'X', nextbuffer},
  {CTLX | '^', enlargewind},
  {CTLX | CTRL | 'B', listbuffers},
  {CTLX | CTRL | 'C', quit},
  {CTLX | CTRL | 'F', filefind},
  {CTLX | CTRL | 'I', insfile},
  {CTLX | CTRL | 'R', fileread},
  {CTLX | CTRL | 'S', filesave},
  {CTLX | CTRL | 'W', filewrite},
  {META | '.', setmark},
  {META | '<', gotobob},
  {META | '>', gotoeob},
  {META | 'B', backword},
  {META | 'E', showversion},		/* non-standard */
  {META | 'F', forwword},
  {META | 'G', gotoline},		/* non-standard */
  {META | 'S', forwsearch},		/* non-standard */
  {META | 'V', pageup},
  {META | 'W', copyregion},
  {META | CTRL | 'N', namebuffer},
  {METE | 'A', backline},
  {METE | 'B', forwline},
  {METE | 'C', forwchar},
  {METE | 'D', backchar},
  {METE | '5', pageup},
  {METE | '6', pagedown},
  {0x7F, backdel},
  {0, 0}
};
