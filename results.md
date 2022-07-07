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
| `CILK_NWORKERS = 1` | 1682 |
| `CILK_NWORKERS = 8` | 229 |
| Speedup | 7.89 | 7.34 |

## Parallel Single Remove

| | Time (ms) | 
| ------------- | ------------- |
| `CILK_NWORKERS = 1` | 1682 |
| `CILK_NWORKERS = 8` | 229 |
| Speedup | 7.89 | 7.34 |

## Parallel Multi Insert

| | Time (ms) | 
| ------------- | ------------- |
| `CILK_NWORKERS = 1` | 1682 |
| `CILK_NWORKERS = 8` | 229 |
| Speedup | 7.89 | 7.34 |
