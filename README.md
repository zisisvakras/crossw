4η εργασία Εισαγωγή στον Προγραμματισμό 2022-2023


## Intro
Η εργασία έγινε ομαδικά απο τους:
sdi2200014 Ζήσιμος Βακράς
sdi2200122 Ιωάννης Ξηρός
ισότιμα, και οι δυό μας έχουμε πλήρη γνώση της υλοποίησης.
(Κάθε γραμμή έχει γραφτεί συλλογικά)
Το πρόγραμμα γίνεται compile με `make` και βρίσκεται στο '/target/crossw'
Στην εργασία αυτή μας βοήθησε πολύ το paper του Stanford πάνω στα csp crosswords
και το paper του P.Prosser πάνω στις τεχνικές labeling και unlabeling των variables σε csp
1. https://web.stanford.edu/~jduchi/projects/crossword_writeup.pdf
2. P. Prosser. Hybrid Algorithms for the Constraint Satisfaction Problem.
Computational Intelligence, Volume 9, Number 3. 1993.
Επίσης μας βοήθησε πολύ και ο στεφ (το rubber ducky μας) και τον ευχαριστούμε πολύ.

## Υλοποίηση
### Αναπαράσταση του σταυρόλεξου


Το σταυρόλεξο μας είναι ενας δισδιάστατος πίνακας char (char**) στον οποίο
τα κελιά που θα μπουν γραμματα αρχικοποιουνται σε '\0' και τα μαύρα κελια
σε '\r'. Ο λόγος που επιλέξαμε αυτά τα chars είναι γιατί το πρόγραμμα 
υποστηρίζει χαρακτήρες όλου του ascii table και αυτοι οι δυο δεν θα υπάρχουν 
σε κάποια λέξη του λεξικού.


### Λεξικό 


Αρχικά δεσμεύουμε χώρο για να φορτωθεί όλο το αρχείο λεξικού στη μνήμη και στη συνέχεια 
ταξινομούμε τις λέξεις κατά μήκος σε υπολεξικά. Όσες λέξεις δεν θα χρειαστούν 
στη λύση δεν εκχωρούνται πουθενα και απλά γίνονται free στο τέλος του
προγράμματος. Στη συνέχεια ταξινομούμε τα υπολεξικά βάσει τη συχνότητα 
εμφάνισης των γραμμάτων (για τα μήκη λέξεων που μας ενδιαφέρουν).


### Χαρτογράφηση του λεξικού 


Για τη γρήγορη αναζήτηση λέξεων στο λεξικό έχουμε δημιουργήσει packed bit 
arrays ή αλλιώς bitmaps στα οποία το κάθε bit αναπαριστά μια λέξη (index) 
σε ενα υπολεξικό, τα bitmaps αυτά έχουν type long long το οποίο μας δίνει 
την δυνατότητα σε ενα 64bit σύστημα να φορτώσουμε (σε έναν κύκλο μηχανής)
όλο τον αριθμό και να κάνουμε γρήγορα πράξεις bitwise-and(&).
Τα bitmaps αποτελούν τον πυρήνα του προγράμματος καθως αποτελούν άμεση 
αναπαρασταση του domain μιας λέξης.


### Χαρτογράφηση του σταυρόλεξου


Για να χαρτογραφήσουμε τις λέξεις πάνω στο σταυρόλεξο χρησιμοποιούμε structs
τα οποία κρατάνε όσες πληροφορίες μας είναι χρήσιμες για τη λύση (για παραπάνω 
πληροφορίες δείτε την υλοποίηση). Σημαντικό είναι ότι κρατάμε και τα intersections
κάθε λέξης σε ενα array για να μας βοηθήσουν στο fc-cbj που θα αναφέρουμε αργότερα.


### Λύση του σταυρόλεξου 


Ο αλγόριθμος που χρησιμοποιούμε είναι μια μίξη των τεχνικών:
- cbj (conflict-directed backjumping) 
- fc (forward checking)
- dvo (dynamic variable ordering) (πιο συγκεκριμένα mrv)


