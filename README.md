#File-Share-Protocol

####The Application
- This is an Application Level File Sharing Protocol with support for download and upload for files and indexed searching. It is made using a client-server model, and uses either transport layer protocols tcp or udp (as per the user's input) to establish communication. It has MD5checksum incorporated into it to handle file transfer errors.

####To build
- `g++ server.cpp -o server -lcrypto`: to build server
- `g++ client.cpp -o client -lcrypto`: to build client

####To Use
- The application has been designed with one server and one client.
  - `./client` to run the client first. Then,
    - First enter the `hostname` or `IP` of the machine you want to connect to
    - Specify the `port` on which you want to connect to on the server
    - Next specify the `protocol` to connect to the server - either `tcp` or `udp`

  - `./server` to run the server first. Then,
    - Enter the `port` number you want your server to listen to
    - Enter the `protocol` - either `tcp` or `udp` for your server to use.


The following commands are supported by the client side application:  

- `FileHash flag (args)`: This command indicates that the client wants to check if any of the files on the other end have been changed. The flag variable can take the values:  
    - `verify`: this checks for the specific file name provided as command line argument and return its ‘checksum’ and ‘lastmodified’ timestamp. Usage - `FileHash verify <filename>`
    - `checkall`: Outputs filename, checksum and lastmodified timestamp of all the files in the shared directory
- `IndexGet flag (args)`: This command requests the display of the shared files on the connected system. The flags can be:
    - `shortlist`: Shows names and details of files between a specific set of timestamps. Usage: `IndexGet shortlist <date1> <date2>`
    - `longlist`: Shows names and details of all files in the shared directory
    - `regex`: Matches the queried regex with the filenames in the shared directory and displays the details of the results. Usage: `IndexGet regex <regex>`
- `FileDownload <protocol> <filename>`: This command requests to download a file from the server using specified protocol. Protocol can be either `tcp` or `udp`. If the client server connection is made using udp, and tcp protocol is passed as parameter, you'll be asked for a new `port` to communicate and transfer the file.
- `FileUpload <filename>`: This command is used to upload a file to the server's shared directory.
