#PTran#
##Descryption##
A Secure TCP Repeater. Listen on Local port, Transmit data which can be Encrypted to A destination host port.   
Running PTran on dest_host with the same key can Decrypt the data. because data is encrypted with stream cipher RC4.  
If you don't need data Encryption,just leave the Key field blank.  

##Note##
use Multithread to establish multiple connection between two ports.
connection NUM is not limited.
