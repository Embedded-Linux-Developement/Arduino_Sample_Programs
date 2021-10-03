
/*****************************************************************************************************
Header file  Html_Templates.h
Description:- Simple header file to store Html templates 

*******************************************************************************************************/

#ifndef Html_Templates_H
#define Html_Templates_H

/* Define Header part*/
const char Debug_Header_Temp[] = "<!DOCTYPE html> \n \
<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\">  \n \
   <title>Trace Debug</title>  \n \
   <!-- Add 1 Sec Auto refresh -->  \n \
   <meta http-equiv=\"refresh\" content=\"1\">   \n \
 </head>  \n \
 <body>  \n \
   <h1 style=\"background-color:Green;text-align: center;\">Following are the Debug trace Live Output</h1>   \n \
   <div style=\"text-align: center;\"> ";



const char Debug_Footer_Temp[] = "</div> \n </body> \n\
</html>";







#endif /* End of  Html_Templates_H */
