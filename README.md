
BusyBox for iOS, jailed or jailbroken.
--------------------------------------
To build: 

```
> brew install gnu-sed
> make ARCH=armv7
or
> make ARCH=arm64
```

You'll get `shell/ispy-shell.dylib` that you can inject into an app using [theos-jailed](https://github.com/BishopFox/theos-jailed), [CydiaSubstrate](http://www.cydiasubstrate.com/), etc. The dylib's constructor sets up a Hush shell bound to port 8765/TCP on the iDevice. It has a controlling TTY. To connect to it, use something like socat:

```
> socat tcp-connect:<your_idevice_ip>:8765 file:`tty`,raw,echo=0
BusyBox v1.20.0.git (2016-07-13 14:19:16 MDT) hush - the humble shell
Enter 'help' for a list of built-in commands.

/ $ uname -a
Darwin iPhwn 15.5.0 Darwin Kernel Version 15.5.0: Mon Apr 18 16:44:06 PDT 2016; root:xnu-3248.50.21~4/RELEASE_ARM64_T7000 iPhone7,2 GNU/Linux
/ $
/ $ id
uid=501(mobile) gid=501(mobile) groups=501(mobile),299(systemusers)
/ $
/ $ cat /etc/passwd
##
# User Database
#
# This file is the authoritative user database.
##
nobody:*:-2:-2:Unprivileged User:/var/empty:/usr/bin/false
root:xxxxxxxxxxxxx:0:0:System Administrator:/var/root:/bin/sh
mobile:xxxxxxxxxxxxx:501:501:Mobile User:/var/mobile:/bin/sh
daemon:*:1:1:System Services:/var/root:/usr/bin/false
_ftp:*:98:-2:FTP Daemon:/var/empty:/usr/bin/false
_networkd:*:24:24:Network Services:/var/networkd:/usr/bin/false
_wireless:*:25:25:Wireless Services:/var/wireless:/usr/bin/false
_installd:*:33:33:Install Daemon:/var/installd:/usr/bin/false
_neagent:*:34:34:NEAgent:/var/empty:/usr/bin/false
_ifccd:*:35:35:ifccd:/var/empty:/usr/bin/false
_securityd:*:64:64:securityd:/var/empty:/usr/bin/false
_mdnsresponder:*:65:65:mDNSResponder:/var/empty:/usr/bin/false
_sshd:*:75:75:sshd Privilege separation:/var/empty:/usr/bin/false
_unknown:*:99:99:Unknown User:/var/empty:/usr/bin/false
_distnote:*:241:241:Distributed Notifications:/var/empty:/usr/bin/false
_astris:*:245:245:Astris Services:/var/db/astris:/usr/bin/false
_ondemand:*:249:249:On Demand Resource Daemon:/var/db/ondemand:/usr/bin/false
/ $
/ $ cd $HOME
/private/var/mobile/Containers/Data/Application/DAFC69C4-A9B1-45FD-A954-6CCF067FB620 $
```

The shell is fairly functional, but on jailed devices you're subject to limitations of the iOS kernel sandbox. This means you can't execve(2) or fork(2), which means you can't run any commands, you don't get pipes, redirection, etc. You do get all the common unix commands in a Bash-like shell.

At the moment any attempt to run a non-builtin command will cause a crash. A fix is TBD! Most new code is located in `shell/ispy-shell.c`.

For more information on injecting this dylib onto jailed devices, see this blog post: https://www.bishopfox.com/blog/2015/02/rethinking-repackaging-ios-apps-part-1/

The rest of this README is the one that comes with BusyBox.

README
----
Please see the LICENSE file for details on copying and usage.
Please refer to the INSTALL file for instructions on how to build.