Βασισμένοι στα προαναφερθέντα papers, για κάθε μεταβλητή δημιουργούμε 
τα σύνολα conf-set και past-fc με βάση τα οποία επιλέγουμε ποια μεταβλητή 
αποδείχθηκε "προβληματική" και επομένως πρέπει να αφαιρεθεί ώστε να 
επιστρέψουμε στο σημείο πριν τοποθετηθεί αναιρώντας όλες τις ενδιάμεσες επιλογές.
Στο σύνολο past-fc αποθηκεύονται οι μεταβλητές οι οποίες έκαναν forward check 
στην εξεταζόμενη μεταβλητή, η τιμη τους δηλαδή επηρεασε το domain της εξεταζόμενης μεταβλητης
ενώ από το σύνολο conf-set (που ενημερώνεται βάσει του past-fc στο unlabeling/fc) 
επιλέγεται η μεταβλητή στην οποία θα γίνει το backjump. Στο conf-set μιας μεταβλητής 
αποθηκεύονται όλα τα προηγούμενα conflict που ειχαν και λέξεις στις οποίες έκανε αυτή intersect.


Ακολουθούμε την εξής λογική:


1. Επιλέγουμε πάντα να συμπληρωθεί το variable (λέξη στο σταυρόλεξο) η οποία 
έχει τις λιγότερες επιλογές (mrv, domain size). Αυτό γίνεται με την συνάρτηση 
sum_bit η οποια υπολογιζει ποσοι άσοι υπάρχουν σε ενα bitmap 
το οποίο είναι ισοδύναμο με το πλήθος των λέξεων που μπορεί να μπουν σε αυτη τη θέση.


2. Το labeling (fc-cbj) (αναθεση value σε variable) γίνεται με τον παρακάτω αλγόριθμο:

    α. Βρίσκεται λέξη (value) από το λεξικό
    β. Αν το συγκεκριμένο value μηδενίζει κάποιο από τα domains των
    intersections, τοτε στο conf-set αυτού της εξεταζόμενης λεξης
    ανέθεσε την ένωση αυτού με το past-fc του intersection

    Αν η συνθήκη του β αληθεύει τότε τα βήματα γ εως ε παραλείπονται 
    και ο αλγόριθμος ξεκινάει πάλι απο το α με καινούργια τιμη του λεξικου
        
    γ. Αποθήκευσε τους χάρτες τους οποίους προκειται να επηρεάσει το συγκεκριμενο value
    (μονο των intersections δηλαδή) στη στοίβα map_stack
    δ. Ενημέρωσε τους χάρτες αυτούς βάσει του value
    ε. Πρόσθεσε την εξεταζόμενη μεταβλητή στο σύνολο past-fc όλων των intersections

3. Το unlabeling (fc-cbj) (επιστροφή σε πρώιμη κατάσταση) γίνεται με τον παρακάτω αλγόριθμο:

    α. Διαπιστώνουμε οτι η εξεταζόμενη μεταβλητή (έστω Α) έχει κενό domain
    β. Βρες το μεγαλύτερο στοιχείο από τα past-fc, conf-set της Α
    (τη πιο πρόσφατα τοποθετημένη μεταβλητή που υπάρχει σε αυτα τα σύνολα, έστω B)
    γ. Τοποθέτησε στο conf-set της B την ένωση των συνόλων past-fc, conf-set της Α
    δ. Αφαίρεσε την Β απο το conf-set της Β

    Επομένως επιλέχθηκε η μεταβλητή Β ως προβληματική 
    και θα επιστρέψουμε στην κατάσταση πριν την ανάθεση της

    Για όλες τις μεταβλητές απο το Α μέχρι το Β κάνε:
    ε. Μηδένισε τα conf-set των μεταβλητών αυτών 
    στ. Επανέφερε απο τη στοίβα map_stack τους χάρτες στις πειραγμένες μεταβλητες

Όποτε η ροή του προγράμματος έχει ως εξής:

Το mrv επιλέγει την μεταβλητή με τις λιγότερες επιλογές και αν βρεθεί τιμή για αυτή προχωράμε στο
labeling αυτής, αποθηκεύουμε το map της στη στοίβα (σε περίπτωση backtrack) στη συνέχεια επιλέγεται
η επόμενη μεταβλητή (mrv). Επαναλαμβάνεται αυτή η διαδικασία μέχρι όλες οι μεταβλητές να γίνουν label
και επομένως να λυθεί το σταυρόλεξο. Φυσικά υπάρχει περίπτωση το mrv να επιλέξει μια μεταβλητή με μηδέν
επιλογές και επομένως εξαντλημένο domain όπου τότε προχωράμε στο unlabeling.
