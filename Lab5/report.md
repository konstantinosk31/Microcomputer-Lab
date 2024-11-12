---
#{{{
# vim: set conceallevel=0:
title: 'Χρήση εξωτερικών Θυρών Επέκτασης στον AVR'
subtitle: 'Εργαστήριο Μικροϋπολογιστών'
author:
- Κριθαρίδης Κωνσταντίνος, $el21045$
- Μπαλάτος Δημήτριος, $el21170$
date: '12 Νοεμβρίου 2024'
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

# Ζήτημα 5.1
Αρχικά, μεταφέρουμε τον κώδικα της εκφώνησης για την επικοινωνία με το PCA9555 μέσω της διεπαφής TWI. Στην συνάρτηση main, αφού αρχικοποιήσουμε το TWI, και θέσουμε το `EXT_PORT0` ως έξοδο μέσω του καταχωρητή `REG_CONFIGURATION_0` και το PORTB ως input, ξεκινάμε να διαβάζουμε διαρκώς το PINB (το οποίο μετατρέπουμε σε θετική λογική). Αποθηκεύουμε σε διαφορετικές μεταβλητές τα $A,\ B,\ C,\ D$ και υπολογίζουμε τα $F0,\ F1$ με βάση αυτές. Τέλος, γράφουμε στο `EXT_PORT0` μέσω του καταχωρητή `REG_OUTPUT_0` τις τιμές των $F0,\ F1$. Αφού έχουμε συνδέσει τους ακροδέκτες `IO0_0` και `IO0_1` του κονέκτορα `P18` με τους ακροδέκτες `LED_PD2` και `LED_PD3`, βλέπουμε το αποτέλεσμα που γράφουμε στο `EXT_PORT0` στα αντίστοιχα LEDs. 

```asm {source=Ex5_1/Ex5_1/main.c}
```

# Ζήτημα 5.2


```c {source=Ex5_2/Ex5_2/main.c}
```

# Ζήτημα 5.3


```c {source=Ex5_3/Ex5_3/main.c}
```