What is busybox:

  BusyBox combines tiny versions of many common UNIX utilities into a single
  small executable.  It provides minimalist replacements for most of the
  utilities you usually find in bzip2, coreutils, dhcp, diffutils, e2fsprogs,
  file, findutils, gawk, grep, inetutils, less, modutils, net-tools, procps,
  sed, shadow, sysklogd, sysvinit, tar, util-linux, and vim.  The utilities
  in BusyBox often have fewer options than their full-featured cousins;
  however, the options that are included provide the expected functionality
  and behave very much like their larger counterparts.

  BusyBox has been written with size-optimization and limited resources in
  mind, both to produce small binaries and to reduce run-time memory usage.
  Busybox is also extremely modular so you can easily include or exclude
  commands (or features) at compile time.  This makes it easy to customize
  embedded systems; to create a working system, just add /dev, /etc, and a
  Linux kernel.  Busybox (usually together with uClibc) has also been used as
  a component of "thin client" desktop systems, live-CD distributions, rescue
  disks, installers, and so on.

  BusyBox provides a fairly complete POSIX environment for any small system,
  both embedded environments and more full featured systems concerned about
  space.  Busybox is slowly working towards implementing the full Single Unix
  Specification V3 (http://www.opengroup.org/onlinepubs/009695399/), but isn't
  there yet (and for size reasons will probably support at most UTF-8 for
  internationalization).  We are also interested in passing the Linux Test
  Project (http://ltp.sourceforge.net).

----------------

Using busybox:

  BusyBox is extremely configurable.  This allows you to include only the
  components and options you need, thereby reducing binary size.  Run 'make
  config' or 'make menuconfig' to select the functionality that you wish to
  enable.  (See 'make help' for more commands.)

  The behavior of busybox is determined by the name it's called under: as
  "cp" it behaves like cp, as "sed" it behaves like sed, and so on.  Called
  as "busybox" it takes the second argument as the name of the applet to
  run (I.E. "./busybox ls -l /proc").

  The "standalone shell" mode is an easy way to try out busybox; this is a
  command shell that calls the built-in applets without needing them to be
  installed in the path.  (Note that this requires /proc to be mounted, if
  testing from a boot floppy or in a chroot environment.)

  The build automatically generates a file "busybox.links", which is used by
  'make install' to create symlinks to the BusyBox binary for all compiled in
  commands.  This uses the CONFIG_PREFIX environment variable to specify
  where to install, and installs hardlinks or symlinks depending
  on the configuration preferences.  (You can also manually run
  the install script at "applets/install.sh").

----------------

Downloading the current source code:

  Source for the latest released version, as well as daily snapshots, can always
  be downloaded from

    http://busybox.net/downloads/

  You can browse the up to the minute source code and change history online.

    http://git.busybox.net/busybox/

  Anonymous GIT access is available.  For instructions, check out:

    http://www.busybox.net/source.html

  For those that are actively contributing and would like to check files in,
  see:

    http://busybox.net/developer.html

  The developers also have a bug and patch tracking system
  (https://bugs.busybox.net) although posting a bug/patch to the mailing list
  is generally a faster way of getting it fixed, and the complete archive of
  what happened is the git changelog.

  Note: if you want to compile busybox in a busybox environment you must
  select CONFIG_DESKTOP.

----------------

Getting help:

  when you find you need help, you can check out the busybox mailing list
  archives at http://busybox.net/lists/busybox/ or even join
  the mailing list if you are interested.

----------------

Bugs:

  if you find bugs, please submit a detailed bug report to the busybox mailing
  list at busybox@busybox.net.  a well-written bug report should include a
  transcript of a shell session that demonstrates the bad behavior and enables
  anyone else to duplicate the bug on their own machine. the following is such
  an example:

    to: busybox@busybox.net
    from: diligent@testing.linux.org
    subject: /bin/date doesn't work

    package: busybox
    version: 1.00

    when i execute busybox 'date' it produces unexpected results.
    with gnu date i get the following output:

	$ date
	fri oct  8 14:19:41 mdt 2004

    but when i use busybox date i get this instead:

	$ date
	illegal instruction

    i am using debian unstable, kernel version 2.4.25-vrs2 on a netwinder,
    and the latest uclibc from cvs.

	-diligent

  note the careful description and use of examples showing not only what
  busybox does, but also a counter example showing what an equivalent app
  does (or pointing to the text of a relevant standard).  Bug reports lacking
  such detail may never be fixed...  Thanks for understanding.

----------------

Portability:

  Busybox is developed and tested on Linux 2.4 and 2.6 kernels, compiled
  with gcc (the unit-at-a-time optimizations in version 3.4 and later are
  worth upgrading to get, but older versions should work), and linked against
  uClibc (0.9.27 or greater) or glibc (2.2 or greater).  In such an
  environment, the full set of busybox features should work, and if
  anything doesn't we want to know about it so we can fix it.

  There are many other environments out there, in which busybox may build
  and run just fine.  We just don't test them.  Since busybox consists of a
  large number of more or less independent applets, portability is a question
  of which features work where.  Some busybox applets (such as cat and rm) are
  highly portable and likely to work just about anywhere, while others (such as
  insmod and losetup) require recent Linux kernels with recent C libraries.

  Earlier versions of Linux and glibc may or may not work, for any given
  configuration.  Linux 2.2 or earlier should mostly work (there's still
  some support code in things like mount.c) but this is no longer regularly
  tested, and inherently won't support certain features (such as long files
  and --bind mounts).  The same is true for glibc 2.0 and 2.1: expect a higher
  testing and debugging burden using such old infrastructure.  (The busybox
  developers are not very interested in supporting these older versions, but
  will probably accept small self-contained patches to fix simple problems.)

  Some environments are not recommended.  Early versions of uClibc were buggy
  and missing many features: upgrade.  Linking against libc5 or dietlibc is
  not supported and not interesting to the busybox developers.  (The first is
  obsolete and has no known size or feature advantages over uClibc, the second
  has known bugs that its developers have actively refused to fix.)  Ancient
  Linux kernels (2.0.x and earlier) are similarly uninteresting.

  In theory it's possible to use Busybox under other operating systems (such as
  MacOS X, Solaris, Cygwin, or the BSD Fork Du Jour).  This generally involves
  a different kernel and a different C library at the same time.  While it
  should be possible to port the majority of the code to work in one of
  these environments, don't be surprised if it doesn't work out of the box.  If
  you're into that sort of thing, start small (selecting just a few applets)
  and work your way up.

  In 2005 Shaun Jackman has ported busybox to a combination of newlib
  and libgloss, and some of his patches have been integrated.

Supported hardware:

  BusyBox in general will build on any architecture supported by gcc.  We
  support both 32 and 64 bit platforms, and both big and little endian
  systems.

  Under 2.4 Linux kernels, kernel module loading was implemented in a
  platform-specific manner.  Busybox's insmod utility has been reported to
  work under ARM, CRIS, H8/300, x86, ia64, x86_64, m68k, MIPS, PowerPC, S390,
  SH3/4/5, Sparc, v850e, and x86_64.  Anything else probably won't work.

  The module loading mechanism for the 2.6 kernel is much more generic, and
  we believe 2.6.x kernel module loading support should work on all
  architectures supported by the kernel.

----------------

Please feed suggestions, bug reports, insults, and bribes back to the busybox
mailing list:

	busybox@busybox.net

and/or maintainer:

	Denys Vlasenko
	<vda.linux@googlemail.com>
