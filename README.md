# UNIX System Programming - Project 1

This project is structured into modules to ensure better code organization. The project structure is as follows:


|-include
|-----alias.h
|-----background.h
|-----history.h
|-----pipe.h
|-----redirection.h
|-----signals.h
|-----utils.h
|-src
|-----alias.cpp
|-----background.cpp
|-----history.cpp
|-----pipe.cpp
|-----redirection.cpp
|-----signals.cpp
|-----utils.cpp
|-Makefile
|-mysh.cpp


## Implementation Details

The implementation of the project has been carried out in C++. The Standard Template Library (STL) has not been used. Instead, most of the code relies on libraries from the C language. The project has been developed adhering to all the specifications provided in the project outline.

## Execution Instructions

To execute the project, follow the steps below:

1. Open a terminal.
2. Navigate to the current directory of the project using the appropriate `cd` commands.
3. Run the `make` command to compile the project.
4. Execute the application using the `./mysh` command.

For instance, to run the program, use:

```bash
./mysh
```

-----------------------------mysh-----------------------------
Implementation
1.	Αποθηκεύει την εντολή του χρήστη στον πίνακα χαρακτήρων users_command
2.	Αποθηκεύει την εντολή στον πίνακα history μέσω της συνάρτησης addHistory()
3.	Χειρίζεται τα σήματα control-c, control-z μέσω της συνάρτησης signal()
4.	Κάνει parse την εντολή μέσω της συνάρτησης parseCommand() σε tokens
5.	Ελέγχει αν η εντολή περιέχει ; και χρειάζεται να εκτελεστούν πολλές εντολές
6.	Ελέγχει αν η εντολή είναι η “history” μέσω της checkHistory()
7.	Ελέγχει αν η εντολή περιέχει alias μέσω της checkAlias()
8.	Ελέγχει αν η εντολή περιέχει pipeline και αν ναι τότε την χειρίζεται κατάλληλα
9.	Αν δεν έχει, τότε ελέγχει για ανακατεύθυνση εισόδου/ εξόδου μέσω της checkRedirection()
10.	Ελέγχει αν η εντολή είναι background μέσω της checkBackground()
11.	Ελέγχει αν η εντολή στο background τελείωσε μέσω της checkFinishedBackground(). H mysh κρατάει το pid της νέας background εντολής στην μεταβλητή bg_pid. Μέσω αυτής της συνάρτησης ελέγχει πότε τελείωσε και αν τελείωσε τότε την κάνει wait και τυπώνει το αναγνωριστικό της
12.	Αν δεν είναι εντολή στο background, τότε κάνει fork()
13.	Χειρίζεται την ανακατεύθυνση αν υπάρχει
14.	Κάνει exec την tokenized εντολή 

-----------------------------Redirection-----------------------------
Examples; 1) in-mysh-now:>ls -l > out.txt
	 2) in-mysh-now:>sort < input.txt > out.txt
	 3) in-mysh-now:>cat input.txt >> out2.txt
	 
Implementation
Η συνάρτηση checkRedirection() διασχίζει τον πίνακα tokens για να αναγνωρίσει σύμβολα ανακατεύθυνσης και τροποποιεί τα flags ανάλογα.
Η handleRedirection() χειρίζεται την ανακατεύθυνση κάνοντας open και dup2 τα αρχεία εισόδου - εξόδου. Ειδικά για την έξοδο, αν το σύμβολο είναι ">" τότε κάνει append αλλιώς αν είναι ">>" τότε κάνει truncate.

-----------------------------Pipes-----------------------------
Examples; 1) in-mysh-now:>cat input.txt | sort > out3.txt
	  2) in-mysh-now:>cat input.txt input2.txt | sort > out3.txt
	 
Implementation
Η συνάρτηση handlePipe() διασχίζει τον πίνακα tokens για να αναγνωρίσει αν υπάρχει σύμβολο pipe και αν ναι, τότε δημιουργεί pipe με read-write ends, που για το καθένα κάνει νέο fork και αφού έχει χωρίσει τον πίνακα tokens σε 2 υποπίνακες για το καθέ άκρο της σωλήνωσης κάνει exec ελέγχοντας και για ανακατεύθυνση. 

-----------------------------Background-----------------------------
Example; 1) in-mysh-now:>sort input.txt &
	    ls
	  
Implementation
Η συνάρτηση checkBackground() διασχίζει τον πίνακα tokens για να αναγνωρίσει αν υπάρχει σύμβολο background και αν ναι, τότε δημιουργεί νέα διεργασία με το pid της να χειρίζεται η μητρική διεργασία. 
Η checkFinishedBackground() ενόσω η σημαία background == true, ελέγχει αν η διεργασία στο παρασκήνιο έχει τελειώσει, αλλιώς κοιμάται για 1 sec και ελέγχει ξανά.

-----------------------------Wildcards-----------------------------
Examples; 1) in-mysh-now:>ls *.t?t
	  2) in-mysh-now:>ls mysh*
	 
Implementation
Η διαχείρηση των wildcards γίνεται στο αρχικό parsing. Άμα ανειχνευτεί ένας από τους 2 χαρακτήρες των wildcards τότε μέσω της βιβλιοθήκης glob.h βρίσκονται όλα τα patterns που ταιριάζουν με την είσοδο του χρήστη.

-----------------------------Aliases-----------------------------
Examples; 1) in-mysh-now:>createalias myalias ls
	  2) in-mysh-now:>destroyalias myalias
	 
Implementation
Η διαχείρηση των aliases γίνεται μέσω της συνάρτησης checkAlias() η οποία ελέγχει τον πίνακα tokens για τα tokens createalias, destroyalias και προσθέτει τα aliases στον πίνακα aliases. Για κάθε input του χρήστη διασχίζεται ο πίνακας aliases για να διακρίνει αν το token του χρήστη είναι aliased ή όχι.

-----------------------------Signals-----------------------------
Examples; 1) in-mysh-now:>^Z
	  2) in-mysh-now:>^C
	 
Implementation
Η διαχείρηση των aliases γίνεται μέσω της συνάρτησης signal() από τη βιβλιοθήκη signal.h. Ορίστικαν οι signal handlers sigint_handler() και sigtstp_handler() για ^C και ^Ζ αντίστοιχα οι οποίες τερματίζουν την τρεχούμενη διεργασία.

-----------------------------History-----------------------------
Examples; 1) in-mysh-now:>myHistory
	  2) in-mysh-now:>myhistory 2
	 
Implementation
Η διαχείρηση του ιστορικού γίνεται μέσω της συνάρτησης addHistory() η οποία προσθέτει κάθε εντολή του χρήστη στον πίνακα ιστορικού history[]. Μέσω της checkHistory() όταν ο χρήστης πληκτρολογήσει myHistory μπορεί να δει το ιστορικό του και πληκτρολογώντας myhistory {index} (πχ. myhistory 2) μπορεί να εκτελέσει την εντολή ιστορικού index.

