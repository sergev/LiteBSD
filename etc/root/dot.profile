PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin
BLOCKSIZE=1k
MORE="-ce"
export PATH BLOCKSIZE MORE

# Fancy colored prompt with current directory name in it.
PS1='[0;31m${PWD#?*/}[1;32m #[m '

# Export TERM for single user shells.
export TERM

echo 'erase ^H, kill ^U, intr ^C status ^T'
stty erase '^H' kill '^U' intr '^C' status '^T' crt

umask 022

#echo "Don't login as root, use the su command."
