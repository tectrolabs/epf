# Entropy Pool Feeder (epf) for Linux

The 'epf' is a utility used for feeding the Linux entropy pool with true random bytes which are securely downloaded from a remote service such as https://entropysector.com 

## Description

In Linux, the entropy pool is used as a source of high quality randomness needed for a variety of security operations. 
The entropy pool is accessed using /dev/random device. To avoid entropy pool starvation, it is important to maintain the pool with enough amount of randomness. The 'epf' utility will continuously monitor the entropy pool and will feed it with a high true randomness when the entropy level is below the threshold value.

The true random byte stream is downloaded from a remote service using SSL and it is additionally encrypted to ensure high security. 

## Getting Started

To build and run 'epf' utility you will need a sudo access permissions to your Linux instance and an Internet connection.

### Dependencies

The following dependencies are required to successfully build the 'epf' utility: 

* C++ compiler
* openssl development

C++ compiler can be installed in CentOS with the following command:
```
sudo yum install gcc-c++
```
Openssl development dependency can be installed on CentOS with the following command:
```
sudo yum install openssl-devel
```

```
### Installing

* Download the project:
```
git clone https://github.com/abelinski/epf
```
* Build the 'epf' utility:
```
cd epf
make 
```
* Install the necessary executables:
```
sudo make install
```
* Copy configuration and public key files to /etc/epf directory:
```
sudo mkdir /etc/epf
sudo cp epf.properties /etc/epf/
sudo cp epf-pubkey.pem /etc/epf/
```
* Create the log folder
```
sudo mkdir /var/log/epf
```
* Add a new crontab entry:
```
sudo crontab -e
```
Add the following line at the end:
```
@reboot /usr/local/bin/run-epf.sh >> /var/log/epf/run-epf.log 2>&1
```
Save the changes
* Reboot 

### Verify 'epf' is working

* Check for any errors in /var/log/epf/run-epf.log 
* Verify the /etc/random pool is getting populated by running the following command (it should not block):
```
dd if=/dev/random of=/dev/null bs=4800 count=10 iflag=fullblock
```

## Authors

Andrian Belinski  

## License

see the LICENSE file for details
