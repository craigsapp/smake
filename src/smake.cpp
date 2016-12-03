//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Filename:      smake.cpp
// Creation Date: Fri Mar 29 21:43:39 PST 1996
// Last Modified: Fri Feb 22 15:37:03 PST 2013 updated for C99 ANSI standard.
// Syntax:        C++
// $Smake:        g++ -O3 -o %b %f -L/user/c/craig/lang/c/lib -loptions %
//                && strip %b
//
// Description:   "Simple MAKE".  Runs embedded actions in a file, such as
//                to compile a program.
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sstream>
#include <iostream>
#include <fstream>

#include "Options.h"

char         MagicString[]  = "$Smake:";    
char*        Basename       = NULL;
const char*  filename       = NULL;
const char** passParams     = NULL;
int          numPassParams;

// global variables for options:
Options options;
int echo    = 0;      // -e: echo the command found, but don't execute it.
int quiet   = 0;      // -x: execute command w/o printing command to terminal.
int numSkip = 4;      // -s: number of characters to skip at beginning of
                      //     newline when continuing command line.

const char* commandMarker = NULL;  // -c: marker for command string start.
const char* variantSmake  = NULL;  // -v: smake variant option

// function declarations:
int    backwardPosition   (char, char*);
void   checkOptions       (Options& opts, int argc, char* argv[]);
void   usage              (void);
void   find               (const char*, istream&);
void   getOptions         (void);
char   nextChar           (istream&);
void   readCommand        (stringstream&, const char*, istream&);
char*  unsuffix           (const char* string);
void   manpage            (void);


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {
   checkOptions(options, argc, argv);

   // Find name of file to extract command from:
   filename = options.getArg(1).c_str();
   Basename = unsuffix(filename);
   ifstream inFile(filename);
   if (!inFile) {
      cerr << "Cannot open file: " << filename << endl;
      exit(1);
   }

   // Determine if any parameters are to be passed:
   numPassParams = options.getArgCount() - 1;
   if (numPassParams > 0) {
      passParams = new const char*[numPassParams];
      for (int i=0; i<numPassParams; i++) {
         passParams[i] = options.getArg(i+2).c_str();
      }
   }

   // Create the command string and run it:
   stringstream strcommand;
   readCommand(strcommand, commandMarker, inFile);
   if (strcommand.str().length() != 0) {
      if (!quiet) {
         cout << strcommand.str() << endl;
      }
      if (!echo) {
         int status = system(strcommand.str().c_str());
         if (status == -1) {
            cerr << "Error executing smake command" << endl;
         }
      }
   } else {
      cerr << "No embedded command found in " << filename << endl;
   }
   
   // free variables on the heap:
   if (Basename     != (void*)NULL) delete [] Basename;
   if (variantSmake != (void*)NULL) delete [] variantSmake;
   if (passParams   != (void*)NULL) delete [] passParams;

   return 0;
}

////////////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// usage -- Show basic help.
//

void usage(const char* command) {
   cerr << "Usage: " << command 
        << " [-s skip][-v variant |-c][-e|-x] input-file [parameters]\n"
        << "   Processes a file according the command string in the file.  "
        <<     "The command\n"
        << "   string starts with the pattern $Smake: and lasts until the "
        <<     "end of the line.\n"
        << "   Symbolic patterns in the command are:\n"
        << "      %f -- replaces with the filename string\n"
        << "      %b -- replaces with the filename base (removes all "
        <<      "characters after\n            last \".\""
        << " of file name and also removes the dot.\n"
        << "Type " << command << " -m for the man page."
        << endl;
}



//////////////////////////////
//
// backwardPosition -- return the position of the last sought
//	character in the string.
//

int backwardPosition(char sought, const char *string) {
   int i;
   for (i = strlen(string) - 1; i>=0; i--) {
      if (sought == string[i]) {
         break;
      }
   }
   return i;
}



//////////////////////////////
//
// unsuffix -- remove the .* extension of a filename.
//

char* unsuffix(const char *string) {
   int dotPosition;
   char *base;

   dotPosition = backwardPosition('.', string);

   if (dotPosition != -1) {
      base = new char[dotPosition+1];
      if (base == NULL) {
         cerr << "Out of memory." << endl;
         exit(1);
      }
      strncpy(base, string, dotPosition);
      base[dotPosition+1] = '\0';
   } else {
      base = new char[1];
      base[0] = '\0';
   }
   return base;
}



//////////////////////////////
//
// checkOptions --
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("c|command-marker=s", "command marker");
   opts.define("v|variant=s",        "variant marker");
   opts.define("s|skip=i:4",         "number of characters to skip");
   opts.define("e|echo=b",           "echo the command being executed");
   opts.define("x|quiet=b",          "be quiet");
   opts.define("h|help=b",           "show help");
   opts.define("m|manual=b",         "show man page");
   opts.define("Z|author=b",         "show author");
   opts.process(argc, argv);

   if (opts.getBoolean("command-marker")) {
      commandMarker = opts.getString("command-marker").c_str();
   }
   if (opts.getBoolean("variant")) {
      variantSmake = opts.getString("variant").c_str();
   }

   numSkip = opts.getInteger("skip");
   echo    = opts.getBoolean("echo");
   quiet   = opts.getBoolean("quiet");

   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp"
           <<  "<craig@ccrma.stanford.edu>; 1996\n";
      exit(1);
   }
   if (opts.getBoolean("manual")) {
      manpage();
      exit(1);
   }
   if (opts.getBoolean("help")) {
      cout << "Written by Craig Stuart Sapp"
           <<  "<craig@ccrma.stanford.edu>; 1996\n";
      exit(1);
   }

   if (opts.getArgCount() < 1) {
      usage(opts.getCommand().c_str());
      exit(1);
   }

   if (variantSmake != NULL) {
      char *temp = new char[strlen(variantSmake) + strlen("$Smake:") + 1];
      strcat(temp, "$Smake-");
      strcat(temp, variantSmake);
      strcat(temp, ":");
      variantSmake = temp;
      commandMarker = variantSmake;
   }
   if (commandMarker == NULL) {
      commandMarker = MagicString;
   }
}



