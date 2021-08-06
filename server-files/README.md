Dell BiosConnect TLS vulnerability PoC

Infrastructure setup:
In a network router you control redirect 8.8.8.8 to a DNS server you control, then add a DNS record for *.dell.com to redirect to an IP you control.  An easy way to do this is using a system running pfsense as the gateway and DNS for the network segment the target device is connected to..

Once your network is setup, set up packet capture if you'd like, you can use the files in this repository to run a malicious https server. (you can use the included certificate to decrypt any TLS traffic)

Run ./Dell/https_server.py and that will start an https server with a valid wildcard ssl Certum certificate for *.realdell.com

In any Dell system that includes BiosConnect, press F12 while booting to show the boot options. From there choose the firmware update over the network and you'll see a connection is established to this VM and a file is requested.

A proper output from the python script during a PoC test should look like this:
192.168.9.197 is the victim laptop IP

user@ubuntu:~/BIOSDisconnect/Dell$ sudo python https_server.py 
192.168.9.197 - - [18/Jun/2021 21:00:59] "GET / HTTP/1.1" 200 -
192.168.9.197 - - [18/Jun/2021 21:01:01] "GET /catalog/CatalogBc.xml HTTP/1.1" 200 -

Within the CatalogBc.xml file you can find the proper reference to the FOTA or CSOS efi files and place your own copy in the path of your choosing in the ./Dell folder (see 'catalog' folder as an example)

For a simple show of remote code execution, place ./extra_files/popCalcWin7.efi as a replacement for the "DellFOTALauncher.efi" in the CatalogBc.xml file, turn off UEFI Secure Boot in the target device and run the PoC flow again.
