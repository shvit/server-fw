# TFTP firmware server

## Intro

This TFTP firmware server wonderful used in embedded firmware development.<br>
When you have many firmware snapshots for complex testing (regression, functional, etc).<br>
You don't operate file names or directories of firmware, no, now only its md5 sums.

## Futures

* get file by md5 sum in *.md5 files
* one root server directory for server upload (and download)
* can set many serach directory for server download

## Plans for future

* using Firebird SQL server as main firmware storage

## Requirements

* Ubuntu 18.04 LTE (newer not tested)
<p>Other GNU Linux distros not tested, sorry</p>

* Clang or GCC as last stable version (using C++17)
<p>For install clang:</p>
<pre>
sudo apt-get update
sudo apt-get install llvm clang clang-tools
</pre>

* OpenSSL library
<pre>
sudo apt-get install libssl-dev
</pre>

## How build and use

1. Get sources 
<pre>
mkdir server-fw
git clone git@github.com:shvit/server-fw.git -b server-fw
</pre>

2. Make binary file (you can skip this and go to 3)
<pre>
cd server-fw
make release
</pre>
Type "bin/server-fw -h" for server settings in help information<br>
<br>

3. Disable any listening 69 port other executables
<pre>
sudo netstat -lup|grep "69\|tftp"
</pre>

4. Install firmware tftp server
<p>
<pre>
make install
</pre>
By default, make root server directory  /mnt/tftp  and one search directory /mnt/backup
<pre>
mkdir -p /mnt/tftp
mkdir -p /mnt/backup
</pre>
</p>

5. Copy firmware files to directory  <i>/mnt/tftp</i>  or  <i>/mnt/backup</i>  and make *.md5 files
<p>
File in search directory  <i>/mnt/backup</i>  can place in separate nested directory or with unique file names<br>
For each  <i>file</i>  in search directory make *.md5 file:
<pre>
md5sum file > file.md5
</pre>
</p>
<p>

Profit!

## Uninstall

For uninstall <b>server-fw</b>
<pre>
make uninstall
</pre>
</p>

## License

GNU general public license version 3.<br>
See LICENSE file in root directory.

## Author
Vitaliy Shirinkin, Russia, 2019-2021<br>
e-mail: vitaliy.shirinkin@gmail.com

## History

See HISTORY file in root directory.
