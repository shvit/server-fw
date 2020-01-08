# TFTP firmware server

## Intro

This TFTP firmware server wonderful used in embedded firmware development.
When you have many firmware shapshots for complex testing (regression, functional, etc).
You don't operate file names of firmware, no, only md5 sums.

## Futures

* get file by md5 sum in *.md5 files
* one root server directory for server upload (and download)
* can set many serach directory for server download

## Plans

* using Firebird SQL server as main firmware storage

## Requirements

* Clang 8
* C++17
* Ubuntu 18.04 LTE
* OpenSSL library (sudo apt-get install libssl-dev)

Other not tested, sorry

## How build and use
1. Get sources 
<pre>
mkdir server_fw
git clone git@github.com:shvit/server\_fw.git -b master server_fw
</pre>
2. Make binary file
<pre>
cd server_fw
make release
</pre>
3. Disable any listening 69 port other executables
<pre>
sudo netstat -lup|grep "69\|tftp"
</pre>
4. Install firmware server
<p>
<pre>
sudo ./install.sh allauto
</pre>
By default, make root server directory _/mnt/tftp_ and one search directory _/mnt/backup_
</p>
5. Copy firmware files to directory _/mnt/tftp_ or _/mnt/backup_ and make *.md5 files
<p>
File in search directory (_/mnt/backup_) can place in separate nested directory or with unique file names<br>
For each _file_ in search directory make *.md5 file:
<pre>
md5sum file > file.md5
</pre>
</p>

Profit!

## License

GNU general public license version 3
