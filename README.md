# minerva-connect-server
HTTP Server written in C for the Minerva Connect project

## Documentation

### Core
This is the server's main functionality.  
The code will bind a socket to the provided ipv4 and port.  
When running, client sockets will be accepted and handled in a new thread.
The thread will parse the HTTP header and look at the URI.  
Rightnow there is no nice layering for routes yet, so the routes are all implemented in the router function.  
Currently PUT, POST, DELETE, OPTIONS, HEAD request methods are not handled yet and will result in error 400 (Bad request).  

### Variables and the .mvcs file
A .mvcs file is a HTML extention with slight modular server-replacement capabilities.  
Server variables and dynamic content functions are programmed in variables.c.  
In a .mvcs file a variable is formatted as following $$SOME_SERVER_VARIABLE$$.  
Thus, a double dollar-sign followed by the name of the variable and ending with another double dollar sign.  
Currently there is no escaping yet so each registered variable will be replaced with actual server variable values.  
However, a trick to avoid this is by adding html tags between the double dollar signs so that the string will not be matched by the search-and-replace logic.  
Variables are only replaced (as implemented in the router in the core.c) when serving a file with a .mvcs extention, regular .html files will never replace dynamic-content variables.  

## PGP authentication
This server is written to be part of the minerva connect project.  
Minerva connect is a project to escape secure networks where serving http is restricted by firewalls.  
In these modern times it is not desired to store password credentials anymore and therefore I introduced password-less authentication.  
Sending a PGP-private key to the server is not a secure way for authenticating because you can never trust that the server operator is not logging it.  
Therefore I decided to implement an authentication protocol based on PGP signatures.  
Your pgp key-pair will exist of a public key that is equal to your identity and a pgp privatekey that is a secret password used for authorization.  
Your private key is stored preferably on a USB that you protect with your life and always carry around.  
When signing up for an account of the server you will give the administrator / server operator your public key.  
The public key will be added to a database that is only locally accessible on the server and never exposed to HTTP.  
When logging in the server will ask you to sign a random code.  
You will sign this code with your private key and send it back to the server.  
In this signature there will be your public key identifier.   
This identifier will be used to look up your public key in the servers database.  
If your public key exists the server will proceed with verifying that the signed message it received was cryptographically signed by you by matching it with your public key.  
If this check turns out to be valid, then the server will create a session for you and you will be logged in.  
In case that the public key does not exist or the signature turns out to not match with the public key (could be the case when public key id collides, a vulnerability in pgp) then the server will deny you access and close the connection.  
The same steps of identification, authentication and authorization apply to making changes to data that belongs to your identity on the server (like settings).  
You again would be asked to sign a code to authorize the changes made to your data.  
This way the server prevents people from making changes to your account when you leave your laptop unlocked in public spaces by accident.  
I hope that more companies will pick up this method of authentication and that it becomes a standard one day as this prevents unauthorized changes, but also passwords from being leaked by databreaches and cracked by criminals.  

## NOTE
This server besides the beautiful concepts of security is practically perhaps not as secure as I think.  
It has been written in c and programs written in c force the programmer to write complex code for managing memory which may lead to vulnerabilities.  
The reason this server is written in c is to make it support many platforms, have great perfomance and be minimal in size.  
If you want to use this server in production environments where it is important to protect the userdata you should let a security team audit the code of the server.  

## Special thanks
- OpenSSL, for providing an amazing library for implementing cryptography
- libgcrypt, for providing an easy interface to PGP cryptography
- Alexander Shulgin

