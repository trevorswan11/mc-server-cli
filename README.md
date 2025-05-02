# MC Server ClI
A basic command line interface for interacting with [mcrcon](https://github.com/Tiiffi/mcrcon) with autocomplete.

## Getting Started
1. First, clone this repository using `git clone https://github.com/trevorswan11/mc-server-cli.git`
2. Then, create a `.env` file. There is no need to use quotes in the following variables, but ensure both are defined as follows:
```
RCON_PORT=<your port here>
RCON_PASS=<your pass here>
```

3. Then, simply run `make`

### Dependencies
- This only works on linux, and the `libreadline-dev` must be installed for this cli to work
