# Benchmark results

## Sequential 

| Operation | Time (ms) |
| ------------- | ------------- | 
| Contains | 9257 |
| Insert | 10326 |
| Remove | 9565 |

## Parallel Contains

| | Time (ms) | 
| ------------- | ------------- |
| `CILK_NWORKERS = 1` | 1682 |
| `CILK_NWORKERS = 8` | 229 |
| Speedup | 7.34 |

## Parallel Single Insert

| | Time (ms) | 
| ------------- | ------------- |
| `CILK_NWORKERS = 1` | 5032 |
| `CILK_NWORKERS = 8` | 841 |
| Speedup | 5.98 |

## Parallel Single Remove

| | Time (ms) | 
| ------------- | ------------- |
| `CILK_NWORKERS = 1` | 2930 |
| `CILK_NWORKERS = 8` | 423 |
| Speedup | 6.93 |

## Parallel Multi Insert (10'000 batches, 1'000 keys each)

| | Time (ms) | 
| ------------- | ------------- |
| `CILK_NWORKERS = 1` | 55848 |
| `CILK_NWORKERS = 8` | 10347 |
| Speedup | 5.40 |
