# Swarm Core sample banks

The audio in `Cicadidae/` and `Orthoptera/` is derived from **InsectSet32**
(Zenodo 7072196), used under **CC BY 4.0**.

Each file is a single channel of an original recording, truncated to 5 seconds
and converted to 16-bit 44.1 kHz mono - the form Swarm Core reduces it to at
load time in any case. No other processing is applied.

Built by `scripts/curate-insect-banks.py` from the full set, selecting round
robin across species so every species in a family is represented.

> InsectSet32, CC BY 4.0. https://doi.org/10.5281/zenodo.7072196
