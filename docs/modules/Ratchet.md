# Ratchet — 8 HP

Ratchet is a trigger-burst generator. Feed it an incoming clock or trigger and it subdivides the interval into a burst of repeats, with per-repeat spread and probability. It is designed to stay in time with the source that drives it.

## Parameters

| Param | Range | Default | Notes |
|---|---|---|---|
| Repeats | 1–8 | 2 | Number of bursts in the current interval |
| Repeats CV amount | -1 to +1 | 0 | Scales the incoming Repeats CV |
| Spread | -1 to +1 | 0 | Shifts the repeats within the interval |
| Repeat probability | 0–100% | 100% | Probability that each repeat will fire |

## Inputs

| Input | Notes |
|---|---|
| Trigger | Incoming clock or trigger to subdivide |
| Repeats CV | Modulates the repeat count |

## Outputs

| Output | Notes |
|---|---|
| Burst | Main trigger burst |
| End of burst | Fires on the last repeat |
