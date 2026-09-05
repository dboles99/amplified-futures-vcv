# Swarm Core sample banks — attribution

The audio in `Cicadidae/` and `Orthoptera/` is derived from **InsectSet32**.

- **Creator:** Marius Faiß (Leiden University, 2022)
- **Curated by:** Baudewijn Odé and Ed Baker · **Supervised by:** Dan Stowell
- **Source:** <https://doi.org/10.5281/zenodo.7072196>
- **Licence:** Creative Commons Attribution 4.0 International (CC BY 4.0)
- **Licence text:** <https://creativecommons.org/licenses/by/4.0/>

## Modifications made

Each file here is **a modified version** of an original recording:

- a single channel taken from the original,
- truncated to 5 seconds,
- converted to 16-bit 44.1 kHz mono.

That is the form Swarm Core reduces the audio to at load time in any case, so
nothing audible is lost by shipping it this way. No other processing is applied.

The selection was made by `scripts/curate-insect-banks.py`, taking recordings
round-robin across species so that every species in a family is represented
rather than one insect repeated.

## Note on licensing

The plugin itself is MIT. **This directory is not.** The audio under
`res/insects/` remains CC BY 4.0 and carries the attribution requirements above;
redistributing it — including inside a build of this plugin — means keeping this
file with it.
