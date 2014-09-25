-----------
FIDO README
-----------

Fido is a multi-threaded file watcher that can be configured
look for pattern matches in log files and execute a program 
or script in the event of a match. Fido scans the as its updated
so that it discovers matches in real-ish time.

------------
INSTALLATION
------------

Fido is built with GNU/autotools so generally you can configure, 
make and make install. We recommend using the default configure
in go.sh. So the process is this:

./go.sh
make
make install

-------
STARTUP 
-------

This distribution contains startup scripts for RedHat Linux. To
install them, run the following command inside the top level source
directory:

  utils/install-startup

---------
CONFIGURE
---------

If you built fido with the default settings, you can configure it
inside /etc/fido/fido.conf Your rules can be stored in /etc/fido/rules
For more details, man fido and man fido.conf