//////////////////////////////
//
// find -- find a string in a istream, and position the istream after
//	that string or at the end of the file.
//

void find(const char* searchString, istream& inFile) {
   char c;
   int length = strlen(searchString);
   int position = 0;

   while (position < length && inFile.get(c)) {
      if (c == searchString[position]) 
         position++;
      else 
         position = 0;
   }
}



//////////////////////////////
//
// nextChar -- return the next non-space, non-return character in the
// 	(text) file.  return '\0' if end-of-file or a return character.
//

char nextChar(istream& inFile) {
   char c = '\0';

   while (inFile.get(c) && isspace(c)) {
      if (c == '\n') {
         c = '\0';
         return c;
      }
   }
   return c;
}



//////////////////////////////
//
// readCommand -- read the command string from the file.
//

void readCommand(stringstream& strcommand, const char* commandMarker, 
      istream& inFile) {
   char c;

   find(commandMarker, inFile);

   c = nextChar(inFile);
   if (c == '\0') return;
   else strcommand << c;

   while (inFile.get(c) && c != '\n') {
      if (c == '%') {
         inFile.get(c);
         switch (c) {
            case '\n': 
               for (int i=0; i<numSkip; i++) {
                  inFile.get(c);
                  if (c == '\n') {
                     cerr << "Error: too many spaces to skip on line"
                          << endl;
                     exit(1);
                  }
               }
               while (inFile.get(c) && isspace(c)) {
                  if (c == '\n') return;
               }
               strcommand << c;
               break;         
            case '1': case '2': case '3': case '4': case '5': 
            case '6': case '7': case '8': case '9':
               if (numPassParams > (int)c - '0' - 1) {
                  strcommand << passParams[(int)c - '0' - 1];
               }
               else {
                  strcommand << '%' << c;
               }
               break;
            case 'f': 
               strcommand << filename;
               break;
            case 'b': 
               strcommand << Basename;
               break;
            default:
               strcommand << '%' << c;      
         }
      } else {
         strcommand << c;
      }
   }
}





//////////////////////////////
//
// manpage --
//

void manpage(void) {
   cout << 
"\n"
"SMAKE(1)            UNIX Programmer's Manual             SMAKE(1)\n"
"\n"
"\n"
"\n"
"NAME\n"
"     smake - Simple MAKE.  Executes a command string embedded in\n"
"     a file.\n"
"\n"
"SYNOPSIS\n"
"     smake [-s skip][-v variant |-c][-e|-x] input-file [parame-\n"
"           ters]\n"
"\n"
"DESCRIPTION\n"
"     Smake searches for the character pattern $Smake: in a file.\n"
"     When it finds that pattern, smake will execute, as a shell\n"
"     command, the text following $Smake: to the end of the line.\n"
"\n"
"     There are several symbolic patterns that can be used in the\n"
"     embedded command:\n"
"\n"
"     %f       If this character pattern is in the embedded com-\n"
"              mand, the input filename passed to smake will be\n"
"              substituted.\n"
"\n"
"     %b       If this character pattern is in the command, the\n"
"              filename minus the characters after the last period\n"
"              will be removed, as well as the last period.  For\n"
"              example, the basename of test.c is test .\n"
"\n"
"     %<newl>  Where <newl> is the newline character. If % comes\n"
"              at the end of the line with no other characters\n"
"              except the newline character following it, the\n"
"              embedded command continues on the following line.\n"
"              The -s option determines where the command resumes\n"
"              on the next line.\n"
"\n"
"     %1--%9   If any parameters follow the input-file on the com-\n"
"              mand line, then these parameters get matched to the\n"
"              symbols %1, %2, etc. up to %9.  If there is no\n"
"              passed parameter for a particular symbol, then it\n"
"              is left unchanged in the command.\n"
"\n"
"OPTIONS\n"
"     -s skip  skip is the number of characters to skip at the\n"
"              beginning of a new line for the shell command\n"
"              line-continuation.  The default value of skip is 4.\n"
"              It is an error to skip beyond the end of the line.\n"
"\n"
"     -v variant\n"
"              variant is a string used to distinguish between\n"
"              different possible commands to extract.  The form\n"
"              of the marker in the file is $Smake-variant:.\n"
"\n"
"     -e       echo the embedded command string without executing\n"
"              it.\n"
"\n"
"     -x       execute the command string only, without printing\n"
"              it to the terminal as well.\n"
"\n"
"     -c pattern\n"
"              pattern is a string to replace the default marker\n"
"              $Smake: with pattern.\n"
"\n"
"EXAMPLES\n"
"     A command to compile a C program and strip the executable:\n"
"\n"
"     // $Smake:    gcc -O -o %b %f && strip %b\n"
"\n"
"     A command to produce and display a PostScript file from a\n"
"     TeX file:\n"
"\n"
"     % $Smake:     tex %b && dvips -o %b.ps %b && open %b.ps\n"
"\n"
"BUGS\n"
"     Cannot use the -v option and the -c option at the same time.\n"
<< endl;

}


