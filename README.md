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

* Ubuntu 18.04

Other not tested, sorry

## How use

1. Make server directory (for example /mnt/tftp).
2. Place firmware files in server directory (p.1).
3. Generate md5 files if not exists (using md5sum). One firmware file - one md5 file.
4. Disable any listening 69 port other executables (see sudo netstat -lup).
5. Clone this repository.
6. Build application.
7. Install.

Profit!

## License

GNU general public license version 3
