sed '/^crease 45.000000$/ D' michigan-src.ac > michigan1.ac
sed '/^crease 180.000000$/ D' michigan1.ac > michigan2.ac
accc +shad michigan2.ac michigan-shade.ac
accc -g michigan.acc -l0 michigan2.ac -l1 michigan-shade.ac -d3 500 -d2 500 -d1 300 -S 300 -es
