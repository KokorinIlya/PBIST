# Benchmark results

## Sequential 

| Operation | Time (ms) |
| ------------- | ------------- | 
| Contains | 9257 |
| Insert | 10326 |
| Remove | 9565 |

## Parallel Contains

| | Binary search (+ extra copies) | Interpolation search, size >= 50, id size = keys count |
| ------------- | ------------- | ------------- | 
| `CILK_NWORKERS = 1` | 4033 | 3542 |
| `CILK_NWORKERS = 8` | 511 | 468 |
| Speedup | 7.89 | 7.57 |

## Parallel Insert

| | Binary search (+ extra copies) | Interpolation search, size >= 50, id size = keys count |
| ------------- | ------------- | ------------- | 
| `CILK_NWORKERS = 1` | 9026 | 7940 |
| `CILK_NWORKERS = 8` | 1497 | 1272 |
| Speedup | 6.03 | 6.24 |


## Parallel Remove

| | Binary search (+ extra copies) | Interpolation search, size >= 50, id size = keys count |
| ------------- | ------------- | ------------- | 
| `CILK_NWORKERS = 1` | 8312 | 7495 |
| `CILK_NWORKERS = 8` | 1122 | 1067 |
| Speedup | 7.41 | 7.02 |
