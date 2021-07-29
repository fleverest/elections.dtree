# Usage
#### Warning: usage directions not updated since migration to Rcpp

```cpp
g++ -Os -o simulate src/simulate_election.cpp
./simulate -s seedstring -t dirichlettree -k 1. -n 5 -m 1000 > sim_n5m1K.csv
```

```cpp
g++ -Os -o montecarlo src/montecarlo.cpp
./montecarlo -i sim_n5m1K.csv -o res_n9m1M.csv1 -e 25 -b 10 -s 9 -k 10. -t dirichlettree -c seed
```

