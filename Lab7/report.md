---
#{{{
# vim: set conceallevel=0:
title: 'Αισθητήρας θερμοκρασίας DS1820 στην κάρτα ntuAboard_G1'
subtitle: 'Εργαστήριο Μικροϋπολογιστών'
author:
- Κριθαρίδης Κωνσταντίνος, $el21045$
- Μπαλάτος Δημήτριος, $el21170$
date: '26 Νοεμβρίου 2024'
subject: ''
keywords: ''
abstract: ''
lang: el_GR
linkcolor: blue
fontsize: 12pt
#fontfamily: 'GFS Artemisia'
mainfont: 'CMU Serif'
sansfont: 'CMU Sans Serif:style=BoldItalic'
monofont: 'CMU Typewriter Text:style=LightOblique'
documentclass: article
classoption: ''
toc: false
papersize: a4
geometry:
- margin=1.5cm
urlcolor: blue
toccolor: black
links-as-notes: false
numbersections: true
#}}}
---

# Ζήτημα 7.1

Αρχικά μετατρέπουμε τον δωσμένο κώδικα από Assembly σε C. Στην συνέχεια ορίζουμε 
νέα βοηθητική συνάρτηση `read_temp(void)`{.c} που διαβάζει την τιμή του DS1820 
και επιστρέφει αντίστοιχη `float`{.c} τιμή.

Για διευκόλυνση, ορίσαμε τύπο `bool`{.c} και μακροεντολές `SET(x, b)`, 
`CLEAR(x, b)` για την γρήγορη αλλαγή κάποιου bit σε αριθμό.

```c {source=Ex7_1/Ex7_1/main.c}
```

# Ζήτημα 7.2

> Στην πλακέτα μας είχαμε αισθητήρα DS18B20.

Αντιγράφουμε τον παραπάνω κώδικα, με μία αλλαγή στην `read_temp(void)`{.c}: η 
συνάρτηση πλέον επιστρέφει σε `unsigned int`{.c} 16 bits την raw τιμή που 
επιστρέφει ο αισθητήρας, προκειμένου να διευκολυνθεί η εκτύπωση της 
θερμοκρασίας.  Για την χρήση LCD οθόνης χρειάζεται να χρησιμοποιήσουμε port 
expander, άρα αντιγράφουμε και τον boilerplate κώδικα επικοινωνίας από παλιότερη 
άσκηση.

Δημιουργούμε νέα συνάρτηση για την εκτύπωση θερμοκρασίας
`lcd_temp(val, precision)`{.c}: διαχωρίζουμε την raw τιμή σε ακέραιο και 
δεκαδικό μέρος, εκτυπώνουμε ολόκληρο το ακέραιο μέρος και μετά `precision` ψηφία 
του δεκαδικού. Στο τέλος εκτυπώνουμε το σύμβολο $^oC$.

Στην συνάρτηση `main`{.c}, διαβάζουμε συνέχεια τιμή θερμοκρασίας και, αν αυτή 
εκφράζει σφάλμα ανάγνωσης εκτυπώνουμε το ζητούμενο error message, αλλιώς 
εκτυπώνουμε την θερμοκρασία μέσω της συναρτησης.

> Για να επιταχύνουμε τον κώδικα εκτύπωσης, αντί να καθαρίζουμε την LCD 
> μετακινούμε τον κέρσορα στην πρώτη στήλη της πρώτης γραμμής μέσω 
> `lcd_command(instr)`{.c}.

```c {source=Ex7_2/Ex7_2/main.c}
```
