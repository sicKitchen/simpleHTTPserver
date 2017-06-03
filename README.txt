
A http Client created in C language.
Created by Spencer Kitchen
For Assignment 2 Part 2 for Networks

Archive contents
----------------
/simpleHTTPserver 
   README.txt		 ReadMe for project
   404page.html          Mock up page for 404: page not found reply
   assign2PT2.c          Simple server.
   index.html            Index page of website hosted on server
   /images         
      base_ball.png      PNG image you can request as a test 
      page2.html         Second html page of website

Compile
-------
'gcc -Wall -std=c99 -o webServer assign2PT2.c '

Run
---
'./webServer'

How to use
----------
From web browser, enter 'localhost:8080/index.html' once
server is running. You can also look at
   'http://localhost:8080/images/page2.html'
   'http://localhost:8080/images/base_ball.png'
If you request a page thats not on server you will get 404 reply.
To change port number that Server runs on, change portno in code to
new port number.

FLAGS
-----
In assign2PT2.c
-Toggle DEBUG output with 'DEBUG_PARSE' & 'DEBUG_MAIN'
-Can turn off server output by setting 'OUTPUT' to false.
-Currentlty no way to shutdown server, <CTRL-c>
