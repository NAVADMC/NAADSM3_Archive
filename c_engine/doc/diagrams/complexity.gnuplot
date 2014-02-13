set xlabel "Relative size"
set ylabel "Relative running time"
plot x*x title 'n^2' w l lw 3, '-' title 'Uncontrolled spread' w l lw 3, \
  '-' title 'Controlled' w l lw 3
0.1  0.01
0.2  0.04
0.5  0.26
1    1
2    4.23
5   27.07
10  101.88
e
0.1  0.03 
0.2  0.08
0.5  0.34
1    1
2    2.96
5   10.8
10  29.22
e
