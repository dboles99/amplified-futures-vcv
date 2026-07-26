# Ratchet — 8 HP

Ratchet subdivides an incoming trigger stream into a burst of repeats. Feed it a clock and it produces a burst that stays aligned with the source, with spread and probability shaping the result.

## Use it

1. Patch a clock or trigger into **TRIG**.
2. Set **Repeats** to choose the burst length.
3. Use **Spread** to move the burst within the interval.
4. Lower **Repeat probability** if you want occasional gaps in the burst.

## See also

[[Street-Grid-Clock]] · [[Pulse]]

**Full parameter spec:** [`docs/modules/Ratchet.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/Ratchet.md)
