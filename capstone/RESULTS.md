# Capstone results log

Absolute ns are machine-local; **ratios and checksum** travel.

Default tape: `LL_TAPE=uniform` (or unset). Also run `cancels`.

| Build | tape | fills_qty | checksum | resting | tape p50 | per-order p50 | notes |
|-------|------|-----------|----------|---------|----------|---------------|-------|
| naive | uniform |  |  |  |  |  |  |
| naive | cancels |  |  |  |  |  |  |
| starter (first) | uniform |  |  |  |  |  |  |
| starter (best) | uniform |  |  |  |  |  |  |
| starter | cancels |  |  |  |  |  |  |
| reference | uniform |  |  |  |  |  | optional |

Starter checksum **must** equal naive on that tape. Prefer **per-order** p50 when you talk about the tick.

## Units I actually used

- 

## What I tried that did not help

- 
