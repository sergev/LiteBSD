PATH=/sbin:/usr/sbin:/bin:/usr/bin
export PATH

BLOCKSIZE=1k
export BLOCKSIZE

# Export TERM for single user shells.
export TERM

# Enable command line editing mode.
set -E

echo 'erase ^H, kill ^U, intr ^C status ^T'
stty erase '^H' kill '^U' intr '^C' status '^T' crt

umask 022

echo "Don't login as root, use the su command."
